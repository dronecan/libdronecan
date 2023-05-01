cmake_minimum_required (VERSION 3.5)
project(DroneCAN)

set(CMAKE_CXX_STANDARD 17)

set(DRONECAN_BASE_DIR ${CMAKE_CURRENT_LIST_DIR}/../../)

# enable debug symbols by default
set(CMAKE_BUILD_TYPE Debug)

# noexcept
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")

# find google test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${DRONECAN_BASE_DIR})
include_directories(${DRONECAN_BASE_DIR}/../libcanard)

# glob set source files from DC_PAL
file(GLOB DC_PAL_SRC
    ${DRONECAN_BASE_DIR}/DC_PAL/*.cpp
)

# glob set source files from DC_PAL_Posix
file(GLOB DC_PAL_POSIX_SRC
    ${DRONECAN_BASE_DIR}/DC_PAL_Posix/*.cpp
)

# glob set source files from DC_PAL_Ardupilot
file(GLOB DC_PAL_ARDUPILOT_SRC
    ${DRONECAN_BASE_DIR}/DC_PAL_Ardupilot/*.cpp
)

# glob set source files from DC_PAL_ChibiOS
file(GLOB DC_PAL_CHIBIOS_SRC
    ${DRONECAN_BASE_DIR}/DC_PAL_ChibiOS/*.cpp
)

enable_testing()

# function to create executables target for each test file
function(create_test_executable test_name)
    # create posix target
    add_executable(${test_name}_posix
        ${test_name}.cpp
        ${DC_PAL_POSIX_SRC}
        ${DC_PAL_SRC}
    )
    target_link_libraries(${test_name}_posix ${GTEST_MAIN_LIBRARIES} pthread)
    # set target defines
    target_compile_definitions(${test_name}_posix PRIVATE
        POSIX_BUILD
    )
endfunction()

# create_test_executable(test_callback)
create_test_executable(test_list)
create_test_executable(test_threads)
