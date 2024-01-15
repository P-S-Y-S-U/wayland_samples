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
        EGL_RED_SIZE, 1,
        EGL_GREEN_SIZE, 1,
        EGL_BLUE_SIZE, 1,
        EGL_ALPHA_SIZE, 1,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
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
    EGLContext eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, NULL);

    if( eglContext == NULL )
    {
        printf("Failed to Create Context\n");
    }

    pEglContext->mEglDisplay = eglDisplay;
    pEglContext->mEglContext = eglContext;
    pEglContext->mEglConfig = eglConfig;
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
    pEglContext->mWindowWidth = windowWidth;
    pEglContext->mWindowHeight = windowHeight;

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

#endif 