set(SRC_DIR ./src)
include_directories(./inc)
include_directories(../cmfw/inc)
include_directories(/usr/local/xml/include/libxml2)
link_directories(/usr/local/xml/lib)
link_directories(../../out/libs)

add_definitions("`pkg-config --libs --cflags dbus-1 glib-2.0 dbus-glib-1`")

set(ACM_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(ACM_sources
  ${ACM_SOURCE_DIR}/main.cpp
  ${ACM_SOURCE_DIR}/App.cpp
  ${ACM_SOURCE_DIR}/AppCore.cpp
  ${ACM_SOURCE_DIR}/CommChannel.cpp
  ${ACM_SOURCE_DIR}/DbusChannel.cpp
  ${ACM_SOURCE_DIR}/LocalChannel.cpp
  ${ACM_SOURCE_DIR}/MessageRouter.cpp
  ${ACM_SOURCE_DIR}/BaseMessage.cpp
  ${ACM_SOURCE_DIR}/Channel.cpp
  ${ACM_SOURCE_DIR}/cJSON.c)

add_executable(opel-appcore-manager ${ACM_sources})

set_target_properties(opel-appcore-manager
		PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "../../out/bin")
  #target_link_libraries(opel-appcore-manager xml2 dbus-glib-1 dbus-1 glib-2.0 pthread bluetooth sqlite3 ${OPEL_LIBZ} opel_cmfw)
target_link_libraries(opel-appcore-manager xml2 dbus-glib-1 dbus-1 glib-2.0 pthread bluetooth sqlite3 opel_cmfw)
