cmake_minimum_required (VERSION 3.5)

enable_testing()

set(SRC_FILES_CORE ${SRC_FILES}
                   ${INTERFACE_DIR}/core_test_interface.cpp
                   test_static_core.cpp)
set(SRC_FILES_CANARD ${SRC_FILES}
                     ${INTERFACE_DIR}/canard_interface.cpp
                     test_canard_interface.cpp)

include_directories(${MODULES_DIR}/googletest/googletest/include)

# create test target
add_executable(test_core ${SRC_FILES_CORE})
add_executable(test_canard ${SRC_FILES_CANARD})

target_link_libraries(test_core GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(test_core)

target_link_libraries(test_canard GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(test_canard)
