
#include "egl_common.h"
#include "client_common.h"
#include "xdg_client_common.h"
#include "shm_helper.h"

#include <GLES2/gl2.h>
#include <stdlib.h>
#include <time.h>

#include <pthread.h>
#include <semaphore.h>

#define ENABLE_MULTI_THREADING 1

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
	struct wl_display* mpWlDisplay;
    struct GlobalObjectState* mpGlobalObjState;
    struct wl_surface* mpWlSurface;
    struct xdg_surface* mpXdgSurface;
    struct xdg_toplevel* mpXdgTopLevel;
	struct eglContext mpEglContext;
	struct wl_callback* mpFrameCallback;

	int8_t mbCloseApplication;
	int8_t mbSurfaceConfigured;

	uint8_t mbSignalDisplayTask;
	sem_t mDisplayTaskSemaphore;

	pthread_t mDispatcherThread;
	pthread_t mRenderingThread;

	struct wl_event_queue* mpDisplayDispatcherQueue;
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
	{
		printf("Invalidating Current Frame Callback\n");
		wl_callback_destroy(pFrameCallback);
	}

	if( !pClientObjState->mbSurfaceConfigured )
		return;

	printf("Recording GL Commands\n");

	recordGlCommands( &pClientObjState->mpEglContext, time );

	printf("Creating new Surface Frame Callback\n");
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

	printf("Surface Configured\n");

	if( pClientObj->mpFrameCallback == NULL )
	{
		printf("Setting Initial Frame Callback\n");
		updateFrame_callback( pClientObj, NULL, time );
	}
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

void* SurfaceUpdater( void* pArg )
{
# if 0
	struct ClientObjState* pClientObjState = (struct ClientObjState*) pArg;
	struct wl_display* pDisplay = pClientObjState->mpWlDisplay;

	printf("Surface Update Thread Identifier : %ld\n", pthread_self());

	pthread_detach( pthread_self() );

	struct wl_callback* configureCallback;

	configureCallback = wl_display_sync( pDisplay );
	wl_callback_add_listener( configureCallback, &configure_listener, pClientObjState );

	while( pClientObjState->mbCloseApplication != 1 )
	{
		//printf("Running Surface Updater\n");
	}

	printf("Terminating SurfaceUpdater Thread\n");
	pthread_exit(NULL);
#else
	struct ClientObjState* pClientObjState = (struct ClientObjState*) pArg;
	struct wl_display* pDisplay = pClientObjState->mpWlDisplay;

	struct wl_callback* configureCallback;
	configureCallback = wl_display_sync( pDisplay );
	wl_callback_add_listener( configureCallback, &configure_listener, pClientObjState );

	if( !pClientObjState->mpDisplayDispatcherQueue )
	{
		wl_proxy_set_queue( (struct wl_proxy*) configureCallback, pClientObjState->mpDisplayDispatcherQueue );
	}
	else 
	{
		printf("Display Event Queue Not Assigned");
	}

#endif 
}

void* DisplayDispatcher( void* pArg )
{
#if ENABLE_MULTI_THREADING
	struct ClientObjState* pClientObjState = (struct ClientObjState*) pArg;

	struct wl_display* pDisplay = pClientObjState->mpWlDisplay;

	pthread_detach( pthread_self() );

	sem_wait(&pClientObjState->mDisplayTaskSemaphore);

	pClientObjState->mpDisplayDispatcherQueue = wl_display_create_queue( pDisplay );

	printf("Display Dispatch Thread Identifier : %ld\n", pthread_self());

	pClientObjState->mbCloseApplication = 0;
	printf("Display Dispatched Running\n");

	int numOfEventsDispatched = 0;
	int numOfEventsRead = 0;
	int sentBytes = 0;
    while( pClientObjState->mbCloseApplication != 1 )
    {
		while( wl_display_prepare_read_queue( pDisplay, pClientObjState->mpDisplayDispatcherQueue ) != 0 )
		{
			printf("Display Events pending while preparing\n");
			printf("Dispatching pending Display Events\n");
			numOfEventsDispatched = wl_display_dispatch_queue_pending( pDisplay, pClientObjState->mpDisplayDispatcherQueue );
			printf("Display Events Dispatched Count : %d\n", numOfEventsDispatched);
		}

		sentBytes = wl_display_flush(pDisplay);

		printf("Sent Bytes to the Compositor : %d\n", sentBytes);
		numOfEventsRead = wl_display_read_events(pDisplay);
		printf("Num of Display Events Read From Queue: %d\n", numOfEventsRead);
		numOfEventsDispatched = wl_display_dispatch_queue_pending( pDisplay, pClientObjState->mpDisplayDispatcherQueue );
		printf("Display Events Dispatched Count : %d\n", numOfEventsDispatched);
    }

	printf("Terminating Display Dispatcher Thread\n");
	pthread_exit(NULL);
#else 
	struct ClientObjState* pClientObjState = (struct ClientObjState*) pArg;

	struct wl_display* pDisplay = pClientObjState->mpWlDisplay;

	pClientObjState->mbCloseApplication = 0;

	printf("Display Dispatched Running\n");

    while( pClientObjState->mbCloseApplication != 1 )
    {
		//printf("before dispatch loop\n");
		int reslt = wl_display_dispatch_pending(pDisplay);
		//printf("after dispatch loop : %d\n", reslt);
    }
#endif 
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
	clientObjState.mpWlDisplay = pDisplay;
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
	
#if ENABLE_MULTI_THREADING
	sem_init( &clientObjState.mDisplayTaskSemaphore, 0, 0 );

	pthread_create(
		&clientObjState.mDispatcherThread,
		NULL,
		&DisplayDispatcher,
		&clientObjState
	);

	if( clientObjState.mbSignalDisplayTask == 0 )
	{
		clientObjState.mbSignalDisplayTask = 1;
		printf("Signaling Dispatch Semaphore\n");
		sem_post(&clientObjState.mDisplayTaskSemaphore);
	}

#if 0
	pthread_create(
		&clientObjState.mRenderingThread,
		NULL,
		&SurfaceUpdater,
		&clientObjState
	);
#else 
	SurfaceUpdater( &clientObjState );
#endif

#if 	1
	printf("Thread Identifiers\n");
	//printf("Rendering Thread : %ld\n", clientObjState.mRenderingThread);
	printf("Dispatcher Thread : %ld\n", clientObjState.mDispatcherThread);


	while( clientObjState.mbCloseApplication != 1 )
	{
		//printf("Main Dispatch Loop\n");
		int dispatchedEvents = wl_display_dispatch( clientObjState.mpWlDisplay );
		printf("Main Dispatched Events : %d\n", dispatchedEvents);
		sleep(1);	
	}
#else
	DisplayDispatcher( &clientObjState ); 
#endif

#else 
	SurfaceUpdater( &clientObjState );
	DisplayDispatcher( &clientObjState );
#endif 	
	printf("Shutting Down Client\n");
	ShutdownEGLContext( &clientObjState.mpEglContext, clientObjState.mpXdgTopLevel, clientObjState.mpXdgSurface, clientObjState.mpWlSurface );
    wl_display_disconnect(pDisplay);
    printf("Client Disconnected from the Display\n");

    return 0;
}