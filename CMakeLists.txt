include(FindPkgConfig)

set(CAMFW_SRC_DIR ${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_definitions(`pkg-config --libs --cflags dbus-1 glib-2.0 dbus-glib-1 gstreamer-1.0` -g)


add_executable(cam_fw ${CAMFW_SRC_DIR}/main.cpp ${CAMFW_SRC_DIR}/OPELgstElement.cpp ${CAMFW_SRC_DIR}/OPELgstElementTx1.cpp ${CAMFW_SRC_DIR}/OPELcamProperty.cpp ${CAMFW_SRC_DIR}/OPELcamRequest.cpp ${CAMFW_SRC_DIR}/OPELcommonUtil.cpp ${CAMFW_SRC_DIR}/OPELglobalRequest.cpp ${CAMFW_SRC_DIR}/OPELrawRequest.cpp ${CAMFW_SRC_DIR}/OPELreplyDBus.cpp ${CAMFW_SRC_DIR}/OPELh264Streaming.cpp)
	


target_link_libraries(cam_fw dbus-1 glib-2.0 dbus-glib-1 gstreamer-1.0 gstapp-1.0 gobject-2.0 gmodule-2.0 boost_serialization pthread)

