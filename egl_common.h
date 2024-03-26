#ifndef EGL_COMMON_H
#define EGL_COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <wayland-client.h>
#include <wayland-egl.h>

#include <EGL/egl.h>
#include <EGL/eglplatform.h>

#include "xdg-shell-client-protocol.h"

#ifndef RENDERING_API
#define RENDERING_API EGL_OPENGL_ES3_BIT
#endif

struct eglContext
{
    EGLNativeDisplayType mNativeDisplay;
    EGLNativeWindowType mNativeWindow;
    uint16_t mWindowWidth, mWindowHeight;
    EGLDisplay mEglDisplay;
    EGLContext mEglContext;
    EGLConfig mEglConfig;
    EGLSurface mEglSurface;
};

void InitEGLContext( struct eglContext* pEglContext );
void CreateEGLSurface( 
    struct wl_surface* pWlSurface, 
    uint16_t windowWidth, uint16_t windowHeight,
    struct eglContext* pEglContext 
);

void ShutdownEGLContext( 
    struct eglContext* pEglContext, 
    struct xdg_toplevel* pXdgTopLevel, 
    struct xdg_surface* pXdgSurface, 
    struct wl_surface* pWlSurface 
);

void LogEGLConfig(
    EGLDisplay eglDisplay,
    EGLConfig eglConfig
);

void SwapEGLBuffers( struct eglContext* pEglContext );

void InitEGLContext( struct eglContext* pEglContext )
{
    EGLDisplay eglDisplay = eglGetDisplay( pEglContext->mNativeDisplay );

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
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_RENDERABLE_TYPE, RENDERING_API,
#ifdef ENABLE_MSAA
        EGL_SAMPLE_BUFFERS, 1,
        EGL_SAMPLES, MSAA_SAMPLES,
#endif
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

    pEglContext->mEglDisplay = eglDisplay;
    pEglContext->mEglContext = eglContext;
    pEglContext->mEglConfig = eglConfig;

    LogEGLConfig( eglDisplay, eglConfig );
}

void CreateEGLSurface( 
    struct wl_surface* pWlSurface, 
    uint16_t windowWidth, uint16_t windowHeight,
    struct eglContext* pEglContext 
)
{
    EGLint result;

    EGLNativeWindowType eglNativeWindow = wl_egl_window_create( pWlSurface, windowWidth, windowHeight );
    EGLSurface eglSurface = eglCreateWindowSurface( 
        pEglContext->mEglDisplay, 
        pEglContext->mEglConfig, 
        eglNativeWindow, 
        NULL 
    );

    if( eglSurface == EGL_NO_SURFACE )
    {
        printf("Failed to Create EGL Surface\n");
    }

    pEglContext->mNativeWindow = eglNativeWindow;
    pEglContext->mEglSurface = eglSurface;

    result = eglMakeCurrent( pEglContext->mEglDisplay, eglSurface, eglSurface, pEglContext->mEglContext );

    if( result != EGL_TRUE )
    {
        printf( "Failed to Make Current Context\n" );
    }
}

void ShutdownEGLContext( 
    struct eglContext* pEglContext, 
    struct xdg_toplevel* pXdgTopLevel, 
    struct xdg_surface* pXdgSurface, 
    struct wl_surface* pWlSurface 
)
{
    eglDestroySurface( pEglContext->mEglDisplay, pEglContext->mEglSurface );
    wl_egl_window_destroy( pEglContext->mNativeWindow );
    xdg_toplevel_destroy( pXdgTopLevel );
    xdg_surface_destroy( pXdgSurface );
    wl_surface_destroy( pWlSurface );
    eglDestroyContext( pEglContext->mEglDisplay, pEglContext->mEglContext );
}

void SwapEGLBuffers( struct eglContext* pEglContext )
{
    eglSwapBuffers( pEglContext->mEglDisplay, pEglContext->mEglSurface );
}

const char* GetEGLReturnString(
    EGLint attrib
)
{
    switch (attrib)
    {
    case EGL_TRUE:
    return "EGL_TRUE";
    case EGL_FALSE:
    return "EGL_FALSE";
    case EGL_BAD_DISPLAY:
    return "EGL_BAD_DISPLAY";
    case EGL_NOT_INITIALIZED:
    return "EGL_NOT_INITIALIZED";
    case EGL_BAD_CONFIG:
    return "EGL_BAD_CONFIG";
    case EGL_BAD_ATTRIBUTE:
    return "EGL_BAD_ATTRIBUTE";
    default:
        break;
    }
}

const char* GetEGLSurfaceTypeString(
    EGLint attrib
)
{
    switch (attrib)
    {
    case EGL_MULTISAMPLE_RESOLVE_BOX_BIT:
    return "EGL_MULTISAMPLE_RESOLVE_BOX_BIT";
    case EGL_PBUFFER_BIT:
    return "EGL_PBUFFER_BIT";
    case EGL_PIXMAP_BIT:
    return "EGL_PIXMAP_BIT";
    case EGL_SWAP_BEHAVIOR_PRESERVED_BIT:
    return "EGL_SWAP_BEHAVIOR_PRESERVED_BIT";
    case EGL_VG_ALPHA_FORMAT_PRE_BIT:
    return "EGL_VG_ALPHA_FORMAT_PRE_BIT";
    case EGL_VG_COLORSPACE_LINEAR_BIT:
    return "EGL_VG_COLORSPACE_LINEAR_BIT";
    case  EGL_WINDOW_BIT:
    return "EGL_WINDOW_BIT";
    default:
        break;
    }
}
const char* GetEGLAttribString(
    EGLint attrib
)
{
    switch (attrib)
    {
    
    case EGL_ALPHA_SIZE:
    return "EGL_ALPHA_SIZE";
    case EGL_ALPHA_MASK_SIZE:
    return "EGL_ALPHA_MASK_SIZE";
    case EGL_BIND_TO_TEXTURE_RGB:
    return "EGL_BIND_TO_TEXTURE_RGB";
    case EGL_BIND_TO_TEXTURE_RGBA:
    return "EGL_BIND_TO_TEXTURE_RGBA";
    case EGL_BLUE_SIZE:
    return "EGL_BLUE_SIZE";
    case EGL_BUFFER_SIZE:
    return "EGL_BUFFER_SIZE";
    case EGL_COLOR_BUFFER_TYPE:
    return "EGL_COLOR_BUFFER_TYPE";
    case EGL_CONFIG_CAVEAT:
    return "EGL_CONFIG_CAVEAT";
    case EGL_CONFIG_ID:
    return "EGL_CONFIG_ID";
    case EGL_CONFORMANT:
    return "EGL_CONFORMANT";
    case EGL_DEPTH_SIZE:
    return "EGL_DEPTH_SIZE";
    case EGL_GREEN_SIZE:
    return "EGL_GREEN_SIZE";
    case EGL_LEVEL:
    return "EGL_LEVEL";
    case EGL_LUMINANCE_SIZE:
    return "EGL_LUMINANCE_SIZE";
    case EGL_MAX_PBUFFER_WIDTH:
    return "EGL_MAX_PBUFFER_WIDTH";
    case EGL_MAX_PBUFFER_HEIGHT:
    return "EGL_MAX_PBUFFER_HEIGHT";
    case EGL_MAX_PBUFFER_PIXELS:
    return "EGL_MAX_PBUFFER_PIXELS";
    case EGL_MAX_SWAP_INTERVAL:
    return "EGL_MAX_SWAP_INTERVAL";
    case EGL_MIN_SWAP_INTERVAL:
    return "EGL_MIN_SWAP_INTERVAL";
    case EGL_NATIVE_RENDERABLE:
    return "EGL_NATIVE_RENDERABLE";
    case EGL_NATIVE_VISUAL_ID:
    return "EGL_NATIVE_VISUAL_ID";
    case EGL_NATIVE_VISUAL_TYPE:
    return "EGL_NATIVE_VISUAL_TYPE";
    case EGL_RED_SIZE:
    return "EGL_RED_SIZE";
    case EGL_RENDERABLE_TYPE:
    return "EGL_RENDERABLE_TYPE";
    case EGL_SAMPLE_BUFFERS:
    return "EGL_SAMPLE_BUFFERS";
    case EGL_SAMPLES:
    return "EGL_SAMPLES";
    case EGL_STENCIL_SIZE:
    return "EGL_STENCIL_SIZE";
    case EGL_SURFACE_TYPE:
    return "EGL_SURFACE_TYPE";
    case EGL_TRANSPARENT_TYPE:
    return "EGL_TRANSPARENT_TYPE";
    case EGL_TRANSPARENT_RED_VALUE:
    return "EGL_TRANSPARENT_RED_VALUE";
    case EGL_TRANSPARENT_GREEN_VALUE:
    return "EGL_TRANSPARENT_GREEN_VALUE";
    case EGL_TRANSPARENT_BLUE_VALUE:
    return "EGL_TRANSPARENT_BLUE_VALUE";
    
    case EGL_RGB_BUFFER:
    return "EGL_RGB_BUFFER";
    case EGL_LUMINANCE_BUFFER:
    return "EGL_LUMINANCE_BUFFER";

    case EGL_SLOW_CONFIG:
    return "EGL_SLOW_CONFIG";
    case EGL_NON_CONFORMANT_CONFIG:
    return "EGL_NON_CONFORMANT_CONFIG";
    
    case EGL_TRANSPARENT_RGB:
    return "EGL_TRANSPARENT_RGB";

    case EGL_TRUE:
    return "EGL_TRUE";
    case EGL_FALSE:
    return "EGL_FALSE";
    case EGL_DONT_CARE:
    return "EGL_DONT_CARE";

    case EGL_NONE:
    return "EGL_NONE";
    default:
    return "Invalid";
    }
}

const char* GetEGLRenderableTypeString(
    EGLint attrib
)
{
    switch (attrib)
    {
    case EGL_OPENGL_BIT:
    return "EGL_OPENGL_BIT";
    case EGL_OPENVG_BIT:
    return "EGL_OPENVG_BIT";
    case EGL_OPENGL_ES_BIT:
    return "EGL_OPENGL_ES_BIT";
    case EGL_OPENGL_ES2_BIT:
    return "EGL_OPENGL_ES2_BIT";
    case EGL_OPENGL_ES3_BIT:
    return "EGL_OPENGL_ES3_BIT";
    default:
        break;
    }
}

void LogEGLConfig(
    EGLDisplay eglDisplay,
    EGLConfig eglConfig 
)
{
    EGLint attribQueries[] = {
        EGL_ALPHA_SIZE,
        EGL_ALPHA_MASK_SIZE,
        EGL_BIND_TO_TEXTURE_RGB,
        EGL_BIND_TO_TEXTURE_RGBA,
        EGL_BLUE_SIZE,
        EGL_BUFFER_SIZE,
        EGL_COLOR_BUFFER_TYPE,
        EGL_CONFIG_CAVEAT,
        EGL_CONFIG_ID,
        EGL_CONFORMANT,
        EGL_DEPTH_SIZE,
        EGL_GREEN_SIZE,
        EGL_LEVEL,
        EGL_LUMINANCE_SIZE,
        EGL_MAX_PBUFFER_WIDTH,
        EGL_MAX_PBUFFER_HEIGHT,
        EGL_MAX_PBUFFER_PIXELS,
        EGL_MAX_SWAP_INTERVAL,
        EGL_MIN_SWAP_INTERVAL,
        EGL_NATIVE_RENDERABLE,
        EGL_NATIVE_VISUAL_ID,
        EGL_NATIVE_VISUAL_TYPE,
        EGL_RED_SIZE,
        EGL_RENDERABLE_TYPE,
        EGL_SAMPLE_BUFFERS,
        EGL_SAMPLES,
        EGL_STENCIL_SIZE,
        EGL_SURFACE_TYPE,
        EGL_TRANSPARENT_TYPE,
        EGL_TRANSPARENT_RED_VALUE,
        EGL_TRANSPARENT_GREEN_VALUE,
        EGL_TRANSPARENT_BLUE_VALUE
    };

    printf("EGLConfig Attributes: \n");
    EGLint value;
    for( uint32_t i = 0; i < 32; i++ )
    {
        EGLBoolean reslt = eglGetConfigAttrib(
            eglDisplay,
            eglConfig,
            attribQueries[i],
            &value
        );

        if( reslt != EGL_TRUE )
        {
            printf("\teglGetConfigAttrib %s for %s\n", GetEGLAttribString(reslt), GetEGLReturnString(attribQueries[i]));
            continue;
        }

        if( attribQueries[i] == EGL_COLOR_BUFFER_TYPE ||
            attribQueries[i] == EGL_CONFIG_CAVEAT ||
            attribQueries[i] == EGL_NATIVE_RENDERABLE ||
            attribQueries[i] == EGL_TRANSPARENT_TYPE
        )
        {
            printf("\t%s = %s\n", GetEGLAttribString(attribQueries[i]), GetEGLAttribString(value));
        }
        else if(    attribQueries[i] == EGL_CONFORMANT ||
                    attribQueries[i] == EGL_RENDERABLE_TYPE ||
                    attribQueries[i] == EGL_SURFACE_TYPE
        )
        {
            if( attribQueries[i] == EGL_CONFORMANT || attribQueries[i] == EGL_RENDERABLE_TYPE )
            {
                if( value & EGL_OPENVG_BIT )
                    printf("\t%s = %s\n", GetEGLAttribString(attribQueries[i]), GetEGLRenderableTypeString(EGL_OPENVG_BIT));
                else if(value & EGL_OPENGL_ES2_BIT)
                    printf("\t%s = %s\n", GetEGLAttribString(attribQueries[i]), GetEGLRenderableTypeString(EGL_OPENGL_ES2_BIT));
                else if(value & EGL_OPENGL_ES3_BIT)
                    printf("\t%s = %s\n", GetEGLAttribString(attribQueries[i]), GetEGLRenderableTypeString(EGL_OPENGL_ES3_BIT));
                else if(value & EGL_OPENGL_ES_BIT)
                    printf("\t%s = %s\n", GetEGLAttribString(attribQueries[i]), GetEGLRenderableTypeString(EGL_OPENGL_ES_BIT));
                else if( value & EGL_OPENGL_BIT )
                    printf("\t%s = %s\n", GetEGLAttribString(attribQueries[i]), GetEGLRenderableTypeString(EGL_OPENGL_BIT));
            }
            
            if( attribQueries[i] == EGL_SURFACE_TYPE )
            {
                if( value & EGL_MULTISAMPLE_RESOLVE_BOX_BIT )
                    printf("\t%s = %s\n", GetEGLAttribString(attribQueries[i]), GetEGLSurfaceTypeString(EGL_MULTISAMPLE_RESOLVE_BOX_BIT));
                else if( value & EGL_PBUFFER_BIT )
                    printf("\t%s = %s\n", GetEGLAttribString(attribQueries[i]), GetEGLSurfaceTypeString(EGL_PBUFFER_BIT));
                else if(value & EGL_PIXMAP_BIT)
                    printf("\t%s = %s\n", GetEGLAttribString(attribQueries[i]), GetEGLSurfaceTypeString(EGL_PIXMAP_BIT));
                else if(value & EGL_SWAP_BEHAVIOR_PRESERVED_BIT)
                    printf("\t%s = %s\n", GetEGLAttribString(attribQueries[i]), GetEGLSurfaceTypeString(EGL_SWAP_BEHAVIOR_PRESERVED_BIT));
                else if(value & EGL_VG_ALPHA_FORMAT_PRE_BIT)
                    printf("\t%s = %s\n", GetEGLAttribString(attribQueries[i]), GetEGLSurfaceTypeString(EGL_VG_ALPHA_FORMAT_PRE_BIT));
                else if(value & EGL_VG_COLORSPACE_LINEAR_BIT)
                    printf("\t%s = %s\n", GetEGLAttribString(attribQueries[i]), GetEGLSurfaceTypeString(EGL_VG_COLORSPACE_LINEAR_BIT));
                else if(value & EGL_WINDOW_BIT)
                    printf("\t%s = %s\n", GetEGLAttribString(attribQueries[i]), GetEGLSurfaceTypeString(EGL_WINDOW_BIT));
            }           
        }
        else
        {
            printf("\t%s = %d\n", GetEGLAttribString(attribQueries[i]), value);
        }
    }
}

#endif 