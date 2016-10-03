cmake_minimum_required(VERSION 2.8)
PROJECT(sensor_manager)

include(FindPkgConfig)

#dependencies
pkg_check_modules(GLIB2	glib-2.0)
pkg_check_modules(DBUS1	dbus-1)
pkg_check_modules(DBUSGLIB1 dbus-glib-1)

FIND_PACKAGE(PkgConfig)
add_definitions("`pkg-config --libs --cflags dbus-1 glib-2.0 dbus-glib-1`")

set(MYLIB_PATH /home/pi)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE RELEASE)
set(SM_sources sensor_manager.c request.c devices.c)
set(SM_driver_sources sensor_manager.c devices.c jetsonGPIO.c sensor0_button.c)


ADD_EXECUTABLE(sensorManager ${SM_sources})
set(COMPILE_FLAGS -Wall -O2 -fPIC)

FIND_LIBRARY(LIBSENSOR NAMES libsensors.so PATHS ${CMAKE_CURRENT_SOURCE_DIR}/BUILD_SO)

TARGET_COMPILE_OPTIONS(sensorManager PRIVATE ${COMPILE_FLAGS})
TARGET_LINK_LIBRARIES(sensorManager pthread c m dbus-glib-1 dbus-1 glib-2.0 ${LIBSENSOR})


set_target_properties(sensorManager
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "../bin")

INCLUDE_DIRECTORIES(./)
INCLUDE_DIRECTORIES(/usr/local/include)
INCLUDE_DIRECTORIES(/usr/include)

LINK_DIRECTORIES(/usr/local/lib/pkgconfig)
LINK_DIRECTORIES(/usr/local/lib)
LINK_DIRECTORIES(/usr/lib/dbus-1.0)
LINK_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/libsensor.so )
LINK_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/BUILD_SO/libsensor.so )

ADD_SUBDIRECTORY(BUILD_SO)
ADD_DEPENDENCIES(sensorManager BUILD_SO/libsensors.so)


