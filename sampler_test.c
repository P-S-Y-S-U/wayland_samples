#define RENDERING_API EGL_OPENGL_ES2_BIT
#define MSAA_SAMPLES 16
//#define ENABLE_MSAA

#include "egl_common.h"
#ifndef WAYLAND_HEADLESS
#include "client_common.h"
#include "xdg_client_common.h"
#endif
#include "TexReader.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

static struct Mesh* pTriangleMesh = NULL;
static struct Mesh* pQuadMesh = NULL;

static clock_t simulation_start;

const uint16_t bytespp = 4;
uint8_t initialFrameCallbackDone = 0;
uint8_t SurfacePresented = 0;
uint8_t imgReadJob = 0;
char argTexfile[256];
int texImgWidth;
int texImgHeight;

#ifndef WAYLAND_HEADLESS
static void updateFrame_callback( void* pData, struct wl_callback* pFrameCallback, uint32_t time );
#else
static void updateFrame_callback( void* pData, uint32_t time );
#endif

static void UpdateUniforms( struct ClientObjState* pClientObj );
static void recordGlCommands( struct ClientObjState* pClientObj, uint32_t time );

static void drawQuad( 
	struct ClientObjState* pClientObj, struct GfxPipeline* pGfxpipeline, uint32_t time,
	float clear_r, float clear_g, float clear_b, float clear_a,
	const void* position, const void* texcoords, const void* colors,
	const void* indices,
	GLuint textureToUse
);

#ifndef WAYLAND_HEADLESS
static void surface_configure_callback( void * pData, struct wl_callback* pCallback, uint32_t time );
#endif

static void InitGLState( struct GlState* pGLState );

static void SetupFBO( 
	GLuint* fbo,
	GLuint* colorAttachmentTexture,
	GLenum texture_format,
	GLenum pixelStorage
);

struct GfxPipeline{
	GLuint gpuprogram;
	GLuint mvp_unifrom;
	GLuint texSampler_uniform;
	GLuint inverseScreenSize_uniform;
    GLuint subpixel_uniform;
    GLuint edgeThreshold_uniform;
    GLuint edgeThresholdMin_uniform;
	GLuint position_attribute;
	GLuint texcoord_attribute;
	GLuint color_attribute;
};

struct ClientObjState
{
#ifndef WAYLAND_HEADLESS
    struct GlobalObjectState* mpGlobalObjState;
    struct wl_surface* mpWlSurface;
    struct xdg_surface* mpXdgSurface;
    struct xdg_toplevel* mpXdgTopLevel;
	struct wl_callback* mpFrameCallback;
#endif

	struct eglContext mpEglContext;

	int8_t mbCloseApplication;
	int8_t mbSurfaceConfigured;

    struct GlState{
        GLuint ibo;
        GLuint meshTexture;

		GLuint sceneFBO;
		GLuint sceneTexture;
		
		struct GfxPipeline renderToQuadPipeline;
		struct GfxPipeline samplerTestPipeline;
    }mGlState;

    struct Uniforms{
		mat4 quadIdentityModelViewProj;
    }mUniforms;
};

#ifdef WAYLAND_HEADLESS
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

		DownloadPixelsFromGPU(
			TEX_ID,
			FBO,
			0, 0,
			surfaceWidth, surfaceHeight,
			pClientObjState->mGlState.sceneTexture,
			GL_RGBA,
			0,
			bytespp,
			BeforeFXAA_pixelDump, BeforeFXAA_pixelDumpSizeInBytes
		);

		imgReadJob = DownloadPixelsFromGPU(
			DEFAULT_FRAME_BUFFER,
			FBO,
			0, 0,
			surfaceWidth, surfaceHeight,
			-1,
			GL_RGBA,
			0,
			bytespp,
			AfterFXAA_pixelDump, AfterFXAA_pixelDumpSizeInBytes
		);
	}
	else
	{
		initialFrameCallbackDone = 1;
	}

	SwapEGLBuffers( &pClientObjState->mpEglContext );
}

#else 
static const struct wl_callback_listener frame_listener = {
	updateFrame_callback
};

static const struct wl_callback_listener configure_listener = {
	surface_configure_callback
};
static void updateFrame_callback( void* pData, struct wl_callback* pFrameCallback, uint32_t time )
{
	struct ClientObjState* pClientObjState = pData;

    if( pFrameCallback != pClientObjState->mpFrameCallback )
	{
		printf("Frame Callback Not Synced\n");
		return;
	}
	pClientObjState->mpFrameCallback = NULL;

    UpdateUniforms( pClientObjState );
	recordGlCommands( pClientObjState, time );

	if( initialFrameCallbackDone && !imgReadJob )
	{
		SurfacePresented = 1;
	}
	else
	{
		initialFrameCallbackDone = 1;
	}

    pClientObjState->mpFrameCallback = wl_surface_frame( pClientObjState->mpWlSurface );
	wl_callback_add_listener( pClientObjState->mpFrameCallback, &frame_listener, pClientObjState );

	SwapEGLBuffers( &pClientObjState->mpEglContext );
}

static void surface_configure_callback( void* pData, struct wl_callback* pCallback, uint32_t time )
{
	struct ClientObjState* pClientObj = pData;
	
	wl_callback_destroy( pCallback );

	pClientObj->mbSurfaceConfigured = 1;

	if( pClientObj->mpFrameCallback == NULL )
		updateFrame_callback( pClientObj, NULL, time );
}

static void xdg_surface_configure(
	void* pData, struct xdg_surface* pXdgSurface, uint32_t serial
)
{
    struct ClientObjState* pClientObjState = pData;
	xdg_surface_ack_configure(pXdgSurface, serial);
}

static void xdg_toplevel_handle_configure(
	void* pData,
	struct xdg_toplevel* pXdgToplevel,
	int32_t width, int32_t height,
	struct wl_array* pStates
)
{
	// no window geometry event, should ignore
	if( width == 0 && height == 0 ) return;

	struct ClientObjState* pClientObjState = pData;

	if( pClientObjState->mpEglContext.mWindowWidth == width && pClientObjState->mpEglContext.mWindowHeight == height )
	{
		return;
	}

	pClientObjState->mpEglContext.mWindowWidth = width;
	pClientObjState->mpEglContext.mWindowHeight = height;

	// resizecontext_attribs
	wl_egl_window_resize( pClientObjState->mpEglContext.mNativeWindow, width, height, 0, 0 );
}

static void xdg_toplevel_handle_close(
    void* pData,
    struct xdg_toplevel* pXdgTopLevel
)
{
	struct ClientObjState* pClientObjState = pData;
	pClientObjState->mbCloseApplication = 1;
}

#endif 

static void UpdateUniforms( struct ClientObjState* pClientObj )
{
	mat4 model, view, projection;

	glm_mat4_identity(model);
	glm_mat4_identity(view);
	glm_mat4_identity(projection);
	glm_mat4_identity( pClientObj->mUniforms.quadIdentityModelViewProj );
}

static void recordGlCommands( struct ClientObjState* pClientObj, uint32_t time )
{    
    struct eglContext* pEglContext = &pClientObj->mpEglContext;
    struct GlState* pGlState = &pClientObj->mGlState;
    GLenum status;

    glBindFramebuffer(GL_FRAMEBUFFER, pClientObj->mGlState.sceneFBO);
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	
	if( status != GL_FRAMEBUFFER_COMPLETE )
	{
		printf("Failed to Setup FBO errorcode : %d at %d\n", status, __LINE__);
	}

	drawQuad( 
		pClientObj, &pClientObj->mGlState.renderToQuadPipeline, time,
		0.0, 0.0, 0.0, 1.0,
		pQuadMesh->vertex_positions, pQuadMesh->vertex_texcoords, NULL,
		pQuadMesh->indices,
		pClientObj->mGlState.meshTexture
	);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if( status != GL_FRAMEBUFFER_COMPLETE )
	{
		printf("Failed to Setup FBO errorcode : %d at %d\n", status, __LINE__);
	}

    drawQuad( 
		pClientObj, &pClientObj->mGlState.samplerTestPipeline, time,
		0.0, 0.0, 0.0, 1.0,
		pQuadMesh->vertex_positions, pQuadMesh->vertex_texcoords, NULL,
		pQuadMesh->indices,
		pClientObj->mGlState.meshTexture
	);
}

static void drawQuad( 
	struct ClientObjState* pClientObj, struct GfxPipeline* pGfxPipeline, uint32_t time,
	float clear_r, float clear_g, float clear_b, float clear_a,
	const void* position, const void* texcoords, const void* colors,
	const void* indices,
	GLuint textureToUse
)
{
	struct eglContext* pEglContext = &pClientObj->mpEglContext;
    struct GlState* pGlState = &pClientObj->mGlState;

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
	
    char* vertex_shader;
    char* fragment_shader;
    size_t bufSize;

    ReadFileContentsToCpuBuffer(
        "shaders/sample_texture.vert",
        &vertex_shader,
        &bufSize
    );

    ReadFileContentsToCpuBuffer(
        "shaders/sample_texture.frag",
        &fragment_shader,
        &bufSize
    );

	pGLState->renderToQuadPipeline.gpuprogram = createGPUProgram(
		vertex_shader, fragment_shader
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

    free(fragment_shader);
    
    ReadFileContentsToCpuBuffer(
        "shaders/sampler_test.frag",
        &fragment_shader,
        &bufSize
    );

	pGLState->samplerTestPipeline.gpuprogram = createGPUProgram(
		vertex_shader, fragment_shader
	);
    pGLState->samplerTestPipeline.position_attribute = 0;
	pGLState->samplerTestPipeline.texcoord_attribute = 1;
    pGLState->samplerTestPipeline.color_attribute = 2;
    glBindAttribLocation(
		pGLState->samplerTestPipeline.gpuprogram, 
		pGLState->samplerTestPipeline.position_attribute,
		"pos"
	);
    glBindAttribLocation(
		pGLState->samplerTestPipeline.gpuprogram, 
		pGLState->samplerTestPipeline.texcoord_attribute,
		"texcoord"
	);
    glLinkProgram(pGLState->samplerTestPipeline.gpuprogram);
    pGLState->samplerTestPipeline.mvp_unifrom = glGetUniformLocation( pGLState->samplerTestPipeline.gpuprogram, "mvp");
	pGLState->samplerTestPipeline.texSampler_uniform = glGetUniformLocation( pGLState->samplerTestPipeline.gpuprogram, "texSampler");

    free(vertex_shader);
    free(fragment_shader);

    glGenBuffers( 1, &pGLState->ibo );

    GenerateTextureFromImage(
        argTexfile,
        &texImgWidth, &texImgHeight,
        &pGLState->meshTexture,
        GL_NEAREST, GL_NEAREST
    );
}

static void SetupFBO( 
	GLuint* fbo,
	GLuint* colorAttachmentTexture,
	GLenum texture_format,
	GLenum pixelStorage
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers( 1, fbo );
	glBindFramebuffer(GL_FRAMEBUFFER, *fbo);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,  GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, *colorAttachmentTexture,
        0
    );
	glCheckError();

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if( status != GL_FRAMEBUFFER_COMPLETE )
	{
		printf("Failed to Setup FBO errorcode : %d\n", status);
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ParseCmdArgs( int argc, const char* argv[] )
{
	if( argc < 2 )
	{
		printf("<pgmName> <texfilename>\n");
		exit(EXIT_FAILURE);
	}

    size_t strLen = strlen(argv[1]);
    memcpy(argTexfile, argv[1], strLen);
}

#ifdef WAYLAND_HEADLESS
int main( int argc, const char* argv[] )
{
	ParseCmdArgs( argc, argv );

    struct wl_display* pDisplay = NULL;

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
		&clientObjState.mGlState.sceneFBO,
		&clientObjState.mGlState.sceneTexture,
		GL_RGBA,
		GL_UNSIGNED_BYTE
	);
	pTriangleMesh = malloc(sizeof(struct Mesh));
	pQuadMesh = malloc(sizeof(struct Mesh));
	GetTriangleMesh(pTriangleMesh);
	GetQuadMesh(pQuadMesh);
	
	clientObjState.mbCloseApplication = 0;

	AfterFXAA_pixelDumpSizeInBytes = surfaceWidth * surfaceHeight * bytespp;
	AfterFXAA_pixelDump = malloc( AfterFXAA_pixelDumpSizeInBytes );

	BeforeFXAA_pixelDumpSizeInBytes = surfaceWidth * surfaceHeight * bytespp;
	BeforeFXAA_pixelDump = malloc( BeforeFXAA_pixelDumpSizeInBytes );

	simulation_start = clock();

	printf("Performing FXAA Sample on Settings: \n");
	printf("		subpixel : %f \n", argSubPixel);
	printf("		edgeThreshold : %f \n", argEdgeThreshold);
	printf("		edgeThresholdMin : %f \n", argEdgeThresholdMin);

    while( clientObjState.mbCloseApplication != 1 )
    {
        updateFrame_callback( &clientObjState, 0 );
    }

    const char* extension = ".png";
    char fullFileName[125];
    sprintf(fullFileName, "AfterFXAA-Tex-%f-%f-%f%s", argSubPixel, argEdgeThreshold, argEdgeThresholdMin, extension);
	WritePixelsToFile(
		fullFileName,
		surfaceWidth, surfaceHeight,
		4,
		1,
		AfterFXAA_pixelDump
	);

	sprintf(fullFileName, "BeforeFXAA-Tex-%f-%f-%f%s", argSubPixel, argEdgeThreshold, argEdgeThresholdMin, extension);
	WritePixelsToFile(
		fullFileName,
		surfaceWidth, surfaceHeight,
		4,
		1,
		BeforeFXAA_pixelDump
	);

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    glDeleteBuffers( 1, &clientObjState.mGlState.ibo );

	glBindBuffer( GL_FRAMEBUFFER, 0 );
	glDeleteFramebuffers( 1, &clientObjState.mGlState.sceneFBO );

	glBindTexture( GL_TEXTURE_2D, 0 );
	glDeleteTextures(1, &clientObjState.mGlState.sceneTexture );

	ShutdownEGLContext( &clientObjState.mpEglContext );
    
    printf("Client Disconnected from the Display\n");

    return 0;
}
#else
int main( int argc, const char* argv[] )
{
	ParseCmdArgs( argc, argv );
    struct wl_display* pDisplay = wl_display_connect(NULL);

    if( !pDisplay )
    {
        printf("Failed to make display connection\n");
    }
    printf("Connection Established with Wayland Display\n");

    struct GlobalObjectState gObjState = {0};

    retrieve_session_global( pDisplay, &gObjState );

    if(	!gObjState.mpOutput || !gObjState.mpCompositor ||
		!gObjState.mpShm || !gObjState.mpXdgWmBase
	)
	{
		printf("Failed to retrieve global objects\n");
		return 1;
	}

    struct ClientObjState clientObjState = {0};
	clientObjState.mpEglContext.mNativeDisplay = pDisplay;
    clientObjState.mpGlobalObjState = &gObjState;

	InitEGLContext( &clientObjState.mpEglContext );
	
	const char* surfaceTitle = "EGL Client";

    clientObjState.mpWlSurface = wl_compositor_create_surface(gObjState.mpCompositor);
	if( !clientObjState.mpWlSurface )
	{
		printf("Failed to Create Wayland Surface\n");
		return 1;
	}

	clientObjState.mpXdgSurface = xdg_wm_base_get_xdg_surface(
		clientObjState.mpGlobalObjState->mpXdgWmBase, clientObjState.mpWlSurface
	);
	if( !clientObjState.mpXdgSurface )
	{
		printf("Failed to create Xdg Surface\n");
		return 1;
	}
	printf("Creating Surface with dimensions %d x %d\n", surfaceWidth, surfaceHeight);

	CreateEGLSurface( clientObjState.mpWlSurface, surfaceWidth, surfaceHeight, &clientObjState.mpEglContext );

	InitGLState( &clientObjState.mGlState );
	SetupFBO(
		&clientObjState.mGlState.sceneFBO,
		&clientObjState.mGlState.sceneTexture,
		GL_RGBA,
		GL_UNSIGNED_BYTE
	);
	pTriangleMesh = malloc(sizeof(struct Mesh));
	pQuadMesh = malloc(sizeof(struct Mesh));
	GetTriangleMesh(pTriangleMesh);
	GetQuadMesh(pQuadMesh);
	
    clientObjState.mpXdgTopLevel = xdg_surface_get_toplevel( clientObjState.mpXdgSurface );
	AssignXDGToplevelListener(clientObjState.mpXdgTopLevel, &clientObjState);
    xdg_toplevel_set_title(clientObjState.mpXdgTopLevel, surfaceTitle);
	
	struct wl_callback* configureCallback;

	configureCallback = wl_display_sync( pDisplay );
	wl_callback_add_listener( configureCallback, &configure_listener, &clientObjState );

	clientObjState.mbCloseApplication = 0;

    while( clientObjState.mbCloseApplication != 1 )
    {
        wl_display_dispatch(pDisplay);
    }

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    glDeleteBuffers( 1, &clientObjState.mGlState.ibo );

	glBindBuffer( GL_FRAMEBUFFER, 0 );
	glDeleteFramebuffers( 1, &clientObjState.mGlState.sceneFBO );

	glBindTexture( GL_TEXTURE_2D, 0 );
	glDeleteTextures(1, &clientObjState.mGlState.sceneTexture );

	ShutdownEGLContext( &clientObjState.mpEglContext, clientObjState.mpXdgTopLevel, clientObjState.mpXdgSurface, clientObjState.mpWlSurface );
    wl_display_disconnect(pDisplay);
    printf("Client Disconnected from the Display\n");

    return 0;
}
#endif

