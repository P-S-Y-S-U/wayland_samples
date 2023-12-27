#include <stdio.h>
#include <stdlib.h>

#include <wayland-server.h>

struct state;
struct output;

struct state
{
	int mX, mY;
	int mResWidth, mResHeight;
};

struct output
{
	struct wl_resource* mpResource;
	struct state*	mpState;
};

static void wl_output_handle_resource_destroy( struct wl_resource* pResource )
{
	printf("Cleaning up wl_output resource\n");

	struct output* pClientOutput = wl_resource_get_user_data(pResource);
	// TODO cleanup resource
}

static void wl_output_handle_release( struct wl_client* pClient, struct wl_resource* pResource )
{
	printf("wl_output release handle\n");
	wl_resource_destroy(pResource);
}


static const struct wl_output_interface wl_output_implementation = {
	.release = wl_output_handle_release
};

static void wl_output_handle_bind(
	struct wl_client* pClient, void* pData,
	uint32_t version, uint32_t id
)
{
	printf("Binding wl_output Version: %d id: %d\n", version, id);

	struct state* pState = pData;
	struct output* pClientOutput = calloc(1, sizeof(struct output));

	struct wl_resource* pResource = wl_resource_create(
		pClient, &wl_output_interface,
		wl_output_interface.version, id
	);
	wl_resource_set_implementation(
		pResource, &wl_output_implementation,
		pClientOutput, wl_output_handle_resource_destroy
	);

	pClientOutput->mpResource = pResource;
	pClientOutput->mpState = pState;

	wl_output_send_geometry(
		pResource, 0, 0, 1920, 1080,
		WL_OUTPUT_SUBPIXEL_UNKNOWN,
		"Foobar, Inc", "Foobar model",
		WL_OUTPUT_TRANSFORM_NORMAL
	);
}

int main( int argc, const char* argv[] )
{
	struct wl_display* pDisplay = wl_display_create();
	if( !pDisplay )
	{
		printf("Failed to create Wayland Display\n");
		return 1;
	}

	struct state displayState = {
		0, 0,
		1920, 1080
	};

	printf("Creating Global wl_output Object\n");
	wl_global_create(
		pDisplay, &wl_output_interface,
		wl_output_interface.version,
		&displayState, wl_output_handle_bind
	);

	const char* pSocket = wl_display_add_socket_auto(pDisplay);
	if(!pSocket)
	{
		printf("Failed to add socket to Wayland Display\n");
		return 1;
	}

	printf("Running Wayland Display on %s\n",pSocket);
	wl_display_run(pDisplay);
	printf("Wayland Display %s is about to be destroyed\n", pSocket);
	wl_display_destroy(pDisplay);
	return 0;
}
