
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

static void thread_request_processed_callback( void* pData, struct wl_callback* pCallback, uint32_t time );

static void InitGLState();

static GLuint createShader( const char* source, GLenum shaderType );

void createWaylandEGLSurface( struct ClientObjState* pClientObjState );

struct ClientObjState
{
	struct wl_display* mpWlDisplay;
    struct GlobalObjectState* mpGlobalObjState;
    struct wl_surface* mpWlSurface;
    struct xdg_surface* mpXdgSurface;
    struct xdg_toplevel* mpXdgTopLevel;
	struct eglContext mpEglContext;
	struct wl_callback* mpFrameCallback;
	struct wl_callback* mpThreadRequestProcessed;

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

static const struct wl_callback_listener thread_request_listener = {
	thread_request_processed_callback
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
		//printf("Invalidating Current Frame Callback\n");
		wl_callback_destroy(pFrameCallback);
	}

	if( !pClientObjState->mbSurfaceConfigured )
		return;

	//printf("Recording GL Commands\n");

	recordGlCommands( &pClientObjState->mpEglContext, time );

	//printf("Creating new Surface Frame Callback\n");
	pClientObjState->mpFrameCallback = wl_surface_frame( pClientObjState->mpWlSurface );
	wl_callback_add_listener( pClientObjState->mpFrameCallback, &frame_listener, pClientObjState );
	wl_proxy_set_queue( (struct wl_proxy*) pClientObjState->mpFrameCallback, pClientObjState->mpDisplayDispatcherQueue );

	SwapEGLBuffers( &pClientObjState->mpEglContext );
}

static void thread_request_processed_callback( void* pData, struct wl_callback* pThreadRequestCallback, uint32_t time )
{
	struct ClientObjState* pClientObjState = pData;

	if( pThreadRequestCallback != pClientObjState->mpThreadRequestProcessed )
	{
		printf("Thread Request Callback Not Synced\n");
		return;
	}
	
	pClientObjState->mpThreadRequestProcessed = NULL;

	printf("Frame Process Request done\n");

	if( pThreadRequestCallback )
	{
		wl_callback_destroy( pThreadRequestCallback );
	}
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

void createWaylandEGLSurface( struct ClientObjState* pClientObjState )
{
	InitEGLContext( &pClientObjState->mpEglContext );
	
    pClientObjState->mpWlSurface = wl_compositor_create_surface(pClientObjState->mpGlobalObjState->mpCompositor);
	if( !pClientObjState->mpWlSurface )
	{
		printf("Failed to Create Wayland Surface\n");
		return;
	}

	pClientObjState->mpXdgSurface = xdg_wm_base_get_xdg_surface(
		pClientObjState->mpGlobalObjState->mpXdgWmBase, pClientObjState->mpWlSurface
	);
	if( !pClientObjState->mpXdgSurface )
	{
		printf("Failed to create Xdg Surface\n");
		return;
	}

    AssignXDGSurfaceListener( pClientObjState->mpXdgSurface, pClientObjState );

	uint16_t surfaceWidth = 800;
	uint16_t surfaceHeight = 600;
	const char* surfaceTitle = "EGL Client";

	CreateEGLSurface( pClientObjState->mpWlSurface, surfaceWidth, surfaceHeight, &pClientObjState->mpEglContext );
	InitGLState();

    pClientObjState->mpXdgTopLevel = xdg_surface_get_toplevel( pClientObjState->mpXdgSurface );
	AssignXDGToplevelListener(pClientObjState->mpXdgTopLevel, pClientObjState);
    xdg_toplevel_set_title(pClientObjState->mpXdgTopLevel, surfaceTitle);

	wl_proxy_set_queue( (struct wl_proxy*) pClientObjState->mpWlSurface, pClientObjState->mpDisplayDispatcherQueue );
	wl_proxy_set_queue( (struct wl_proxy*) pClientObjState->mpXdgSurface, pClientObjState->mpDisplayDispatcherQueue );
	wl_proxy_set_queue( (struct wl_proxy*) pClientObjState->mpXdgTopLevel, pClientObjState->mpDisplayDispatcherQueue );
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

void* DisplayDispatcher( void* pArg )
{
#if ENABLE_MULTI_THREADING
	struct ClientObjState* pClientObjState = (struct ClientObjState*) pArg;
	pClientObjState->mpDisplayDispatcherQueue = wl_display_create_queue( pClientObjState->mpWlDisplay );

	struct wl_display* pDisplay = pClientObjState->mpWlDisplay;

	pthread_detach( pthread_self() );

	sem_wait(&pClientObjState->mDisplayTaskSemaphore);

	createWaylandEGLSurface( pClientObjState );
	
	struct wl_callback* configureCallback;
	configureCallback = wl_display_sync( pDisplay );
	wl_callback_add_listener( configureCallback, &configure_listener, pClientObjState );
	wl_proxy_set_queue( (struct wl_proxy*) configureCallback, pClientObjState->mpDisplayDispatcherQueue );

	printf("Display Dispatch Thread Identifier : %ld\n", pthread_self());

	pClientObjState->mbCloseApplication = 0;

	int numOfEventsDispatched = 0;
	int numOfEventsRead = 0;
	int sentBytes = 0;
	int numOfRequestsProcessed = 0;

	printf("Performing RoundTrip on Thread Queue\n");
	numOfRequestsProcessed = wl_display_roundtrip_queue( pClientObjState->mpWlDisplay, pClientObjState->mpDisplayDispatcherQueue );
	printf("Num of Requests Processed after Roundtrip : %d\n", numOfRequestsProcessed);
	printf("Display Dispatched Running\n");
    while( pClientObjState->mbCloseApplication != 1 )
    {
		numOfEventsDispatched = wl_display_dispatch_queue( pDisplay, pClientObjState->mpDisplayDispatcherQueue );

		if( numOfEventsDispatched > 0 )
			printf("Custom Queue Dispatched Events :%d\n", numOfEventsDispatched);

#if 0
		pClientObjState->mpThreadRequestProcessed = wl_display_sync( pClientObjState->mpWlDisplay );
		wl_callback_add_listener( 
			pClientObjState->mpThreadRequestProcessed, 
			&thread_request_listener, 
			pClientObjState 
		);
		wl_proxy_set_queue( (struct wl_proxy*) pClientObjState->mpThreadRequestProcessed, pClientObjState->mpDisplayDispatcherQueue );
#endif
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

	printf("Thread Identifiers\n");
	printf("Dispatcher Thread : %ld\n", clientObjState.mDispatcherThread);


	while( clientObjState.mbCloseApplication != 1 )
	{
		//printf("Main Dispatch Loop\n");
		int dispatchedEvents = wl_display_dispatch( clientObjState.mpWlDisplay );
		if( dispatchedEvents > 0 )
		{
			printf("Main Dispatched Events : %d\n", dispatchedEvents);
		}
		//printf("Performed Dispatcher Main Queue\n");
		//sleep(1);	
	}
#else 
	DisplayDispatcher( &clientObjState );
#endif 	
	printf("Shutting Down Client\n");
	ShutdownEGLContext( &clientObjState.mpEglContext, clientObjState.mpXdgTopLevel, clientObjState.mpXdgSurface, clientObjState.mpWlSurface );

	wl_event_queue_destroy( clientObjState.mpDisplayDispatcherQueue);
    wl_display_disconnect(pDisplay);
    printf("Client Disconnected from the Display\n");

    return 0;
}