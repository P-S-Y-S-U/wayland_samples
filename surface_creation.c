#include <stdio.h>
#include <string.h>
#include <wayland-client.h>

struct clientState
{
	struct wl_compositor* mpCompositor;
};


static void registry_handle_global(
	void* pData, struct wl_registry* pRegistry,
	uint32_t name, const char* pInterface, uint32_t version
)
{
	printf("Global Object Handle interface: %s, version: %d, name: %d\n",
		pInterface, version, name );

	struct	clientState* pClientState = pData;

	if( strcmp("wl_compositor", pInterface) == 0 )
	{
		pClientState->mpCompositor = wl_registry_bind(pRegistry, name,
			&wl_compositor_interface, version
		);
	}
	else if( strcmp("wl_output", pInterface) == 0 )
	{
	}
}

static void registry_handle_global_remove(
	void* pData, struct wl_registry* pRegistry,
	uint32_t name
)
{
	printf("name: %d has been removed from registry\n", name);
}

static const struct wl_registry_listener registry_listener = {
	.global = registry_handle_global,
	.global_remove = registry_handle_global_remove
};

int main(int argc, const char* argv[])
{
	struct wl_display* pDisplay = wl_display_connect(NULL);
	if(!pDisplay)
	{
		printf("Failed to make display connection\n");
	}
	printf("Connection Established with Wayland Display\n");

	struct clientState state = { 0 };

	struct wl_registry* pRegistry = wl_display_get_registry(pDisplay);
	if(!pRegistry)
	{
		printf("Failed to obtain display registry\n");
	}

	printf("Registry for globals has been retrieved\n");
	wl_registry_add_listener(pRegistry, &registry_listener, &state);

	wl_display_roundtrip(pDisplay);

	if( !state.mpCompositor )
	{
		printf("Failed to retrieve global objects\n");
		return 1;
	}

	struct wl_surface* pSurface = wl_compositor_create_surface(state.mpCompositor);
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
