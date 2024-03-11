cmake_minimum_required(VERSION 3.16)
project(WaylandSamples)

# CMAKE VAR SETUP
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake ${CMAKE_MODULE_PATH})

# Find CMAKE package
find_package(Rt REQUIRED)
find_package(Wayland REQUIRED)
find_package(EGL REQUIRED)
find_package(GLES2 REQUIRED)

# CMAKE C & CXX FLAGS SETUP

# compile definitions

# PROJECT VARS SETUP
set(PROJECT_BIN     "bin")
set(PROJECT_LIB     "lib")
set(PROJECT_ARCHIVE "lib/static")
set(PROJECT_INSTALL ${CMAKE_INSTALL_PREFIX})
set(PROJECT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")

# PROJECT CMAKE SCRIPT REQUIREMENTS (Dependencies cmake script)#

# CONAN SETUP #

# Additional cmake scripts in sub-directory #


set(XDG_PROTOCOL_SRCS xdg-shell-protocol.c)

##########

add_executable(SampleClient sample_client.c)
target_include_directories(SampleClient     PUBLIC              $<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}> 
                                                                $<BUILD_INTERFACE:${Wayland_Client_INCLUDE_DIR}>
                                                                )
target_link_libraries(SampleClient PUBLIC ${Wayland_Client_LIBRARY})

##########

add_executable(OutputEventHandle output_event_handle.c)
target_include_directories(OutputEventHandle PUBLIC             $<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}> 
                                                                $<BUILD_INTERFACE:${Wayland_Client_INCLUDE_DIR}>
                                                                )
target_link_libraries(OutputEventHandle PUBLIC ${Wayland_Client_LIBRARY})

##########

add_executable(GlobalObjList global_object_list.c)
target_include_directories(GlobalObjList PUBLIC                 $<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}> 
                                                                $<BUILD_INTERFACE:${Wayland_Client_INCLUDE_DIR}>
                                                                )
target_link_libraries(GlobalObjList PUBLIC ${Wayland_Client_LIBRARY})

##########

add_executable(SurfaceCreation surface_creation.c ${XDG_PROTOCOL_SRCS})
target_include_directories(SurfaceCreation PUBLIC               $<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}> 
                                                                $<BUILD_INTERFACE:${Wayland_Client_INCLUDE_DIR}>
                                                                )
target_link_libraries(SurfaceCreation PUBLIC ${Rt_LIBRARY} ${Wayland_Client_LIBRARY})

##########

add_executable(XdgShellClient xdg_shell_client.c ${XDG_PROTOCOL_SRCS})
target_include_directories(XdgShellClient PUBLIC                $<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}> 
                                                                $<BUILD_INTERFACE:${Wayland_Client_INCLUDE_DIR}>
                                                                )
target_link_libraries(XdgShellClient PUBLIC ${Rt_LIBRARY} ${Wayland_Client_LIBRARY})

##########

add_executable(EGLInfo egl_info.c)
target_include_directories(EGLInfo  PUBLIC                      $<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}> 
                                                                $<BUILD_INTERFACE:${EGL_INCLUDE_DIR}>
                                                                )
target_link_libraries(EGLInfo PUBLIC ${EGL_LIBRARY})

add_executable(GLInfo gl_info.c)
target_include_directories(GLInfo  PUBLIC                   $<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}> 
                                                            $<BUILD_INTERFACE:${EGL_INCLUDE_DIR}>
                                                            $<BUILD_INTERFACE:${GLES2_INCLUDE_DIR}>
                                                            )
target_link_libraries(GLInfo PUBLIC ${EGL_LIBRARY} ${GLES2_LIBRARY} ${Wayland_Client_LIBRARY} ${Wayland_Egl_LIBRARY}) 
##########

add_executable(EGLSample egl_sample.c ${XDG_PROTOCOL_SRCS})
target_include_directories(EGLSample  PUBLIC                    $<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}> 
                                                                $<BUILD_INTERFACE:${Wayland_Client_INCLUDE_DIR}>
                                                                $<BUILD_INTERFACE:${Wayland_Egl_INCLUDE_DIR}>
                                                                $<BUILD_INTERFACE:${GLES2_INCLUDE_DIR}>
                                                                $<BUILD_INTERFACE:${EGL_INCLUDE_DIR}>
                                                                )
target_link_libraries(EGLSample PUBLIC ${EGL_LIBRARY} ${GLES2_LIBRARY} ${Rt_LIBRARY} ${Wayland_Client_LIBRARY} ${Wayland_Egl_LIBRARY})

##########

add_executable(EGLClient egl_client.c ${XDG_PROTOCOL_SRCS})
target_include_directories(EGLClient PUBLIC                     $<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}> 
                                                                $<BUILD_INTERFACE:${Wayland_Client_INCLUDE_DIR}>
                                                                $<BUILD_INTERFACE:${Wayland_Egl_INCLUDE_DIR}>
                                                                $<BUILD_INTERFACE:${GLES2_INCLUDE_DIR}>
                                                                $<BUILD_INTERFACE:${EGL_INCLUDE_DIR}>
                                                                )
target_link_libraries(EGLClient PUBLIC ${EGL_LIBRARY} ${GLES2_LIBRARY} ${Rt_LIBRARY} ${Wayland_Client_LIBRARY} ${Wayland_Egl_LIBRARY})