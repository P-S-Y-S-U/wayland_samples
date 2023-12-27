#include <stdio.h>
#include <wayland-client.h>

int main(int argc, const char* argv[] )
{
	struct wl_display* pDisplay = wl_display_connect(NULL);
	if(!pDisplay)
	{
		printf("Failed to connect to wayland display\n");
		return 1;
	}

	printf("Connection Established!\n");
	wl_display_disconnect(pDisplay);
	printf("Display Disconnection done from client");
	return 0;
}
