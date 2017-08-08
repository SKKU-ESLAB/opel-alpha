include_directories(./inc)

set(CMAKE_CXX_STANDARD 11)
set(CMFW_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(BluetoothCommPort OBJECT ${CMFW_SOURCE_DIR}/BluetoothCommPort.cpp)
add_library(WifiDirectCommPort OBJECT ${CMFW_SOURCE_DIR}/WifiDirectCommPort.cpp)
add_library(CommRawPacket OBJECT ${CMFW_SOURCE_DIR}/CommRawPacket.cpp)
add_library(DeviceController OBJECT ${CMFW_SOURCE_DIR}/DeviceController.cpp)
add_library(TmpControl OBJECT ${CMFW_SOURCE_DIR}/TmpControl.cpp)
add_library(opel-cmfw STATIC ${CMFW_SOURCE_DIR}/CommPort.cpp
  $<TARGET_OBJECTS:BluetoothCommPort>
  $<TARGET_OBJECTS:WifiDirectCommPort>
  $<TARGET_OBJECTS:CommRawPacket>
  $<TARGET_OBJECTS:DeviceController>
  $<TARGET_OBJECTS:TmpControl>)

target_link_libraries(opel-cmfw bluetooth)

set_target_properties(opel-cmfw
		PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "../../out/libs"
		LIBRARY_OUTPUT_DIRECTORY "../../out/lib"
		RUNTIME_OUTPUT_DIRECTORY "../../out/bin"
		COMPILE_FLAGS "-std=c++11")
