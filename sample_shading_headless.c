#define RENDERING_API EGL_OPENGL_ES2_BIT
#define MSAA_SAMPLES 16
#define MSAA_SAMPLE_SHADING 0.5f
//#define ENABLE_MSAA

#include "egl_common.h"
#include "TexReader.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <cglm/cglm.h>
#include <cglm/mat4.h>

#define USE_STB
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include "shaders.h"
#include "mesh.h"
#include "gldebug.h"
#include "utils.h"

struct ClientObjState;
struct GlState;
struct GfxPipeline;

static uint32_t startTime = 0;
static uint16_t surfaceWidth = 1920;
static uint16_t surfaceHeight = 1080;
static uint32_t numOfMSAAsamples = 0;
static float minSampleShadingValue = 0.0f;
static char texFileName[256];

static int texWidth;
static int texHeight;

static struct Mesh* pTriangleMesh = NULL;
static struct Mesh* pQuadMesh = NULL;

static clock_t simulation_start;

void* pixelDump = NULL;
size_t pixelDumpSizeInBytes;
const uint16_t bytespp = 4;
uint8_t initialFrameCallbackDone = 0;
uint8_t SurfacePresented = 0;
uint8_t imgReadJob = 0;

static PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC glFramebufferTexture2DMultisampleEXT = NULL;
static PFNGLMINSAMPLESHADINGOESPROC glMinSampleShadingOES = NULL;

static void updateFrame_callback( void* pData, uint32_t time );
static void UpdateUniforms( struct ClientObjState* pClientObj );
static void recordGlCommands( struct ClientObjState* pClientObj, uint32_t time );
static void drawTriangle(
	struct ClientObjState* pClientObj, uint32_t time,
	const void* position, const void* texcoords, const void* colors
);
static void drawQuad( 
	struct ClientObjState* pClientObj, uint32_t time,
	float clear_r, float clear_g, float clear_b, float clear_a,
	const void* position, const void* texcoords, const void* colors,
	const void* indices,
	GLuint textureToUse
);

static void InitGLState( struct GlState* pGLState );

static void SetupFBO( 
	GLuint* fbo,
	GLuint* colorAttachmentTexture,
	GLenum texture_format,
	GLenum pixelStorage,
	GLuint numOfSamples
);

struct GfxPipeline{
	GLuint gpuprogram;
	GLuint mvp_unifrom;
	GLuint texSampler_uniform;
	GLuint position_attribute;
	GLuint texcoord_attribute;
	GLuint color_attribute;
};

struct ClientObjState
{
	struct eglContext mpEglContext;

	int8_t mbCloseApplication;
	int8_t mbSurfaceConfigured;

    struct GlState{
        GLuint ibo;
		GLuint texture;

		GLuint msaaFBO;
		GLuint msaaTexture;
		
		struct GfxPipeline vertexcolorPipeline;
		struct GfxPipeline renderToQuadPipeline;
    }mGlState;

    struct Uniforms{
        mat4 mvp;
		mat4 quadIdentityModelViewProj;
    }mUniforms;
};

static void updateFrame_callback( void* pData, uint32_t time )
{
	struct ClientObjState* pClientObjState = pData;

	if( imgReadJob )
		pClientObjState->mbCloseApplication = 1;

    UpdateUniforms( pClientObjState );
	recordGlCommands( pClientObjState, time );

	if( initialFrameCallbackDone && !imgReadJob )
	{
		SurfacePresented = 1;
		
		imgReadJob = DownloadPixelsFromGPU(
			DEFAULT_FRAME_BUFFER,
			FBO,
			0, 0,
			surfaceWidth, surfaceHeight,
			-1,
			GL_RGBA,
			0,
			bytespp,
			pixelDump, pixelDumpSizeInBytes
		);
	}
	else
	{
		initialFrameCallbackDone = 1;
	}

	SwapEGLBuffers( &pClientObjState->mpEglContext );
}

static void UpdateUniforms( struct ClientObjState* pClientObj )
{
	mat4 model, view, projection;

	glm_mat4_identity(model);
	glm_mat4_identity(view);
	glm_mat4_identity(projection);
	glm_mat4_identity( pClientObj->mUniforms.mvp );
	glm_mat4_identity( pClientObj->mUniforms.quadIdentityModelViewProj );

	float degree = 90.0f;
	float fovy = 45.0f;

	//glm_make_rad(&degree);
	glm_make_rad(&fovy);

	clock_t duration = clock() - simulation_start;
	float duration_in_secs = ( (float) duration ) / CLOCKS_PER_SEC;

	float rotation_axis[] = { 0.0, 1.0, 0.0 };
	//glm_rotate(
	//	model,
	//	duration_in_secs * degree,
	//	rotation_axis
	//);

	float view_eye[] = { 0.0, 0.0, 3.0 };
	float view_center[] = { 0.0, 0.0, 0.0 };
	float view_up[] = { 0.0, 1.0, 0.0 };

	glm_lookat(
		view_eye,
		view_center,
		view_up,
		view
	);

	glm_perspective(
		fovy,
		(float) surfaceWidth / (float) surfaceHeight,
		0.1,
		10.0,
		projection
	);

	glm_mat4_mul( projection, view, pClientObj->mUniforms.mvp );
	glm_mat4_mul( pClientObj->mUniforms.mvp, model, pClientObj->mUniforms.mvp );
}

static void recordGlCommands( struct ClientObjState* pClientObj, uint32_t time )
{    
    struct eglContext* pEglContext = &pClientObj->mpEglContext;
    struct GlState* pGlState = &pClientObj->mGlState;
    GLenum status;

    glBindFramebuffer(GL_FRAMEBUFFER, pClientObj->mGlState.msaaFBO);
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	
	if( status != GL_FRAMEBUFFER_COMPLETE )
	{
		printf("Failed to Setup FBO errorcode : %d at %d\n", status, __LINE__);
	}

	drawQuad( 
		pClientObj, time,
		0.0, 0.0, 0.0, 1.0,
		pQuadMesh->vertex_positions, pQuadMesh->vertex_texcoords, NULL,
		pQuadMesh->indices,
		pClientObj->mGlState.texture
	);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if( status != GL_FRAMEBUFFER_COMPLETE )
	{
		printf("Failed to Setup FBO errorcode : %d at %d\n", status, __LINE__);
	}
	drawQuad( 
		pClientObj, time,
		0.0, 0.0, 0.0, 1.0,
		pQuadMesh->vertex_positions, pQuadMesh->vertex_texcoords, NULL,
		pQuadMesh->indices,
		pClientObj->mGlState.msaaTexture
	);
}

static void drawTriangle(
	struct ClientObjState* pClientObj, uint32_t time,
	const void* position, const void* texcoords, const void* colors
)
{
	struct eglContext* pEglContext = &pClientObj->mpEglContext;
    struct GlState* pGlState = &pClientObj->mGlState;
	struct GfxPipeline* pGfxPipeline = &pClientObj->mGlState.vertexcolorPipeline;

	glViewport(0, 0, surfaceWidth, surfaceHeight );
	glClearColor( 0.1, 0.1, 0.4, 1.0 );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glDisable(GL_CULL_FACE);

	glUseProgram(pGfxPipeline->gpuprogram);

	glUniformMatrix4fv(
        pGfxPipeline->mvp_unifrom, 1, GL_FALSE,
        (GLfloat*) pClientObj->mUniforms.mvp
    );

    glVertexAttribPointer(
        pGfxPipeline->position_attribute,
        3, GL_FLOAT, GL_FALSE,
        0,
        position
    );
	glEnableVertexAttribArray(pGfxPipeline->position_attribute);
	if( texcoords )
	{
    	glVertexAttribPointer(
    	    pGfxPipeline->texcoord_attribute,
    	    2, GL_FLOAT, GL_FALSE,
    	    0,
    	    texcoords
    	);
		glEnableVertexAttribArray(pGfxPipeline->texcoord_attribute);
	}
	if( colors )
	{
    	glVertexAttribPointer(
    	    pGfxPipeline->color_attribute,
    	    3, GL_FLOAT, GL_FALSE,
    	    0,
    	    colors
    	);
		glEnableVertexAttribArray(pGfxPipeline->color_attribute);
	}

	glDrawArrays( GL_TRIANGLES, 0, 3 );

    glDisableVertexAttribArray(pGfxPipeline->position_attribute);
	if(texcoords)
    	glDisableVertexAttribArray(pGfxPipeline->texcoord_attribute);
	if(colors)
    	glDisableVertexAttribArray(pGfxPipeline->color_attribute);
}

static void drawQuad( 
	struct ClientObjState* pClientObj, uint32_t time,
	float clear_r, float clear_g, float clear_b, float clear_a,
	const void* position, const void* texcoords, const void* colors,
	const void* indices,
	GLuint textureToUse
)
{
	struct eglContext* pEglContext = &pClientObj->mpEglContext;
    struct GlState* pGlState = &pClientObj->mGlState;
	struct GfxPipeline* pGfxPipeline = &pClientObj->mGlState.renderToQuadPipeline;

	glViewport(0, 0, surfaceWidth, surfaceHeight );

	glClearColor(clear_r, clear_g, clear_b, clear_a);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(pGfxPipeline->gpuprogram);

	glUniformMatrix4fv(
        pGfxPipeline->mvp_unifrom, 1, GL_FALSE,
        (GLfloat*) pClientObj->mUniforms.quadIdentityModelViewProj 
    );
    glUniform1i( pGfxPipeline->texSampler_uniform, 0 );

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureToUse);

    glVertexAttribPointer(
        pGfxPipeline->position_attribute,
        2, GL_FLOAT, GL_FALSE,
        0,
        position
    );
	glEnableVertexAttribArray(pGfxPipeline->position_attribute);
	if( texcoords )
	{
    	glVertexAttribPointer(
    	    pGfxPipeline->texcoord_attribute,
    	    2, GL_FLOAT, GL_FALSE,
    	    0,
    	    texcoords
    	);
		glEnableVertexAttribArray(pGfxPipeline->texcoord_attribute);
	}
	if( colors )
	{
    	glVertexAttribPointer(
    	    pGfxPipeline->color_attribute,
    	    3, GL_FLOAT, GL_FALSE,
    	    0,
    	    colors
    	);
		glEnableVertexAttribArray(pGfxPipeline->color_attribute);
	}

	if( indices )
	{
    	glBindBuffer(
    	    GL_ELEMENT_ARRAY_BUFFER, pGlState->ibo
    	);
    	glBufferData(
    	    GL_ELEMENT_ARRAY_BUFFER,
    	    6 * sizeof(uint16_t),
    	    indices,
    	    GL_STATIC_DRAW
    	);
	}

    glDrawElements(
        GL_TRIANGLES, 6,
        GL_UNSIGNED_SHORT,
        0
    );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	//glBindTexture(GL_TEXTURE_2D, 0);

    glDisableVertexAttribArray(pGfxPipeline->position_attribute);
	if(texcoords)
    	glDisableVertexAttribArray(pGfxPipeline->texcoord_attribute);
	if(colors)
    	glDisableVertexAttribArray(pGfxPipeline->color_attribute);
}

static void InitGLState( struct GlState* pGLState )
{
	InitDebugMessenger();
	
	glFramebufferTexture2DMultisampleEXT = (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC) eglGetProcAddress( "glFramebufferTexture2DMultisampleEXT" );

	if( !glFramebufferTexture2DMultisampleEXT )
	{
		printf("Failed to get func pointer to glFramebufferTexture2DMultisampleIMG\n");
	}

    glMinSampleShadingOES = (PFNGLMINSAMPLESHADINGOESPROC) eglGetProcAddress( "glMinSampleShadingOES" );

    if( !glMinSampleShadingOES )
    {
        printf("Failed to get func pointer to glMinSampleShadingOES\n");
    }

	pGLState->vertexcolorPipeline.gpuprogram = createGPUProgram(
		vertex_color_vertex_shader_text, vertex_color_frag_shader_text
	);
    pGLState->vertexcolorPipeline.position_attribute = 0;
	pGLState->vertexcolorPipeline.texcoord_attribute = 1;
    pGLState->vertexcolorPipeline.color_attribute = 2;
    glBindAttribLocation(
		pGLState->vertexcolorPipeline.gpuprogram, 
		pGLState->vertexcolorPipeline.position_attribute,
		"pos"
	);
    glBindAttribLocation(
		pGLState->vertexcolorPipeline.gpuprogram, 
		pGLState->vertexcolorPipeline.color_attribute,
		"color"
	);
    glLinkProgram(pGLState->vertexcolorPipeline.gpuprogram);
    pGLState->vertexcolorPipeline.mvp_unifrom = glGetUniformLocation( pGLState->vertexcolorPipeline.gpuprogram, "mvp");

	pGLState->renderToQuadPipeline.gpuprogram = createGPUProgram(
		sample_texture_vertex_shader_text, sample_texture_frag_shader_text
	);
    pGLState->renderToQuadPipeline.position_attribute = 0;
	pGLState->renderToQuadPipeline.texcoord_attribute = 1;
    pGLState->renderToQuadPipeline.color_attribute = 2;
    glBindAttribLocation(
		pGLState->renderToQuadPipeline.gpuprogram, 
		pGLState->renderToQuadPipeline.position_attribute,
		"pos"
	);
    glBindAttribLocation(
		pGLState->renderToQuadPipeline.gpuprogram, 
		pGLState->renderToQuadPipeline.texcoord_attribute,
		"texcoord"
	);
    glLinkProgram(pGLState->renderToQuadPipeline.gpuprogram);
    pGLState->renderToQuadPipeline.mvp_unifrom = glGetUniformLocation( pGLState->renderToQuadPipeline.gpuprogram, "mvp");
	pGLState->renderToQuadPipeline.texSampler_uniform = glGetUniformLocation( pGLState->renderToQuadPipeline.gpuprogram, "texSampler");


    glGenBuffers( 1, &pGLState->ibo );

	GenerateTextureFromImage(
		texFileName,
		&texWidth, &texHeight,
		&pGLState->texture,
		GL_LINEAR, GL_LINEAR
	);

    GLboolean bParam;
    GLfloat fParam;
    GLint iParam;

    glEnable( GL_SAMPLE_SHADING_OES );
    glMinSampleShadingOES( minSampleShadingValue );

    glGetBooleanv( GL_SAMPLE_SHADING_OES, &bParam );
    glGetFloatv( GL_MIN_SAMPLE_SHADING_VALUE_OES, &fParam );
    glGetIntegerv( GL_MAX_SAMPLES_EXT, &iParam );

    numOfMSAAsamples = iClamp( numOfMSAAsamples, 0, iParam );

    printf("SAMPLE SHADING %s\n", bParam ? "Enabled" : "Disabled");
    printf("MIN SAMPLE SHADING VALUE %f\n", fParam);
}

static void SetupFBO( 
	GLuint* fbo,
	GLuint* colorAttachmentTexture,
	GLenum texture_format,
	GLenum pixelStorage,
	GLuint numOfSamples
)
{
	glGenTextures(1, colorAttachmentTexture);
	glBindTexture(GL_TEXTURE_2D, *colorAttachmentTexture);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		texture_format,
		surfaceWidth, surfaceHeight,
		0, texture_format,
		pixelStorage,
		NULL
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers( 1, fbo );
	glBindFramebuffer(GL_FRAMEBUFFER, *fbo);
	glFramebufferTexture2DMultisampleEXT(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, *colorAttachmentTexture,
		0,
		numOfSamples
	);
	glCheckError();

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if( status != GL_FRAMEBUFFER_COMPLETE )
	{
		printf("Failed to Setup FBO errorcode : %d\n", status);
	}

	GLint framebufferSamples = 0;
	glGetFramebufferAttachmentParameteriv(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_SAMPLES_EXT, &framebufferSamples
	);
	glCheckError();
	printf("FrameBuffer Texture Samples : %d\n", framebufferSamples);
	numOfMSAAsamples = framebufferSamples;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main( int argc, const char* argv[] )
{
    struct wl_display* pDisplay = NULL;

    size_t strBytes = 0;
    if( argc < 2 )
    {
        printf("No Filename Specified..\n");
        printf("Usage: <app> <texfilename> <msaaSampleCount> <minSampleShadingValue>\n");
        return 0;
    }
    else
    {
        strBytes = strlen(argv[1]);
		printf("Bytes : %d\n", strBytes);
        memcpy(texFileName, argv[1], strBytes );
		printf("%s %s\n", texFileName, argv[1]);
        if( argc > 3 )
        {
            numOfMSAAsamples = atoi( argv[2] );
            minSampleShadingValue = atof( argv[3] );
        }
        else if( argc > 2 && argc < 3 )
        {
            numOfMSAAsamples = atoi( argv[2] );
            minSampleShadingValue = MSAA_SAMPLE_SHADING;
        }
        else
        {
            numOfMSAAsamples = MSAA_SAMPLES;
            minSampleShadingValue = MSAA_SAMPLE_SHADING;
        }
    }

    struct ClientObjState clientObjState = {0};
	clientObjState.mpEglContext.mNativeDisplay = pDisplay;

	InitEGLContext( &clientObjState.mpEglContext );
	
	const char* surfaceTitle = "EGL Client";	
#if 0
	EGLint param;
	eglGetConfigAttrib( clientObjState.mpEglContext.mEglDisplay, clientObjState.mpEglContext.mEglConfig, EGL_MAX_PBUFFER_WIDTH, &param );
	surfaceWidth = param / 2;
	eglGetConfigAttrib( clientObjState.mpEglContext.mEglDisplay, clientObjState.mpEglContext.mEglConfig, EGL_MAX_PBUFFER_HEIGHT, &param );
	surfaceHeight = param / 2;
#endif

	printf("Creating Surface with dimensions %d x %d\n", surfaceWidth, surfaceHeight);
	CreateEGLSurface( surfaceWidth, surfaceHeight, &clientObjState.mpEglContext );
	InitGLState( &clientObjState.mGlState );
	SetupFBO(
		&clientObjState.mGlState.msaaFBO,
		&clientObjState.mGlState.msaaTexture,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		numOfMSAAsamples
	);
	pTriangleMesh = malloc(sizeof(struct Mesh));
	pQuadMesh = malloc(sizeof(struct Mesh));
	GetTriangleMesh(pTriangleMesh);
	GetQuadMesh(pQuadMesh);
	
	clientObjState.mbCloseApplication = 0;

	pixelDumpSizeInBytes = surfaceWidth * surfaceHeight * bytespp;
	pixelDump = malloc( pixelDumpSizeInBytes );

	printf("Loaded Texture from %s : Dimensions %d x %d\n", texFileName, texWidth, texHeight);
    printf("Rendering With MSAA %dx with %ff min sample shading\n", numOfMSAAsamples, minSampleShadingValue);
	simulation_start = clock();

    while( clientObjState.mbCloseApplication != 1 )
    {
        updateFrame_callback( &clientObjState, 0 );
    }

    char samplesStr[10];
    sprintf(samplesStr, "%d", numOfMSAAsamples);
    char minShadStr[10];
    sprintf(minShadStr, "%f", minSampleShadingValue);
    const char* extension = ".png";
    char fullFileName[250];
    sprintf(fullFileName, "SampleShading-%s-%s%s", samplesStr, minShadStr, extension);
	WritePixelsToFile(
		fullFileName,
		surfaceWidth, surfaceHeight,
		4,
		pixelDump
	);
	
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    glDeleteBuffers( 1, &clientObjState.mGlState.ibo );

	glBindBuffer( GL_FRAMEBUFFER, 0 );
	glDeleteFramebuffers( 1, &clientObjState.mGlState.msaaFBO );

	glBindTexture( GL_TEXTURE_2D, 0 );
	glDeleteTextures(1, &clientObjState.mGlState.msaaTexture );

	ShutdownEGLContext( &clientObjState.mpEglContext );
    
    printf("Client Disconnected from the Display\n");

    return 0;
}