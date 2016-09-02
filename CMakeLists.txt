include(FindPkgConfig)

include_directories(./include)

add_definitions(`pkg-config --libs --cflags dbus-1 glib-2.0 dbus-glib-1 gstreamer-1.0`)

add_executable(cam_fw ./src/main.cpp ./src/OPELgstElement.cpp ./src/OPELgstElementTx1.cpp ./src/OPELcamProperty.cpp)

target_link_libraries(cam_fw dbus-1 glib-2.0 dbus-glib-1 gstapp-1.0 gstreamer-1.0 gobject-2.0 gmodule-2.0)

