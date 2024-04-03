# Find Wayland
#
# Wayland_Client_INCLUDE_DIR
# Wayland_Server_INCLUDE_DIR
# Wayland_Egl_INCLUDE_DIR
#
# Wayland_Client_LIBRARY
# Wayland_Server_LIBRARY
# Wayland_Egl_LIBRARY
#
message("Custom FindWayland.cmake")
FIND_PATH(Wayland_Client_INCLUDE_DIR NAMES wayland-client.h)
FIND_PATH(Wayland_Server_INCLUDE_DIR NAMES wayland-server.h)
FIND_PATH(Wayland_Egl_INCLUDE_DIR NAMES wayland-egl.h)

FIND_LIBRARY(Wayland_Client_LIBRARY NAMES wayland-client)
FIND_LIBRARY(Wayland_Server_LIBRARY NAMES wayland-server)
FIND_LIBRARY(Wayland_Egl_LIBRARY NAMES wayland-egl)

set(Wayland_LIBRARIES ${Wayland_Client_LIBRARY} ${Wayland_Server_LIBRARY} ${Wayland_Egl_LIBRARY})

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    Wayland 
    FOUND_VAR
        Wayland_FOUND
    REQUIRED_VARS
        Wayland_LIBRARIES
    HANDLE_COMPONENTS
)

MARK_AS_ADVANCED(Wayland_Client_INCLUDE_DIR Wayland_Client_LIBRARY)
MARK_AS_ADVANCED(Wayland_Server_INCLUDE_DIR Wayland_Server_LIBRARY)
MARK_AS_ADVANCED(Wayland_Egl_INCLUDE_DIR Wayland_Egl_LIBRARY)
