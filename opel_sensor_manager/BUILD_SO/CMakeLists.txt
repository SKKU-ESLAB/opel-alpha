cmake_minimum_required(VERSION 2.8)
PROJECT(BUILD_SO)

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

set(SM_driver_sources devices.c driver_common.c sensor1_acc.c sensor2_temp.c sensor3_sound.c sensor4_light.c sensor5_motion.c sensor6_vib.c sensor7_touch.c)

ADD_DEFINITIONS(-Wall -O2)
ADD_LIBRARY(sensors SHARED ${SM_driver_sources})


INCLUDE_DIRECTORIES(./)

INCLUDE_DIRECTORIES(/usr/local/include)
INCLUDE_DIRECTORIES(/usr/include)

LINK_DIRECTORIES(/usr/local/lib/pkgconfig)
LINK_DIRECTORIES(/usr/local/lib)
LINK_DIRECTORIES(/usr/lib/dbus-1.0)
LINK_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} )
LINK_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

