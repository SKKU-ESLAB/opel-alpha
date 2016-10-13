cmake_minimum_required(VERSION 2.8)
PROJECT(system_server)

include(FindPkgConfig)

set(SRC_DIR ./src)

add_executable(opeld ${SRC_DIR}/main)

set_target_properties(opeld
		PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "../bin")

LINK_DIRECTORIES(/usr/local/lib/pkgconfig)
LINK_DIRECTORIES(/usr/local/lib)
