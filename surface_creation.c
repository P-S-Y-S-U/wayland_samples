#include <stdio.h>
#include <string.h>
#include <wayland-client.h>

#include "global_registry_handle.h"

int main(int argc, const char* argv[])
{
	struct wl_display* pDisplay = wl_display_connect(NULL);
	if(!pDisplay)
	{
		printf("Failed to make display connection\n");
	}
	printf("Connection Established with Wayland Display\n");

	struct GlobalObjectState gObjState = {0};

	struct wl_registry* pRegistry = wl_display_get_registry(pDisplay);
	if(!pRegistry)
	{
		printf("Failed to obtain display registry\n");
	}

	printf("Registry for globals has been retrieved\n");
	wl_registry_add_listener(pRegistry, &g_registryListener, &gObjState);

	wl_display_roundtrip(pDisplay);

	if( !gObjState.mpCompositor )
	{
		printf("Failed to retrieve global objects\n");
		return 1;
	}

	struct wl_surface* pSurface = wl_compositor_create_surface(gObjState.mpCompositor);
	if( !pSurface )
	{
		printf("Failed to Create Wayland Surface\n");
		return 1;
	}

	printf("Starting Client Event Loop\n");

	while( wl_display_dispatch(pDisplay) != -1 )
	{

	}
	wl_display_disconnect(pDisplay);
	printf("Client Disconnected from the Display\n");
	return 0;
}
