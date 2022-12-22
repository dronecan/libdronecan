cmake_minimum_required (VERSION 3.5)

enable_testing()

file(GLOB_RECURSE TEST_SOURCES *.cpp)
list(APPEND SRC_FILES ${TEST_SOURCES})
list(APPEND SRC_FILES ${INTERFACE_DIR}/core_test_interface.cpp)
list(APPEND SRC_FILES ${INTERFACE_DIR}/canard_interface.cpp)

include_directories(${MODULES_DIR}/googletest/googletest/include)

# create test target
add_executable(test ${SRC_FILES})

target_link_libraries(test GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(test)
