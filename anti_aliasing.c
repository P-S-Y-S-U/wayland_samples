#define RENDERING_API EGL_OPENGL_ES2_BIT
//#define ENABLE_MSAA
#define MSAA_SAMPLES 4

#define IMAGE_FILE_PATH "./Text_Sample.png"

#include "egl_common.h"
#include "client_common.h"
#include "xdg_client_common.h"
#include "shm_helper.h"
#include "TexReader.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <cglm/mat4.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

struct ClientObjState;
struct GlState;

static const char* vertex_shader_text = 
    "uniform mat4 mvp;\n"
    "attribute vec4 pos;\n"
    "attribute vec2 texcoord;\n"
    "attribute vec4 color;\n"
    "varying vec2 out_texcoord;\n"
	"varying vec4 vertex_color;\n"
	"void main() {\n"
	" gl_Position = mvp * pos;\n"
    " out_texcoord = texcoord;\n"
	" vertex_color = color;\n"
	"}\n";

static const char* frag_shader_text = 
	"precision highp float;\n"
    "varying vec2 out_texcoord;\n"
	"varying vec4 vertex_color;\n"
    "uniform sampler2D texSampler;\n"
	"void main() {\n"
	"  gl_FragColor = texture2D(texSampler, out_texcoord);\n"
	"}\n";

static uint32_t startTime = 0;

static PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMGPROC glFramebufferTexture2DMultisampleIMG = NULL;

static void wl_buffer_release( void* pData, struct wl_buffer* pWlBuffer );

static void updateFrame_callback( void* pData, struct wl_callback* pFrameCallback, uint32_t time );
static void UpdateUniforms( struct ClientObjState* pClientObj );
static void recordGlCommands( struct ClientObjState* pClientObj, uint32_t time );

static void surface_configure_callback( void * pData, struct wl_callback* pCallback, uint32_t time );

static void InitGLState( struct GlState* pGLState );

static GLuint createShader( const char* source, GLenum shaderType );

struct ClientObjState
{
    struct GlobalObjectState* mpGlobalObjState;
    struct wl_surface* mpWlSurface;
    struct xdg_surface* mpXdgSurface;
    struct xdg_toplevel* mpXdgTopLevel;
	struct eglContext mpEglContext;
	struct wl_callback* mpFrameCallback;

	int8_t mbCloseApplication;
	int8_t mbSurfaceConfigured;

    struct GlState{
        GLuint mvp_unifrom;
        GLuint texSampler_uniform;
        GLuint position_attribute;
        GLuint texcoord_attribute;
        GLuint color_attribute;

        GLuint ibo;
        GLuint texture;
    }mGlState;

    struct Uniforms{
        mat4 mvp;
    }mUniforms;
};

static const struct wl_buffer_listener wl_buffer_listener = {
	.release = wl_buffer_release
};

static const struct wl_callback_listener frame_listener = {
	updateFrame_callback
};

static const struct wl_callback_listener configure_listener = {
	surface_configure_callback
};

static void wl_buffer_release( void* pData, struct wl_buffer* pWlBuffer )
{
	wl_buffer_destroy(pWlBuffer);
}

static void updateFrame_callback( void* pData, struct wl_callback* pFrameCallback, uint32_t time )
{
	struct ClientObjState* pClientObjState = pData;

	if( pFrameCallback != pClientObjState->mpFrameCallback )
	{
		printf("Frame Callback Not Synced\n");
		return;
	}
	pClientObjState->mpFrameCallback = NULL;

	if( pFrameCallback )
		wl_callback_destroy(pFrameCallback);

	if( !pClientObjState->mbSurfaceConfigured )
		return;

    UpdateUniforms( pClientObjState );
	recordGlCommands( pClientObjState, time );

	pClientObjState->mpFrameCallback = wl_surface_frame( pClientObjState->mpWlSurface );
	wl_callback_add_listener( pClientObjState->mpFrameCallback, &frame_listener, pClientObjState );

	SwapEGLBuffers( &pClientObjState->mpEglContext );
}

static void UpdateUniforms( struct ClientObjState* pClientObj )
{
    glm_mat4_identity( pClientObj->mUniforms.mvp );
}

static void recordGlCommands( struct ClientObjState* pClientObj, uint32_t time )
{
#if 0
	if( startTime == 0 )
		startTime = time;

	srand( time - startTime );

	float red = ( ( rand() % ( 255 - 1 + 1 ) ) + 1 ) / 255.0;
	float green = ( ( rand() % ( 255 - 1 + 1 ) ) + 1 ) / 255.0;
	float blue = ( ( rand() % ( 255 - 1 + 1 ) ) + 1 ) / 255.0;
	float alpha = ( ( rand() % ( 255 - 0 + 1 ) ) + 0 ) / 255.0;	
#else
	float red = 0.25;
	float green = 0.0;
	float blue = 0.65;
	float alpha = 1.0;
#endif
    
    struct eglContext* pEglContext = &pClientObj->mpEglContext;
    struct GlState* pGlState = &pClientObj->mGlState;

	glViewport(0, 0, pEglContext->mWindowWidth, pEglContext->mWindowHeight );
    glUniformMatrix4fv(
        pGlState->mvp_unifrom, 1, GL_FALSE,
        (GLfloat*) pClientObj->mUniforms.mvp
    );
    glUniform1i( pClientObj->mGlState.texSampler_uniform, 0 );
    glCheckError();
    
    glCheckError();

	glClearColor(red, green, blue, alpha);
	glClear(GL_COLOR_BUFFER_BIT);

    static const float vertex_scale = 0.95;
    static const float uv_scale = 1.0;

    static const float vertex_positions[4][2] = {
        -vertex_scale, vertex_scale,
	    -vertex_scale, -vertex_scale,
	    vertex_scale, -vertex_scale,
	    vertex_scale, vertex_scale
    };

    static const float vertex_texcoords[4][2] = {
        0.0, uv_scale,
	    0.0, 0.0,
	    uv_scale, 0.0,
	    uv_scale, uv_scale
    };

    static const float vertex_colors[4][3] = {
    	1.0, 1.0, 0.0,
    	0.0, 1.0, 0.0,
    	0.0, 0.5, 1.0,
        1.0, 0.0, 1.0,
    };

    static const uint16_t indices[6] = {
        0, 1, 2,
        0, 2, 3
    };

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pGlState->texSampler_uniform);

    glVertexAttribPointer(
        pGlState->position_attribute,
        2, GL_FLOAT, GL_FALSE,
        0,
        vertex_positions
    );
    glVertexAttribPointer(
        pGlState->texcoord_attribute,
        2, GL_FLOAT, GL_FALSE,
        0,
        vertex_texcoords
    );
    glVertexAttribPointer(
        pGlState->color_attribute,
        3, GL_FLOAT, GL_FALSE,
        0,
        vertex_colors
    );
    glEnableVertexAttribArray(pGlState->position_attribute);
    glEnableVertexAttribArray(pGlState->texcoord_attribute);
    glEnableVertexAttribArray(pGlState->color_attribute);

    glBindBuffer(
        GL_ELEMENT_ARRAY_BUFFER, pGlState->ibo
    );
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        6 * sizeof(uint16_t),
        indices,
        GL_STATIC_DRAW
    );

	// glDrawArrays( GL_TRIANGLES, 0, 3 );
    glDrawElements(
        GL_TRIANGLES, 6,
        GL_UNSIGNED_SHORT,
        0
    );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

    glDisableVertexAttribArray(pGlState->position_attribute);
    glDisableVertexAttribArray(pGlState->texcoord_attribute);
    glDisableVertexAttribArray(pGlState->color_attribute);
}

static void surface_configure_callback( void* pData, struct wl_callback* pCallback, uint32_t time )
{
	struct ClientObjState* pClientObj = pData;
	
	wl_callback_destroy( pCallback );

	pClientObj->mbSurfaceConfigured = 1;

	if( pClientObj->mpFrameCallback == NULL )
		updateFrame_callback( pClientObj, NULL, time );
}

static void InitGLState( struct GlState* pGLState )
{
	glFramebufferTexture2DMultisampleIMG = (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMGPROC) eglGetProcAddress( "glFramebufferTexture2DMultisampleIMG" );

	if( !glFramebufferTexture2DMultisampleIMG )
	{
		printf("Failed to get func pointer to glFramebufferTexture2DMultisampleIMG\n");
	}

	GLuint frag, vert;
	GLuint program;
	GLint status;

	frag = createShader( frag_shader_text, GL_FRAGMENT_SHADER);
	vert = createShader( vertex_shader_text, GL_VERTEX_SHADER);

	program = glCreateProgram();
	glAttachShader(program, frag);
	glAttachShader(program, vert);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (!status) {
		char log[1000];
		GLsizei len;
		glGetProgramInfoLog(program, 1000, &len, log);
		fprintf(stderr, "Error: linking:\n%*s\n", len, log);
		exit(1);
	}

	glUseProgram(program);

    pGLState->position_attribute = 0;
    pGLState->texcoord_attribute = 1;
    pGLState->color_attribute = 2;

    glBindAttribLocation(program, pGLState->position_attribute, "pos");
    glBindAttribLocation(program, pGLState->texcoord_attribute, "tex");
    glBindAttribLocation(program, pGLState->color_attribute, "color");
    glLinkProgram(program);

    pGLState->mvp_unifrom = glGetUniformLocation( program, "mvp");
    pGLState->texSampler_uniform = glGetUniformLocation( program, "texSampler" );

    glGenBuffers( 1, &pGLState->ibo );

    int texWidth, texHeight, channels;

    stbi_set_flip_vertically_on_load(1);

    uint8_t* imgData = stbi_load(
        IMAGE_FILE_PATH, &texWidth, &texHeight, &channels, STBI_rgb_alpha
    );

    glGenTextures( 1, &pGLState->texture );
    glBindTexture(GL_TEXTURE_2D, pGLState->texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(
        GL_TEXTURE_2D, 0, 
        GL_RGBA, 
        texWidth, texHeight, 0, 
        GL_RGBA, 
        GL_UNSIGNED_BYTE,
        imgData
    );

    glCheckError();

    stbi_image_free(imgData);
}

static GLuint createShader( const char* source, GLenum shaderType )
{
	GLuint shader;
	GLint status;

	shader = glCreateShader(shaderType);

	glShaderSource(shader, 1, (const char **) &source, NULL);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (!status) {
		char log[1000];
		GLsizei len;
		glGetShaderInfoLog(shader, 1000, &len, log);
		fprintf(stderr, "Error: compiling %s: %*s\n",
			shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment",
			len, log);
		exit(1);
	}

	return shader;
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

    AssignXDGSurfaceListener( clientObjState.mpXdgSurface, &clientObjState );

	uint16_t surfaceWidth = 1920;
	uint16_t surfaceHeight = 1080;
	const char* surfaceTitle = "EGL Client";

	CreateEGLSurface( clientObjState.mpWlSurface, surfaceWidth, surfaceHeight, &clientObjState.mpEglContext );
	InitGLState( &clientObjState.mGlState );

    clientObjState.mpXdgTopLevel = xdg_surface_get_toplevel( clientObjState.mpXdgSurface );
	AssignXDGToplevelListener(clientObjState.mpXdgTopLevel, &clientObjState);
    xdg_toplevel_set_title(clientObjState.mpXdgTopLevel, surfaceTitle);
	//xdg_toplevel_handle_configure( &clientObjState, clientObjState.mpXdgTopLevel, surfaceWidth, surfaceHeight, NULL );
	//xdg_toplevel_set_fullscreen( clientObjState.mpXdgTopLevel, clientObjState.mpGlobalObjState->mpOutput );
	
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

	ShutdownEGLContext( &clientObjState.mpEglContext, clientObjState.mpXdgTopLevel, clientObjState.mpXdgSurface, clientObjState.mpWlSurface );
    wl_display_disconnect(pDisplay);
    printf("Client Disconnected from the Display\n");

    return 0;
}