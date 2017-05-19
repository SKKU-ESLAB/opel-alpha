set(SRC_DIR ./src)
include_directories(./inc)
include_directories(/usr/local/xml/include/libxml2)
link_directories(/usr/local/xml/lib)
link_directories(../../out/libs)

add_definitions("`pkg-config --libs --cflags dbus-1 glib-2.0 dbus-glib-1`")

find_library(OPEL_LIBZ z ${SRC_DIR}/libz)
add_library(RemoteFileManager OBJECT ${SRC_DIR}/RemoteFileManager.cpp)
add_library(GlobalData OBJECT ${SRC_DIR}/GlobalData.cpp)
add_library(JsonString OBJECT ${SRC_DIR}/JsonString.cpp)
add_library(CommManager OBJECT ${SRC_DIR}/CommManager.cpp)
add_library(miniunz OBJECT ${SRC_DIR}/libz/miniunz.c ${SRC_DIR}/libz/ioapi.c ${SRC_DIR}/libz/unzip.c)
add_library(AppPackageManager OBJECT ${SRC_DIR}/AppPackageManager.cpp)
add_library(AppPackageRepo OBJECT ${SRC_DIR}/AppPackageRepo.cpp)
add_library(AppStatusManager OBJECT ${SRC_DIR}/AppStatusManager.cpp)
add_library(RepoClass OBJECT ${SRC_DIR}/RepoClass.cpp)
add_library(DbusManager OBJECT ${SRC_DIR}/DbusManager.cpp)
add_executable(opel-appcore ${SRC_DIR}/MainSysAppManager $<TARGET_OBJECTS:RemoteFileManager> $<TARGET_OBJECTS:GlobalData> $<TARGET_OBJECTS:JsonString> $<TARGET_OBJECTS:CommManager> $<TARGET_OBJECTS:miniunz> $<TARGET_OBJECTS:AppPackageManager> $<TARGET_OBJECTS:AppPackageRepo> $<TARGET_OBJECTS:AppStatusManager> $<TARGET_OBJECTS:RepoClass> $<TARGET_OBJECTS:DbusManager>)

set_target_properties(opel-appcore
		PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/bin)
target_link_libraries(opel-appcore xml2 dbus-glib-1 dbus-1 glib-2.0 pthread bluetooth sqlite3 ${OPEL_LIBZ} opel_cmfw)
