# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/QTserver_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/QTserver_autogen.dir/ParseCache.txt"
  "CMakeFiles/client_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/client_autogen.dir/ParseCache.txt"
  "QTserver_autogen"
  "client_autogen"
  )
endif()
