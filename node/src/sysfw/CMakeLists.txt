cmake_minimum_required(VERSION 2.8)

add_custom_target(
  nil-appsys ALL
  COMMAND node-gyp configure
  COMMAND node-gyp build
  COMMAND mkdir -p ${CMAKE_SOURCE_DIR}/node/sysfw
  COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/build/Release/nil.node ${CMAKE_SOURCE_DIR}/node/sysfw/
  COMMENT "Building NIL for App/Sys API"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
