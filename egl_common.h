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

struct eglContext
{
    EGLNativeDisplayType mNativeDisplay;
    EGLNativeWindowType mNativeWindow;
    uint16_t mWindowWidth, mWindowHeight;
    EGLDisplay mEglDisplay;
    EGLContext mEglContext;
    EGLSurface mEglSurface;
};

void CreateNativeWindow(
    struct eglContext* pEglContext,
    struct wl_surface* pSurface,
    const char* title, 
    uint16_t width, uint16_t height 
);
EGLBoolean CreateEGLContext( struct eglContext* pEglContext );

void ShutdownEGLContext( 
    struct eglContext* pEglContext, 
    struct xdg_toplevel* pXdgTopLevel, 
    struct xdg_surface* pXdgSurface, 
    struct wl_surface* pWlSurface 
);
void CreateWindowWithEGLContext(
    struct wl_surface* pSurface,
    const char* title, 
    uint16_t width, uint16_t height,
    struct eglContext* pEglContext
);
void SwapEGLBuffers( struct eglContext* pEglContext );


void CreateNativeWindow(
    struct eglContext* pEglContext,
    struct wl_surface* pSurface,
    const char* title, 
    uint16_t width, uint16_t height 
)
{
    struct wl_egl_window* pEglWindow = wl_egl_window_create( pSurface, width, height );
    
    if(pEglWindow == EGL_NO_SURFACE)
    {
        printf("Failed to Create EGL Window\n");
        exit(1);
    }
    printf("EGL Window Created\n");
    pEglContext->mWindowWidth = width;
    pEglContext->mWindowHeight = height;
    pEglContext->mNativeWindow = pEglWindow;
}

EGLBoolean CreateEGLContext(
    struct eglContext* pEglContext
)
{
    EGLDisplay eglDisplay = eglGetDisplay( pEglContext->mNativeDisplay );
    if( eglDisplay == EGL_NO_DISPLAY )
    {
        printf("Failed to retrieve EGL Display\n");
        return EGL_FALSE;
    }

    EGLint majorVersion, minorVersion;
    if( !eglInitialize( eglDisplay, &majorVersion, &minorVersion ) )
    {
        printf("Failed to Initialize EGL\n");
        return EGL_FALSE;
    }

    EGLint numOfConfigs;
    if( (eglGetConfigs( eglDisplay, NULL, 0, &numOfConfigs) != EGL_TRUE) || (numOfConfigs == 0) )
    {
        printf("Failed to Get Num of EGL Configs\n");
        return EGL_FALSE;
    }

    EGLint fboAtrributes[]  = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_NONE
    };
    EGLConfig config;

    if( ( eglChooseConfig( eglDisplay, fboAtrributes, &config, 1, &numOfConfigs ) != EGL_TRUE ) || (numOfConfigs != 1) )
    {
        printf("No EGL config chosen\n");
        return EGL_FALSE;
    }

    EGLSurface eglSurface = eglCreateWindowSurface( eglDisplay, config, pEglContext->mNativeWindow, NULL );
    if( eglSurface == EGL_NO_SURFACE )
    {
        printf("Failed to create EGL Surface\n");
        return EGL_FALSE;
    }

    EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE
    };
    EGLContext eglContext = eglCreateContext( eglDisplay, config, EGL_NO_CONTEXT, contextAttribs );
    if( eglContext == EGL_NO_CONTEXT )
    {
        printf("Failed to create EGL context\n");
        return EGL_FALSE;
    }

    if( !eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext) )
    {
        printf("Couldn't Make Current Context to the Window\n");
        return EGL_FALSE;
    }

    pEglContext->mEglDisplay = eglDisplay;
    pEglContext->mEglSurface = eglSurface;
    pEglContext->mEglContext = eglContext;
    return EGL_TRUE;
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

void CreateWindowWithEGLContext(
    struct wl_surface* pSurface,
    const char* title, 
    uint16_t width, uint16_t height,
    struct eglContext* pEglContext
)
{
    CreateNativeWindow( pEglContext, pSurface, title, width, height );
    if( CreateEGLContext( pEglContext ) != EGL_TRUE )
    {
        printf("Failed to start a EGL context for the surface\n");
    }
}

void SwapEGLBuffers( struct eglContext* pEglContext )
{
    eglSwapBuffers( pEglContext->mEglDisplay, pEglContext->mEglSurface );
}

#endif 