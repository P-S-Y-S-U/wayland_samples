#include <stdio.h>
#include <string.h>
#include <wayland-client.h>

struct wl_output* gpOutput = NULL;

static void registry_handle_global(
	void* pData, struct wl_registry* pRegistry,
	uint32_t name, const char* pInterface, uint32_t version
)
{
	printf("Global Object Handle interface: %s, version: %d, name: %d\n",
		pInterface, version, name );
	if( strcmp("wl_output", pInterface) == 0 )
	{
		printf("Retrieving wl_output global object\n");
		gpOutput = wl_registry_bind( pRegistry, name, &wl_output_interface, version );
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

static void wl_output_geometry_event(
	void* pData, struct wl_output* wl_output,
	int32_t x, int32_t y, int32_t phyWidth, int32_t phyHeight,
	int32_t subpixel,
	const char* pMake, const char* pModel,
	int32_t transform
)
{
	printf("wl_output_geomtery_event: \n");
	printf("x: %d, y: %d,\n", x, y);
	printf("phyWidth: %d, phyHeight: %d, \n", phyWidth, phyHeight);
	printf("subpixel: %d, \n", subpixel);
	printf("make: %s, model: %s, \n", pMake, pModel);
	printf("transform: %d \n", transform);
}

static const struct wl_output_listener output_listener = {
	.geometry = wl_output_geometry_event
};

int main(int argc, const char* argv[])
{
	struct wl_display* pDisplay = wl_display_connect(NULL);
	if(!pDisplay)
	{
		printf("Failed to make display connection\n");
	}
	printf("Connection Established with Wayland Display\n");
	struct wl_registry* pRegistry = wl_display_get_registry(pDisplay);
	if(!pRegistry)
	{
		printf("Failed to obtain display registry\n");
	}

	printf("Registry for globals has been retrieved\n");
	wl_registry_add_listener(pRegistry, &registry_listener, NULL);

	wl_display_roundtrip(pDisplay);

	if( !gpOutput )
	{
		printf("Failed to retrieve global objects\n");
		return 1;
	}
	wl_output_add_listener( gpOutput, &output_listener, NULL );

	printf("Starting Client Event Loop\n");

	while( wl_display_dispatch(pDisplay) != -1 )
	{

	}
	wl_display_disconnect(pDisplay);
	printf("Client Disconnected from the Display\n");
	return 0;
}

