cmake_minimum_required (VERSION 3.2.0)

# add local C++ files
file(GLOB_RECURSE STANDALONE_SRC_FILES *.cpp)

list(APPEND SRC_FILES ${STANDALONE_SRC_FILES})

# add local directory to include path
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# add standalone application target
add_executable(standalone ${SRC_FILES})
