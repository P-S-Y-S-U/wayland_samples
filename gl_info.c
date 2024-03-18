#include <wayland-client.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <EGL/eglplatform.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <stdio.h>
#include <string.h>


struct wl_display* wlDisplay;
struct wl_compositor* wlCompositor;

static void registry_handle_global(
    void* pData, struct wl_registry* pRegistry,
	uint32_t name, const char* pInterface, uint32_t version
)
{
    printf("Global Object Handle interface: %s, version: %d, name: %d\n",
		pInterface, version, name );

	if( strcmp("wl_compositor", pInterface) == 0 )
	{
		wlCompositor = wl_registry_bind(
			pRegistry, name,
			&wl_compositor_interface, version
		);
	}
}

static const struct wl_registry_listener g_registryListener = {
	.global = registry_handle_global
};

int main( int* argc, int* argv[] )
{
    wlDisplay = wl_display_connect(NULL);

    struct wl_registry* pRegistry = wl_display_get_registry(wlDisplay);
    wl_registry_add_listener(pRegistry, &g_registryListener, NULL);

    wl_display_dispatch(wlDisplay);

    EGLDisplay eglDisplay = eglGetDisplay( wlDisplay );

    EGLint result;

    /* initialize the EGL display connection */
    EGLint major;
    EGLint minor;
    result = eglInitialize(eglDisplay, &major, &minor);
        
    if( result != EGL_TRUE )
    {
        printf("Failed to Initialize EGL\n");
    }

    result = eglBindAPI(EGL_OPENGL_ES_API);

    if( result != EGL_TRUE )
    {
        printf("Failed to bind API\n");
    }

    EGLint const attribute_list[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 1,
        EGL_GREEN_SIZE, 1,
        EGL_BLUE_SIZE, 1,
        EGL_ALPHA_SIZE, 1,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_NONE
    };

    EGLint const context_attribs [] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    EGLint num_config;
    EGLConfig eglConfig;

    /* get an appropriate EGL frame buffer configuration */
    result = eglChooseConfig(eglDisplay, attribute_list, &eglConfig, 1, &num_config);
        
    if( result != EGL_TRUE )
    {
        printf("Failed to Choose EGL config\n");
    }

    /* create an EGL rendering context */
    EGLContext eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, context_attribs);

    if( eglContext == NULL )
    {
        printf("Failed to Create Context\n");
    }

    struct wl_surface* wlSurface = wl_compositor_create_surface( wlCompositor );

    EGLNativeWindowType eglNativeWindow = wl_egl_window_create( wlSurface, 640, 480 );
    EGLSurface eglSurface = eglCreateWindowSurface( eglDisplay, eglConfig, eglNativeWindow, NULL );
    eglMakeCurrent( eglDisplay, eglSurface, eglSurface, eglContext );

    char* extension = 
        (char*)glGetString(GL_EXTENSIONS);
    
    for( uint32_t i = 0; extension[i] != '\0'; i++ )
    {
        if( extension[i] == ' ' )
            extension[i] = '\n';
    }

    printf("GL Extensions : \n %s\n", extension); 
    
    eglDestroySurface( wlDisplay, eglSurface );
    wl_egl_window_destroy( eglNativeWindow );
    wl_surface_destroy( wlSurface );
    eglDestroyContext( eglDisplay, eglContext );
    wl_display_disconnect(wlDisplay);

    return 0;
}