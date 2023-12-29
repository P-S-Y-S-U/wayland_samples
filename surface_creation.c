#include <stdio.h>
#include <string.h>
#include <wayland-client.h>

#include "global_registry_handle.h"
#include "shm_helper.h"

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

	const int width = 1920, height = 1080;
	const int stride = width * 4;
	const int shm_pool_size = height * stride * 2;

	int fd = allocate_shm_file(shm_pool_size);
	if( fd == -1 )
	{
		printf("Failed to allocate shm file\n");
		return 1;
	}

	uint8_t *pool_data = mmap(
		NULL, shm_pool_size,
	    PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0
	);

	struct wl_shm* pShm = gObjState.mpShm;
	struct wl_shm_pool* pPool = wl_shm_create_pool(pShm, fd, shm_pool_size);

	int index = 0;
	int offset = height * stride * index;

	struct wl_buffer* pBuffers[2];
	struct wl_buffer* pFb0 = pBuffers[0];
	struct wl_buffer* pFb1 = pBuffers[1];

	pFb0 = wl_shm_pool_create_buffer(
		pPool, offset,
    	width, height, stride, WL_SHM_FORMAT_XRGB8888
	);

	uint32_t *pixels = (uint32_t *)&pool_data[offset];
	memset(pixels, 0, width * height * 4);

	wl_surface_attach(pSurface, pFb0, 0, 0);
	wl_surface_damage(pSurface, 0, 0, width, height);
	wl_surface_commit(pSurface);

	index = 1;
	offset = height * stride * index;

	pFb1 = wl_shm_pool_create_buffer(
		pPool, offset,
    	width, height, stride, WL_SHM_FORMAT_XRGB8888
	);

	pixels = (uint32_t *)&pool_data[offset];
	memset(pixels, 0, width * height * 4);

	printf("Starting Client Event Loop\n");

	while( wl_display_dispatch(pDisplay) != -1 )
	{

	}
	wl_display_disconnect(pDisplay);
	printf("Client Disconnected from the Display\n");
	return 0;
}
