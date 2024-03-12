#include <wayland-client.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <EGL/eglplatform.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "xdg-shell-client-protocol.h"
#include "xdg_global_handle.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

struct wl_display* wlDisplay;
struct wl_compositor* wlCompositor;
struct xdg_wm_base* xdgWmBase;
struct wl_surface* wlSurface;
struct wl_callback* wlFrameCallback;
EGLDisplay eglDisplay;
EGLNativeWindowType eglNativeWindow;
EGLContext eglContext;
EGLSurface eglSurface;

uint32_t surface_width = 640;
uint32_t surface_height = 480;

uint32_t initialFrameCallback = 0;
uint32_t SurfacePresented = 0;
uint32_t exitPG = 0;
uint32_t dumpTimes = 5;

uint32_t currentDownload = 0;
uint32_t numOfPBOs = 1;
uint32_t numOfDownload = 0;
uint32_t downloadLimit = 2;

PFNGLMAPBUFFERRANGEEXTPROC gl_map_buffer_range_EXT = NULL;
PFNGLUNMAPBUFFEROESPROC gl_unmap_buffer_oes = NULL;
PFNGLGENBUFFERSPROC gl_gen_buffers = NULL;

GLuint* pboBuffers;
void* pixelDump;

static void updateFrame_callback( void* pData, struct wl_callback* pFrameCallback, uint32_t time );
static void CreatePBO();
static void CaptureFrameUsingPBO();
static void DeletePBO();

static const struct wl_callback_listener g_frameListener = {
    .done = updateFrame_callback
};

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
    else if( strcmp(xdg_wm_base_interface.name, pInterface) == 0 )
	{
		xdgWmBase = wl_registry_bind(
			pRegistry, name,
			&xdg_wm_base_interface, version
		);
		xdg_wm_base_add_listener(
			xdgWmBase,
			&xdg_wm_base_listener, NULL
		);
	}

}

static const struct wl_registry_listener g_registryListener = {
	.global = registry_handle_global
};

static void updateFrame_callback( void* pData, struct wl_callback* pFrameCallback, uint32_t time )
{
    if( pFrameCallback != wlFrameCallback )
        return;
    wlFrameCallback = NULL;

    if( wlFrameCallback )
        wl_callback_destroy( wlFrameCallback );

    if( !initialFrameCallback )
        initialFrameCallback = 1;
    else
    {
        SurfacePresented = 1;
        CaptureFrameUsingPBO();
    }
    
    {
        glViewport(0, 0, surface_width, surface_height);
        glClearColor(1.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    wlFrameCallback = wl_surface_frame( wlSurface );
    wl_callback_add_listener( wlFrameCallback, &g_frameListener, NULL );

    eglSwapBuffers( eglDisplay, eglSurface );
}

static void xdg_surface_configure(
	void* pData, struct xdg_surface* pXdgSurface, uint32_t serial
)
{
	xdg_surface_ack_configure(pXdgSurface, serial);

    if( !initialFrameCallback )
        updateFrame_callback(
            NULL, NULL, 0
        );
}

static const struct xdg_surface_listener g_xdgSurfaceListener = {
    .configure = xdg_surface_configure
};

static void xdg_toplevel_handle_configure(
	void* pData,
	struct xdg_toplevel* pXdgToplevel,
	int32_t width, int32_t height,
	struct wl_array* pStates
)
{
	// resizecontext_attribs
	wl_egl_window_resize( eglNativeWindow, width, height, 0, 0 );
}

static const struct xdg_toplevel_listener g_xdgTopLevelListener = {
    .configure = xdg_toplevel_handle_configure
};

static void CreatePBO()
{
    if( !gl_map_buffer_range_EXT ||
        !gl_unmap_buffer_oes ||
        !gl_gen_buffers 
    )
    {
        gl_map_buffer_range_EXT = (void*) eglGetProcAddress("glMapBufferRangeEXT");
        gl_unmap_buffer_oes = (void*) eglGetProcAddress( "glUnmapBufferOES" );
        gl_gen_buffers = (void*) eglGetProcAddress( "glGenBuffers" );
    }

    size_t bufferSizeInBytes = surface_width * surface_height * 4;
    pboBuffers = malloc( numOfPBOs * sizeof(GLuint) );
    
    gl_gen_buffers( numOfPBOs, pboBuffers );

    for( uint16_t i = 0; i < numOfPBOs; i++ )
    {
        glBindBuffer( GL_PIXEL_PACK_BUFFER_NV, pboBuffers[i] );
        glBufferData(
            GL_PIXEL_PACK_BUFFER_NV,
            bufferSizeInBytes,
            NULL,
            GL_STREAM_DRAW
        );
    }

    glBindBuffer( GL_PIXEL_PACK_BUFFER_NV, 0 );
}

static void CaptureFrameUsingPBO()
{
    size_t bufferSizeInBytes = surface_width * surface_height * 4;
    uint32_t bytespp = 4;
    GLenum imgFormat = GL_RGBA;

    if( numOfDownload < numOfPBOs )
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER_NV, pboBuffers[currentDownload]);
        glPixelStorei(GL_PACK_ALIGNMENT, bytespp );
        glReadPixels(
            0, 0,
            surface_width, surface_height,
            imgFormat,
            GL_UNSIGNED_BYTE,
            0
        );
    }
    else 
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER_NV, pboBuffers[currentDownload]);

        uint8_t* mappedBuffer = gl_map_buffer_range_EXT(
            GL_PIXEL_PACK_BUFFER_NV,
            0,
            bufferSizeInBytes, 
            GL_MAP_READ_BIT_EXT
        );

        if( mappedBuffer != NULL )
        {
            memcpy(pixelDump, mappedBuffer, bufferSizeInBytes);
            gl_unmap_buffer_oes(GL_PIXEL_PACK_BUFFER_NV);
        }
        else
        {
            printf("Couldn't Map Buffer");
        }

        glPixelStorei(GL_PACK_ALIGNMENT, bytespp);
        glReadPixels(
          0, 0, 
          surface_width, 
          surface_height, 
          imgFormat,
          GL_UNSIGNED_BYTE, 
          0
        );
    }
    currentDownload++;
    currentDownload = currentDownload % numOfPBOs;
    numOfDownload++;

    if ( numOfDownload > downloadLimit )
    {
        numOfDownload = numOfPBOs;
        exitPG = 1;
    }
    
    //sleep(1);
}

static void DeletePBO()
{
    glBindBuffer( GL_PIXEL_PACK_BUFFER_NV, 0 );
    glDeleteBuffers( numOfPBOs, pboBuffers );
}

int main( int* argc, int* argv[] )
{
    wlDisplay = wl_display_connect(NULL);

    struct wl_registry* pRegistry = wl_display_get_registry(wlDisplay);
    wl_registry_add_listener(pRegistry, &g_registryListener, NULL);

    wl_display_dispatch(wlDisplay);

    eglDisplay = eglGetDisplay( wlDisplay );

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
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
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
    eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, context_attribs);

    if( eglContext == NULL )
    {
        printf("Failed to Create Context\n");
    }

    wlSurface = wl_compositor_create_surface( wlCompositor );

    eglNativeWindow = wl_egl_window_create( wlSurface, surface_width, surface_height );
    eglSurface = eglCreateWindowSurface( eglDisplay, eglConfig, eglNativeWindow, NULL );
    eglMakeCurrent( eglDisplay, eglSurface, eglSurface, eglContext );

    struct xdg_surface* xdgSurface = xdg_wm_base_get_xdg_surface( xdgWmBase, wlSurface );
    xdg_surface_add_listener(
        xdgSurface,
        &g_xdgSurfaceListener,
        NULL
    );

    struct xdg_toplevel* xdgTopLevel = xdg_surface_get_toplevel( xdgSurface );
    xdg_toplevel_add_listener(
        xdgTopLevel,
        &g_xdgTopLevelListener,
        NULL
    );

    pixelDump = malloc( surface_width * surface_height * 4 );
    CreatePBO();

    wl_surface_commit( wlSurface );

    while( !exitPG )
        wl_display_dispatch(wlDisplay);
    
    DeletePBO();
    if( !pixelDump )
        printf("Pixel Not Dumped\n");
    else
    {
        int img_ret = stbi_write_jpg("POB-dump.jpg", surface_width, surface_height, 4, pixelDump, 100);
        if( img_ret == 0 )
            printf("Failed to write image\n");
    }

    eglDestroySurface( wlDisplay, eglSurface );
    wl_egl_window_destroy( eglNativeWindow );
    wl_surface_destroy( wlSurface );
    eglDestroyContext( eglDisplay, eglContext );
    wl_display_disconnect(wlDisplay);

    return 0;
}