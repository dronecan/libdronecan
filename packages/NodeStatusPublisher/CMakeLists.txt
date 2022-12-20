cmake_minimum_required (VERSION 3.5)

# add local C++ files
file(GLOB_RECURSE NODESTATUSPUBLISHER_SRC_FILES *.cpp)
list(APPEND SRC_FILES NODESTATUSPUBLISHER_SRC_FILES)

# add local directory to include path
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
