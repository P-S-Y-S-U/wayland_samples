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
#include <time.h>
#include <math.h>

#include <cglm/cglm.h>
#include <cglm/mat4.h>

#define USE_STB
#define STB_IMAGE_WRITE_IMPLEMENTATION

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

void* pixelDump = NULL;
size_t pixelDumpSizeInBytes;
const uint16_t bytespp = 4;
uint8_t initialFrameCallbackDone = 0;
uint8_t SurfacePresented = 0;
uint8_t imgReadJob = 0;

#ifndef WAYLAND_HEADLESS
static void updateFrame_callback( void* pData, struct wl_callback* pFrameCallback, uint32_t time );
#else
static void updateFrame_callback( void* pData, uint32_t time );
#endif

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
static void drawFXAAPass(
	struct ClientObjState* pClientObj, uint32_t time,
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

		GLuint sceneFBO;
		GLuint sceneTexture;
		
		GLuint fxaaFBO;
		GLuint fxaaTexture;

		struct GfxPipeline vertexcolorPipeline;
		struct GfxPipeline renderToQuadPipeline;
		struct GfxPipeline fxaaPipeline;
    }mGlState;

    struct Uniforms{
        mat4 mvp;
		mat4 quadIdentityModelViewProj;
		vec2 inverseScreenSize; 
        float subpixel;
        float edgeThreshold;
        float edgeThresholdMin;
    }mUniforms;
};

#ifdef WAYLAND_HEADLESS
static void updateFrame_callback( void* pData, uint32_t time )
{
	struct ClientObjState* pClientObjState = pData;

	//if( imgReadJob )
	//	pClientObjState->mbCloseApplication = 1;

    UpdateUniforms( pClientObjState );
	recordGlCommands( pClientObjState, time );

	if( initialFrameCallbackDone && !imgReadJob )
	{
		SurfacePresented = 1;
#if 0		
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
#endif
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

	//if( imgReadJob )
	//	pClientObjState->mbCloseApplication = 1;

    UpdateUniforms( pClientObjState );
	recordGlCommands( pClientObjState, time );

	if( initialFrameCallbackDone && !imgReadJob )
	{
		SurfacePresented = 1;
#if 0		
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
#endif
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

	pClientObj->mUniforms.inverseScreenSize[0] = (float) surfaceWidth;
	pClientObj->mUniforms.inverseScreenSize[1] = (float) surfaceHeight;

    pClientObj->mUniforms.subpixel = 0.75;
    pClientObj->mUniforms.edgeThreshold = 0.125;
    pClientObj->mUniforms.edgeThresholdMin = 0.0312;
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

	drawTriangle( 
		pClientObj, time,
		pTriangleMesh->vertex_positions, pTriangleMesh->vertex_texcoords, pTriangleMesh->vertex_colors
	);

#if 0
	glBindFramebuffer(GL_FRAMEBUFFER, pClientObj->mGlState.fxaaFBO);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if( status != GL_FRAMEBUFFER_COMPLETE )
	{
		printf("Failed to Setup FBO errorcode : %d at %d\n", status, __LINE__);
	}
	drawFXAAPass( 
		pClientObj, time,
		pQuadMesh->vertex_positions, pQuadMesh->vertex_texcoords, NULL,
		pQuadMesh->indices,
		pClientObj->mGlState.sceneTexture
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
		pClientObj->mGlState.fxaaTexture
	);
#else
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if( status != GL_FRAMEBUFFER_COMPLETE )
	{
		printf("Failed to Setup FBO errorcode : %d at %d\n", status, __LINE__);
	}
	drawFXAAPass( 
		pClientObj, time,
		pQuadMesh->vertex_positions, pQuadMesh->vertex_texcoords, NULL,
		pQuadMesh->indices,
		pClientObj->mGlState.sceneTexture
	);
#endif 
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

static void drawFXAAPass(
	struct ClientObjState* pClientObj, uint32_t time,
	const void* position, const void* texcoords, const void* colors,
	const void* indices,
	GLuint textureToUse
)
{

	struct eglContext* pEglContext = &pClientObj->mpEglContext;
    struct GlState* pGlState = &pClientObj->mGlState;
	struct GfxPipeline* pGfxPipeline = &pClientObj->mGlState.fxaaPipeline;

	glViewport(0, 0, surfaceWidth, surfaceHeight );

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(pGfxPipeline->gpuprogram);

	glUniformMatrix4fv(
        pGfxPipeline->mvp_unifrom, 1, GL_FALSE,
        (GLfloat*) pClientObj->mUniforms.quadIdentityModelViewProj 
    );
    glUniform1i( pGfxPipeline->texSampler_uniform, 0 );
	glUniform2fv( pGfxPipeline->inverseScreenSize_uniform, 1,
		(GLfloat*) pClientObj->mUniforms.inverseScreenSize
	);
    glUniform1f( pGfxPipeline->subpixel_uniform, pClientObj->mUniforms.subpixel );
    glUniform1f( pGfxPipeline->edgeThreshold_uniform, pClientObj->mUniforms.edgeThreshold );
    glUniform1f( pGfxPipeline->edgeThresholdMin_uniform, pClientObj->mUniforms.edgeThresholdMin );

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
        "shaders/vertex_color.vert",
        &vertex_shader,
        &bufSize
    );

    ReadFileContentsToCpuBuffer(
        "shaders/vertex_color.frag",
        &fragment_shader,
        &bufSize
    );

	pGLState->vertexcolorPipeline.gpuprogram = createGPUProgram(
		vertex_shader, fragment_shader
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

    free(vertex_shader);
    free(fragment_shader);

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


	free(vertex_shader);
    free(fragment_shader);

    ReadFileContentsToCpuBuffer(
        "shaders/fxaa_org.vert",
        &vertex_shader,
        &bufSize
    );

    ReadFileContentsToCpuBuffer(
        "shaders/fxaa_org.frag",
        &fragment_shader,
        &bufSize
    );

	pGLState->fxaaPipeline.gpuprogram = createGPUProgram(
		vertex_shader, fragment_shader
	);
    pGLState->fxaaPipeline.position_attribute = 0;
	pGLState->fxaaPipeline.texcoord_attribute = 1;
    pGLState->fxaaPipeline.color_attribute = 2;
    glBindAttribLocation(
		pGLState->fxaaPipeline.gpuprogram, 
		pGLState->fxaaPipeline.position_attribute,
		"pos"
	);
    glBindAttribLocation(
		pGLState->fxaaPipeline.gpuprogram, 
		pGLState->fxaaPipeline.texcoord_attribute,
		"texcoord"
	);
    glLinkProgram(pGLState->fxaaPipeline.gpuprogram);
    pGLState->fxaaPipeline.mvp_unifrom = glGetUniformLocation( pGLState->fxaaPipeline.gpuprogram, "mvp");
	pGLState->fxaaPipeline.texSampler_uniform = glGetUniformLocation( pGLState->fxaaPipeline.gpuprogram, "u_texture");
	pGLState->fxaaPipeline.inverseScreenSize_uniform = glGetUniformLocation( pGLState->fxaaPipeline.gpuprogram, "g_Resolution" );
    pGLState->fxaaPipeline.subpixel_uniform= glGetUniformLocation(pGLState->fxaaPipeline.gpuprogram, "m_Subpix");
    pGLState->fxaaPipeline.edgeThreshold_uniform = glGetUniformLocation(pGLState->fxaaPipeline.gpuprogram, "m_EdgeThreshold");
    pGLState->fxaaPipeline.edgeThresholdMin_uniform = glGetUniformLocation(pGLState->fxaaPipeline.gpuprogram, "m_EdgeThresholdMin");

    glGenBuffers( 1, &pGLState->ibo );

    free(vertex_shader);
    free(fragment_shader);
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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

#ifdef WAYLAND_HEADLESS
int main( int argc, const char* argv[] )
{
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

	pixelDumpSizeInBytes = surfaceWidth * surfaceHeight * bytespp;
	pixelDump = malloc( pixelDumpSizeInBytes );

	simulation_start = clock();

    while( clientObjState.mbCloseApplication != 1 )
    {
        updateFrame_callback( &clientObjState, 0 );
    }

#if 0
    char samplesStr[10];
    sprintf(samplesStr, "%d", numOfMSAAsamples);
    const char* file = "FXAA";
    const char* extension = ".png";
    char fullFileName[125];
    sprintf(fullFileName, "%s-%s%s", file, samplesStr, extension);
	WritePixelsToFile(
		fullFileName,
		surfaceWidth, surfaceHeight,
		4,
		pixelDump
	);
#endif

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
	SetupFBO(
		&clientObjState.mGlState.fxaaFBO,
		&clientObjState.mGlState.fxaaTexture,
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

	pixelDumpSizeInBytes = surfaceWidth * surfaceHeight * bytespp;
	pixelDump = malloc( pixelDumpSizeInBytes );

	simulation_start = clock();

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

