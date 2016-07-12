find_package(Threads REQUIRED)
find_path(OPEL_CAMERAFRAMEWORK_DIR ./)


include_directories(${OPEL_CAMERAFRAMEWORK_DIR}/include)
add_definitions(`pkg-config --libs --cflags dbus-1 glib-2.0 dbus-glib-1` -g `pkg-config --libs --cflags dbus-1 glib-2.0 dbus-glib-1`)

add_executable(cam_manager ./src/main.cpp ./src/cam_core.cpp ./src/cam_property.cpp ./src/cam_status.cpp ./src/cam_cap_check.cpp)
target_link_libraries(cam_manager dbus-1 glib-2.0 dbus-glib-1 ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(cam_manager
		PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "../bin")
