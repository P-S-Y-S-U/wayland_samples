#include <stdlib.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "xdg-shell-client-protocol.h"
#include "global_registry_handle.h"
#include "client_common.h"
#include "xdg_client_common.h"


typedef EGLNativeWindowType NativeWindowType;
//extern NativeWindowType createNativeWindow(void);
static EGLint const attribute_list[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 1,
        EGL_GREEN_SIZE, 1,
        EGL_BLUE_SIZE, 1,
        EGL_ALPHA_SIZE, 1,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
};

static EGLint const context_attrib[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
};

EGLDisplay display;
EGLConfig config;
EGLContext context;
EGLSurface surface;
NativeWindowType native_window;
EGLint num_config;

struct wl_display* wlDisplay;
struct wl_surface* wlSurface;
struct xdg_surface* xdgSurface;
struct xdg_toplevel* xdgToplevel;

static void xdg_surface_configure(
	void* pData, struct xdg_surface* pXdgSurface, uint32_t serial
)
{
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

	// resize
	wl_egl_window_resize( native_window, width, height, 0, 0 );
	wl_surface_commit( wlSurface );
}

static void xdg_toplevel_handle_close(
    void* pData,
    struct xdg_toplevel* pXdgTopLevel
)
{

}

int main(int argc, char ** argv)
{       
        wlDisplay = wl_display_connect(NULL);

        struct GlobalObjectState gObjState = {0};
        retrieve_session_global( wlDisplay, &gObjState );

        /* get an EGL display connection */
        display = eglGetDisplay(wlDisplay);

        EGLint result;

        /* initialize the EGL display connection */
        EGLint major;
        EGLint minor;
        result = eglInitialize(display, &major, &minor);
        
        if( result != EGL_TRUE )
        {
                printf("Failed to Initialize EGL\n");
        }

        result = eglBindAPI(EGL_OPENGL_API);

        if( result != EGL_TRUE )
        {
                printf("Failed to bind API\n");
        }

        /* get an appropriate EGL frame buffer configuration */
        result = eglChooseConfig(display, attribute_list, &config, 1, &num_config);
        
        if( result != EGL_TRUE )
        {
                printf("Failed to Choose EGL config\n");
        }

        /* create an EGL rendering context */
        context = eglCreateContext(display, config, EGL_NO_CONTEXT, NULL);

        if( context == NULL )
        {
                printf("Failed to Create Context");
        }

        wlSurface = wl_compositor_create_surface(gObjState.mpCompositor);
        xdgSurface = xdg_wm_base_get_xdg_surface( gObjState.mpXdgWmBase, wlSurface );
        AssignXDGSurfaceListener(xdgSurface, NULL);

        native_window = wl_egl_window_create( wlSurface, 1280, 720 );
        /* create an EGL window surface */
        surface = eglCreateWindowSurface(display, config, native_window, NULL);

        /* connect the context to the surface */
        result = eglMakeCurrent(display, surface, surface, context);

        if( result != EGL_TRUE )
        {
                printf("Failed to Make Current Context\n");
        }

        xdgToplevel = xdg_surface_get_toplevel( xdgSurface );
        xdg_toplevel_set_title(xdgToplevel, "Egl sample");
        AssignXDGToplevelListener( xdgToplevel, NULL );
        wl_surface_commit(wlSurface);
        
        wl_display_dispatch_pending(wlDisplay);

        /* clear the color buffer */
        glClearColor(1.0, 1.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glFlush();
        eglSwapBuffers(display, surface);

        sleep(10);

        eglDestroySurface( wlDisplay, wlSurface );
        wl_egl_window_destroy( native_window );
        xdg_toplevel_destroy( xdgToplevel );
        xdg_surface_destroy( xdgSurface );
        wl_surface_destroy( wlSurface );
        eglDestroyContext( wlDisplay, context );

        wl_display_disconnect(wlDisplay);

        return EXIT_SUCCESS;
}