# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/stream_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/stream_autogen.dir/ParseCache.txt"
  "stream_autogen"
  )
endif()
