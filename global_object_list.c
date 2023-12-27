#include <stdio.h>
#include <wayland-client.h>

static void registry_handle_global(
	void* pData, struct wl_registry* pRegistry,
	uint32_t name, const char* pInterface, uint32_t version
)
{
	printf("interface: %s, version: %d, name: %d\n",
		pInterface, version, name );
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
	struct wl_registry* pRegistry = wl_display_get_registry(pDisplay);
	if(!pRegistry)
	{
		printf("Failed to obtain display registry\n");
	}
	printf("Registry for globals has been retrieved\n");
	wl_registry_add_listener(pRegistry, &registry_listener, NULL);
	wl_display_roundtrip(pDisplay);
	wl_display_disconnect(pDisplay);
	printf("Client Disconnected from the Display\n");
	return 0;
}
