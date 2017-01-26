set(CMFW_SRC_DIR ./src)

include_directories(./inc)
add_library(cmfw_bt OBJECT ${CMFW_SRC_DIR}/cmfw_bt.cpp)
add_library(cmfw_wfd OBJECT ${CMFW_SRC_DIR}/cmfw_wfd.cpp)
add_library(tmp_control OBJECT ${CMFW_SRC_DIR}/tmp_control.cpp)

add_library(opel_cmfw STATIC ${CMFW_SRC_DIR}/opel_cmfw.cpp $<TARGET_OBJECTS:cmfw_bt> $<TARGET_OBJECTS:cmfw_wfd> $<TARGET_OBJECTS:tmp_control>)
target_link_libraries(opel_cmfw bluetooth)

set_target_properties(opel_cmfw
		PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "../../out/libs"
		LIBRARY_OUTPUT_DIRECTORY "../../out/lib"
		RUNTIME_OUTPUT_DIRECTORY "../../out/bin"
		COMPILE_FLAGS "-std=c++11")
