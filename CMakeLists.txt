include(FindPkgConfig)

set(CAMFW_SRC_DIR ${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_definitions(`pkg-config --libs --cflags dbus-1 glib-2.0 dbus-glib-1 gstreamer-1.0` -g)



add_executable(cam_fw ${CAMFW_SRC_DIR}/main.cpp ${CAMFW_SRC_DIR}/OPELgstElement.cpp ${CAMFW_SRC_DIR}/OPELgstElementTx1.cpp ${CAMFW_SRC_DIR}/OPELcamProperty.cpp ${CAMFW_SRC_DIR}/OPELcamRequest.cpp)


target_link_libraries(cam_fw dbus-1 glib-2.0 dbus-glib-1 gstapp-1.0 gstreamer-1.0 gobject-2.0 gmodule-2.0 ${CMAKE_SOURCE_DIR}/third-party/libboost_serialization.a boost_serialization boost_system)

