#ifndef XDG_SHELL_COMMON_H
#define XDG_SHELL_COMMON_H

#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"

static void xdg_surface_configure(
	void* pData, struct xdg_surface* pXdgSurface, uint32_t serial
);

static const struct xdg_surface_listener xdg_surface_listener = {
	.configure = xdg_surface_configure
};

void AssignXDGSurfaceListener( struct xdg_surface* pXdgSurface, void* pClientUsrData )
{
    xdg_surface_add_listener(
        pXdgSurface,
        &xdg_surface_listener,
        pClientUsrData
    );
}

#endif 