#ifndef XDG_SHELL_COMMON_H
#define XDG_SHELL_COMMON_H

#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"

// xdg_surface listeners & CBs
static void xdg_surface_configure(
	void* pData, struct xdg_surface* pXdgSurface, uint32_t serial
);

static const struct xdg_surface_listener xdg_surface_listener = {
	.configure = xdg_surface_configure
};

// xdg_toplevel listeners & CBs
static void xdg_toplevel_handle_configure(
    void* pData, struct xdg_toplevel* pXdgTopLevel, 
    int32_t w, int32_t h,
    struct wl_array* states
);
static void xdg_toplevel_handle_close(
    void* pData,
    struct xdg_toplevel* pXdgTopLevel
);

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_handle_configure,
    .close = xdg_toplevel_handle_close
};

void AssignXDGSurfaceListener( struct xdg_surface* pXdgSurface, void* pClientUsrData )
{
    xdg_surface_add_listener(
        pXdgSurface,
        &xdg_surface_listener,
        pClientUsrData
    );
}

void AssignXDGToplevelListener( struct xdg_toplevel* pXdgToplevel, void* pClientUsrData )
{
    xdg_toplevel_add_listener(
        pXdgToplevel,
        &xdg_toplevel_listener,
        pClientUsrData
    );
}

#endif 