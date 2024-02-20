SERVER_LIBS := -lwayland-server -lrt 
SIMPLE_CLIENT_LIBS :=  -lrt -lwayland-client
EGL_CLIENT_LIBS := -lEGL -lGLESv2 -lrt -lwayland-client -lwayland-egl

WAYLAND_INCLUDE_DIR := /usr/include 

CFLAGS = -I ${WAYLAND_INCLUDE_DIR}

CC := gcc

all: make_build_dir sample_client output_event_handle global_object_list surface_creation xdg_shell_client egl_info egl_sample egl_client

make_build_dir:
	@echo "creating build directory"
	@mkdir -p ./build

sample_client: make_build_dir
	@echo "building $@"
	@${CC} ./$@.c ${CFLAGS} -o ./build/$@ ${SIMPLE_CLIENT_LIBS}

output_event_handle : make_build_dir
	@echo "building $@"
	@${CC} ./$@.c ${CFLAGS} -o ./build/$@ ${SIMPLE_CLIENT_LIBS}

global_object_list : make_build_dir
	@echo "building $@"
	@${CC} ./$@.c ${CFLAGS} -o ./build/$@ ${SIMPLE_CLIENT_LIBS}

surface_creation : make_build_dir
	@echo "building $@"
	@${CC} ./$@.c ./xdg-shell-protocol.c ${CFLAGS} -o ./build/$@ ${SIMPLE_CLIENT_LIBS}

xdg_shell_client : make_build_dir
	@echo "building $@"
	@${CC} ./$@.c ./xdg-shell-protocol.c ${CFLAGS} -o ./build/$@ ${SIMPLE_CLIENT_LIBS}

egl_info : make_build_dir
	@echo "building $@"
	@${CC} ./$@.c ${CFLAGS} -o ./build/$@ -lEGL

egl_sample : make_build_dir
	@echo "building $@"
	@${CC} ./$@.c ./xdg-shell-protocol.c ${CFLAGS} -o ./build/$@ ${EGL_CLIENT_LIBS}

egl_client : make_build_dir
	@echo "building $@"
	@${CC} ./$@.c ./xdg-shell-protocol.c ${CFLAGS} -o ./build/$@ ${EGL_CLIENT_LIBS}

clean :
	@echo "Cleaning build"
	@rm -r ./build




