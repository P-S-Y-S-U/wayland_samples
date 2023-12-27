#include <stdio.h>
#include <stdlib.h>

#include <wayland-server.h>

struct Output;
struct OutputState;
struct Compositor;

struct OutputState
{
	int32_t mX, mY;
	int32_t mPhyWidth, mPhyHeight;
	int32_t mSubpixel;
	const char* mpMake;
	const char* mpModel;
	int32_t mTransform;
};

struct Output
{
	struct wl_resource* mpResource;
	struct OutputState* mpState;
};

struct Compositor
{
	struct wl_resource* mpResource;
};

// Output Handle

static void wl_output_handle_resource_destroy( struct wl_resource* pResource )
{
	printf("Cleaning up wl_output resource\n");

	struct Output* pClientOutput = wl_resource_get_user_data(pResource);
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

	struct OutputState* pState = pData;
	struct Output* pClientOutput = calloc(1, sizeof(struct Output));

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
		pResource, pState->mX, pState->mY, pState->mPhyWidth, pState->mPhyHeight,
		pState->mSubpixel,
		pState->mpMake, pState->mpModel,
		pState->mTransform
	);
}

// Compositor Handle

static void wl_compositor_handle_resource_destroy( struct wl_resource* pResource )
{
	printf("Cleaning up wl_compositor resource\n");

	struct Compositor* pCompositor = wl_resource_get_user_data(pResource);
	// TODO cleanup resource
}

static void wl_compositor_handle_create_surface(
	struct wl_client* pClient, struct wl_resource* pResource, uint32_t id
)
{
	printf("Compositor Create surface request from Client with id : %d\n", id);
}


static const struct wl_compositor_interface wl_compositor_impl = {
	.create_surface = wl_compositor_handle_create_surface
};

static void wl_compositor_handle_bind(
	struct wl_client* pClient, void* pData,
	uint32_t version, uint32_t id
)
{
	printf("Binding wl_compositor Version: %d id: %d\n", version, id);

	struct Compositor* pCompositor = calloc(1, sizeof(struct Compositor));

	struct wl_resource* pResource = wl_resource_create(
		pClient, &wl_compositor_interface,
		wl_compositor_interface.version, id
	);
	wl_resource_set_implementation( pResource, &wl_compositor_impl,
		pCompositor, wl_compositor_handle_resource_destroy
	);
	pCompositor->mpResource = pResource;
}


int main( int argc, const char* argv[] )
{
	struct wl_display* pDisplay = wl_display_create();
	if( !pDisplay )
	{
		printf("Failed to create Wayland Display\n");
		return 1;
	}

	struct OutputState displayState = {
		0, 0,
		1920, 1080,
		WL_OUTPUT_SUBPIXEL_UNKNOWN,
		"Foo Inc", "Foo Model",
		WL_OUTPUT_TRANSFORM_NORMAL
	};

	printf("Creating Global wl_output Object\n");
	wl_global_create(
		pDisplay, &wl_output_interface,
		wl_output_interface.version,
		&displayState, wl_output_handle_bind
	);

	printf("Creating Global wl_compositor Object\n");
	wl_global_create(
		pDisplay, &wl_compositor_interface,
		wl_compositor_interface.version,
		NULL, wl_compositor_handle_bind
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
