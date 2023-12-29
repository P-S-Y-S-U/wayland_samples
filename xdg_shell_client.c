#include <stdio.h>
#include <string.h>
#include <wayland-client.h>

#include "global_registry_handle.h"
#include "shm_helper.h"

struct ClientObjState;

static void wl_buffer_release( void* pData, struct wl_buffer* pWlBuffer );
static struct wl_buffer* draw_frame( struct ClientObjState* pClientObjState );
static void xdg_surface_configure(
	void* pData, struct xdg_surface* pXdgSurface, uint32_t serial
);

struct ClientObjState
{
	struct GlobalObjectState* mpGlobalObjState;
	struct wl_surface* mpWlSurface;
	struct xdg_surface* mpXdgSurface;
	struct xdg_toplevel* mpXdgTopLevel;
};

static const struct xdg_surface_listener xdg_surface_listener = {
	.configure = xdg_surface_configure
};

static const struct wl_buffer_listener wl_buffer_listener = {
	.release = wl_buffer_release
};

static void wl_buffer_release( void* pData, struct wl_buffer* pWlBuffer )
{
	wl_buffer_destroy(pWlBuffer);
}

static struct wl_buffer* draw_frame( struct ClientObjState* pClientObjState )
{
	const int width = 640, height = 480;
	const int stride = width * 4;
	const int shm_pool_size = height * stride;

	int fd = allocate_shm_file(shm_pool_size);
	if( fd == -1 )
	{
		printf("Failed to allocate shm file\n");
		return NULL;
	}

	uint32_t *pool_data = mmap(
		NULL, shm_pool_size,
		PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0
	);

	if( pool_data == MAP_FAILED )
	{
		printf("Failed to map data from shm pool\n");
		close(fd);
		return NULL;
	}

	struct wl_shm* pShm = pClientObjState->mpGlobalObjState->mpShm;
	struct wl_shm_pool* pPool = wl_shm_create_pool(pShm, fd, shm_pool_size);

	struct wl_buffer* pWlBuffer = wl_shm_pool_create_buffer(
		pPool, 0,
	    	width, height, stride, WL_SHM_FORMAT_XRGB8888
	);
	wl_shm_pool_destroy(pPool);
	close(fd);

	// Fill Buffer
	for( int y = 0; y < height; y++ )
	{
		for( int x = 0; x < width; x++ )
		{
			if( ( x + y  / 8 * 8 ) % 16 < 8 )
				pool_data[y * width + x] = 0xFF666666;
			else
				pool_data[y * width + x] = 0xFFEEEEEE;
		}
	}

	munmap(pool_data, shm_pool_size);
	wl_buffer_add_listener(pWlBuffer, &wl_buffer_listener, NULL);
	return pWlBuffer;
}

static void xdg_surface_configure(
	void* pData, struct xdg_surface* pXdgSurface, uint32_t serial
)
{
	struct ClientObjState* pClientObjState = pData;
	xdg_surface_ack_configure(pXdgSurface, serial);

	struct wl_buffer* pBuffer = draw_frame(pClientObjState);
	wl_surface_attach(pClientObjState->mpWlSurface, pBuffer, 0, 0);
	wl_surface_commit(pClientObjState->mpWlSurface);
}

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

	if(	!gObjState.mpOutput || !gObjState.mpCompositor ||
		!gObjState.mpShm || !gObjState.mpXdgWmBase
	)
	{
		printf("Failed to retrieve global objects\n");
		return 1;
	}

	struct ClientObjState clientObjState = {0};
	clientObjState.mpGlobalObjState = &gObjState;

	clientObjState.mpWlSurface = wl_compositor_create_surface(gObjState.mpCompositor);
	if( !clientObjState.mpWlSurface )
	{
		printf("Failed to Create Wayland Surface\n");
		return 1;
	}

	clientObjState.mpXdgSurface = xdg_wm_base_get_xdg_surface(
		clientObjState.mpGlobalObjState->mpXdgWmBase, clientObjState.mpWlSurface
	);
	if( !clientObjState.mpXdgSurface )
	{
		printf("Failed to create Xdg Surface\n");
		return 1;
	}
	xdg_surface_add_listener(
		clientObjState.mpXdgSurface,
		&xdg_surface_listener,
		&clientObjState
	);

	clientObjState.mpXdgTopLevel = xdg_surface_get_toplevel(clientObjState.mpXdgSurface);
	xdg_toplevel_set_title(clientObjState.mpXdgTopLevel, "Example Xdg Client");
	wl_surface_commit(clientObjState.mpWlSurface);

	printf("Starting Client Event Loop\n");

	while( wl_display_dispatch(pDisplay) != -1 )
	{

	}
	wl_display_disconnect(pDisplay);
	printf("Client Disconnected from the Display\n");
	return 0;
}
