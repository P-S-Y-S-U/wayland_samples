
#include "egl_common.h"
#include "client_common.h"
#include "xdg_client_common.h"
#include "shm_helper.h"

#include <GLES2/gl2.h>
#include <stdlib.h>
#include <time.h>

struct ClientObjState;

static const char* vertex_shader_text = 
	"#version 300 es\n"
	"out vec4 vertex_color;\n"
	"const vec2 positions[3] = vec2[3](\n"
		"vec2(0.0, -0.5),\n"
		"vec2(0.5, 0.5),\n"
		"vec2(-0.5, 0.5)\n"
	");\n"
	"const vec3 colors[3] = vec3[3](\n"
		"vec3(1.0, 0.0, 0.0),\n"
		"vec3(0.0, 1.0, 0.0),\n"
		"vec3(0.0, 0.0, 1.0)\n"
	");\n"
	"void main() {\n"
	" gl_Position = vec4( positions[gl_VertexID], 0.0, 1.0 );\n"
	" vertex_color = vec4( colors[gl_VertexID], 1.0 );\n"
	"}\n";

static const char* frag_shader_text = 
	"#version 300 es\n"
	"precision mediump float;\n"
	"in vec4 vertex_color;\n"
	"out vec4 fragColor;\n"
	"void main() {\n"
	"  fragColor = vertex_color;\n"
	"}\n";

static uint32_t startTime = 0;

static void wl_buffer_release( void* pData, struct wl_buffer* pWlBuffer );

static void updateFrame_callback( void* pData, struct wl_callback* pFrameCallback, uint32_t time );
static void recordGlCommands( struct eglContext* pEglContext, uint32_t time );

static void surface_configure_callback( void * pData, struct wl_callback* pCallback, uint32_t time );

static void InitGLState();

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

	recordGlCommands( &pClientObjState->mpEglContext, time );

	pClientObjState->mpFrameCallback = wl_surface_frame( pClientObjState->mpWlSurface );
	wl_callback_add_listener( pClientObjState->mpFrameCallback, &frame_listener, pClientObjState );

	SwapEGLBuffers( &pClientObjState->mpEglContext );
}

static void recordGlCommands( struct eglContext* pEglContext, uint32_t time )
{
#if 1
	if( startTime == 0 )
		startTime = time;

	srand( time - startTime );

	float red = ( ( rand() % ( 255 - 1 + 1 ) ) + 1 ) / 255.0;
	float green = ( ( rand() % ( 255 - 1 + 1 ) ) + 1 ) / 255.0;
	float blue = ( ( rand() % ( 255 - 1 + 1 ) ) + 1 ) / 255.0;
	float alpha = ( ( rand() % ( 255 - 0 + 1 ) ) + 0 ) / 255.0;	
#else
	float red = 0.0;
	float green = 0.0;
	float blue = 0.0;
	float alpha = 1.0;
#endif 
	glViewport(0, 0, pEglContext->mWindowWidth, pEglContext->mWindowHeight );

	glClearColor(red, green, blue, alpha);
	glClear(GL_COLOR_BUFFER_BIT);

	glDrawArrays( GL_TRIANGLES, 0, 3 );
}

static void surface_configure_callback( void* pData, struct wl_callback* pCallback, uint32_t time )
{
	struct ClientObjState* pClientObj = pData;
	
	wl_callback_destroy( pCallback );

	pClientObj->mbSurfaceConfigured = 1;

	if( pClientObj->mpFrameCallback == NULL )
		updateFrame_callback( pClientObj, NULL, time );
}

static void InitGLState()
{
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
static void xdg_toplevel_handle_configure_bounds(
    void* pData,
    struct xdg_toplevel* pXdgTopLevel,
    int32_t width,
    int32_t height
)
{
}

static void xdg_toplevel_handle_compositor_capabilities(
    void* pData,
    struct xdg_toplevel* pXdgTopLevel,
    struct wl_array* pCapabilities
)
{

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

	uint16_t surfaceWidth = 800;
	uint16_t surfaceHeight = 600;
	const char* surfaceTitle = "EGL Client";

	CreateEGLSurface( clientObjState.mpWlSurface, surfaceWidth, surfaceHeight, &clientObjState.mpEglContext );
	InitGLState();

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

	ShutdownEGLContext( &clientObjState.mpEglContext, clientObjState.mpXdgTopLevel, clientObjState.mpXdgSurface, clientObjState.mpWlSurface );
    wl_display_disconnect(pDisplay);
    printf("Client Disconnected from the Display\n");

    return 0;
}