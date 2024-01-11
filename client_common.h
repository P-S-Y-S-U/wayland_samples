#ifndef CLIENT_COMMMON_H
#define CLIENT_COMMMON_H

#include "global_registry_handle.h"

#include <wayland-client.h>

void retrieve_session_global( struct wl_display* pDisplay, struct GlobalObjectState* pGObjState )
{
    struct wl_registry* pRegistry = wl_display_get_registry(pDisplay);

    if(!pRegistry)
    {
        printf("Failed to Obtain Display Registry\n");
    }
    wl_registry_add_listener(pRegistry, &g_registryListener, pGObjState);

    wl_display_roundtrip(pDisplay);
}

#endif 