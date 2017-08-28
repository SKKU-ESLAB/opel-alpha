include_directories(./inc)

set(CMAKE_CXX_STANDARD 11)
set(MESSAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(MESSAGE_sources
  ${MESSAGE_SOURCE_DIR}/BaseMessage.cpp
  ${MESSAGE_SOURCE_DIR}/MessageFactory.cpp)

add_library(opel-message SHARED ${MESSAGE_sources})

set_target_properties(opel-message
		PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "../../out/libs"
		LIBRARY_OUTPUT_DIRECTORY "../../out/lib"
		RUNTIME_OUTPUT_DIRECTORY "../../out/bin"
		COMPILE_FLAGS "-std=c++11")
