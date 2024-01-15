
#include "egl_common.h"
#include "client_common.h"
#include "xdg_client_common.h"
#include "shm_helper.h"

#include <GLES2/gl2.h>

struct ClientObjState;

static void wl_buffer_release( void* pData, struct wl_buffer* pWlBuffer );
#if 0 
	static struct wl_buffer* draw_frame( struct ClientObjState* pClientObjState );
#else
	static void draw_frame();
#endif

struct ClientObjState
{
    struct GlobalObjectState* mpGlobalObjState;
    struct wl_surface* mpWlSurface;
    struct xdg_surface* mpXdgSurface;
    struct xdg_toplevel* mpXdgTopLevel;
	struct eglContext mpEglContext;

	int8_t mbCloseApplication;
};

static const struct wl_buffer_listener wl_buffer_listener = {
	.release = wl_buffer_release
};

static void wl_buffer_release( void* pData, struct wl_buffer* pWlBuffer )
{
	wl_buffer_destroy(pWlBuffer);
}


static void draw_frame()
{
	glClearColor(0.6, 0.25, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
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

	// resize
	wl_egl_window_resize( pClientObjState->mpEglContext.mNativeWindow, width, height, 0, 0 );
	wl_surface_commit( pClientObjState->mpWlSurface );
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

	uint16_t surfaceWidth = 800;
	uint16_t surfaceHeight = 600;
	const char* surfaceTitle = "EGL Client";

	CreateEGLSurface( clientObjState.mpWlSurface, surfaceWidth, surfaceHeight, &clientObjState.mpEglContext );

    clientObjState.mpXdgTopLevel = xdg_surface_get_toplevel( clientObjState.mpXdgSurface );
	AssignXDGToplevelListener(clientObjState.mpXdgTopLevel, &clientObjState);
    xdg_toplevel_set_title(clientObjState.mpXdgTopLevel, surfaceTitle);
    wl_surface_commit(clientObjState.mpWlSurface);

	clientObjState.mbCloseApplication = 0;

    while( clientObjState.mbCloseApplication != 1 )
    {
		wl_display_dispatch_pending(clientObjState.mpEglContext.mNativeDisplay);
		draw_frame();
		SwapEGLBuffers( &clientObjState.mpEglContext );
    }

	ShutdownEGLContext( &clientObjState.mpEglContext, clientObjState.mpXdgTopLevel, clientObjState.mpXdgSurface, clientObjState.mpWlSurface );
    wl_display_disconnect(pDisplay);
    printf("Client Disconnected from the Display\n");

    return 0;
}