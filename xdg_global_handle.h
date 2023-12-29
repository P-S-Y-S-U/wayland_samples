#ifndef _XDG_GLOBAL_HANDLE_H
#define _XDG_GLOBAL_HANDLE_H

#include "xdg-shell-client-protocol.h"

static void xdg_wm_base_ping(
	void* pData, struct xdg_wm_base* xdg_wm_base, uint32_t serial
);

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
	.ping = xdg_wm_base_ping
};

static void xdg_wm_base_ping(
	void* pData, struct xdg_wm_base* xdg_wm_base, uint32_t serial
)
{
	printf("Receive ping event from server with serial: %d\n", serial);
	xdg_wm_base_pong( xdg_wm_base, serial );
}

#endif
