#include <stdio.h>
#include <wayland-server.h>

int main( int argc, const char* argv[] )
{
	struct wl_display* pDisplay = wl_display_create();
	if( !pDisplay )
	{
		printf("Failed to create Wayland Display\n");
		return 1;
	}

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
