cmake_minimum_required(VERSION 2.8)
include(FindPkgConfig)

set(CAMFW_SRC_DIR ./src)
include_directories(./inc)

add_definitions(`pkg-config --libs --cflags dbus-1 glib-2.0 dbus-glib-1 gstreamer-1.0`)

add_executable(opel-camera ${CAMFW_SRC_DIR}/main.cpp ${CAMFW_SRC_DIR}/OPELgstElement.cpp ${CAMFW_SRC_DIR}/OPELgstElementTx1.cpp ${CAMFW_SRC_DIR}/OPELcamProperty.cpp ${CAMFW_SRC_DIR}/OPELcamRequest.cpp ${CAMFW_SRC_DIR}/OPELcommonUtil.cpp ${CAMFW_SRC_DIR}/OPELglobalRequest.cpp ${CAMFW_SRC_DIR}/OPELrawRequest.cpp ${CAMFW_SRC_DIR}/OPELreplyDBus.cpp ${CAMFW_SRC_DIR}/OPELh264Streaming.cpp)
	
target_link_libraries(opel-camera dbus-1 glib-2.0 dbus-glib-1 gstreamer-1.0 gstapp-1.0 gobject-2.0 gmodule-2.0 boost_serialization pthread)

set_target_properties(opel-camera
		PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/bin)
