cmake_minimum_required(VERSION 2.8)
PROJECT(sensor_manager)

include(FindPkgConfig)

#dependencies
pkg_check_modules(GLIB2	glib-2.0)
pkg_check_modules(DBUS1	dbus-1)
pkg_check_modules(DBUSGLIB1 dbus-glib-1)

FIND_PACKAGE(PkgConfig)
add_definitions("`pkg-config --libs --cflags dbus-1 glib-2.0 dbus-glib-1`")

set(ROOT_DIR /home/pi/__home_directory/OPEL/sensor_manager)
set(MYLIB_PATH /home/pi)
set(CMAKE_VERBOSE_MAKEFILE ON)

ADD_EXECUTABLE(sensorManager sensor_manager.c request.c devices.c sensor1_acc.c sensor2_temp.c sensor3_sound.c sensor4_light.c sensor5_motion.c sensor6_vib.c sensor7_touch.c)

TARGET_LINK_LIBRARIES(sensorManager pthread wiringPi c m dbus-glib-1 dbus-1 glib-2.0)
set_target_properties(sensorManager
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "../bin")

INCLUDE_DIRECTORIES(./)
INCLUDE_DIRECTORIES(/usr/local/include)
INCLUDE_DIRECTORIES(/usr/include)

LINK_DIRECTORIES(/usr/local/lib/pkgconfig)
LINK_DIRECTORIES(/usr/local/lib)
LINK_DIRECTORIES(/usr/lib/dbus-1.0)



ADD_DEFINITIONS(-Wall -O2)
