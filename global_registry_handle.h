#ifndef _GLOBAL_REGISTRY_HANDLE_H
#define _GLOBAL_REGISTRY_HANDLE_H

#include <wayland-client.h>
#include <stdio.h>

struct GlobalObjectState;
static void registry_handle_global( 
    void* pData, struct wl_registry* pRegistry,
	uint32_t name, const char* pInterface, uint32_t version 
);
static void registry_handle_global_remove( 
    void* pData, struct wl_registry* pRegistry, 
    uint32_t name 
);

struct GlobalObjectState
{
    struct wl_compositor* mpCompositor;
    struct wl_output*  mpOutput;
};

static const struct wl_registry_listener g_registryListener = {
	.global = registry_handle_global,
	.global_remove = registry_handle_global_remove
};

static void registry_handle_global(
	void* pData, struct wl_registry* pRegistry,
	uint32_t name, const char* pInterface, uint32_t version
)
{
	printf("Global Object Handle interface: %s, version: %d, name: %d\n",
		pInterface, version, name );

	struct	GlobalObjectState* pObjState = pData;

	if( strcmp("wl_compositor", pInterface) == 0 )
	{
		pObjState->mpCompositor = wl_registry_bind(
            pRegistry, name,
			&wl_compositor_interface, version
		);
	}
	else if( strcmp("wl_output", pInterface) == 0 )
	{
        pObjState->mpOutput = wl_registry_bind(
            pRegistry, name,
			&wl_output_interface, version
		);
	}
}

static void registry_handle_global_remove(
	void* pData, struct wl_registry* pRegistry,
	uint32_t name
)
{
	printf("name: %d has been removed from registry\n", name);
}


#endif 