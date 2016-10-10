set(SRC_DIR ./src)
include_directories(./inc)
include_directories(/usr/local/xml/include/libxml2)
link_directories(/usr/local/xml/lib)
link_directories(../lib)

add_definitions("`pkg-config --libs --cflags dbus-1 glib-2.0 dbus-glib-1`")

find_library(OPEL_LIBZ z ${SRC_DIR}/libz)
add_library(remoteFileManager OBJECT ${SRC_DIR}/remoteFileManager.cpp)
add_library(globalData OBJECT ${SRC_DIR}/globalData.cpp)
add_library(jsonString OBJECT ${SRC_DIR}/jsonString.cpp)
add_library(commManager OBJECT ${SRC_DIR}/commManager.cpp)
add_library(miniunz OBJECT ${SRC_DIR}/libz/miniunz.c ${SRC_DIR}/libz/ioapi.c ${SRC_DIR}/libz/unzip.c)
add_library(appPackageManager OBJECT ${SRC_DIR}/appPackageManager.cpp)
add_library(appPackageRepo OBJECT ${SRC_DIR}/appPackageRepo.cpp)
add_library(appStatusManager OBJECT ${SRC_DIR}/appStatusManager.cpp)
add_library(repoClass OBJECT ${SRC_DIR}/_repoClass.cpp)
add_library(DbusManager OBJECT ${SRC_DIR}/DbusManager.cpp)
add_executable(sysAppManager ${SRC_DIR}/mainSysAppManager $<TARGET_OBJECTS:remoteFileManager> $<TARGET_OBJECTS:globalData> $<TARGET_OBJECTS:jsonString> $<TARGET_OBJECTS:commManager> $<TARGET_OBJECTS:miniunz> $<TARGET_OBJECTS:appPackageManager> $<TARGET_OBJECTS:appPackageRepo> $<TARGET_OBJECTS:appStatusManager> $<TARGET_OBJECTS:repoClass> $<TARGET_OBJECTS:DbusManager>)

set_target_properties(sysAppManager
		PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "../bin/appManager")
target_link_libraries(sysAppManager xml2 dbus-glib-1 dbus-1 glib-2.0 pthread bluetooth sqlite3 ${OPEL_LIBZ} opel_cmfw)
