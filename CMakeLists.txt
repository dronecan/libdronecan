cmake_minimum_required (VERSION 3.5)
project(CubeFramework)

# set cross compilation
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(CMAKE_SIZE arm-none-eabi-size)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_RANLIB arm-none-eabi-ranlib)


set(INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/interface)
set(MODULES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/modules)
set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)


# execute process to generate libcanard headers using dronecan_dsdlc/dronecan_dsdlc.py
execute_process(COMMAND python3 ${MODULES_DIR}/dronecan_dsdlc/dronecan_dsdlc.py
    -O ${CMAKE_CURRENT_BINARY_DIR}/dsdlc_generated
    ${MODULES_DIR}/DSDL/uavcan
    ${MODULES_DIR}/DSDL/dronecan
    ${MODULES_DIR}/DSDL/ardupilot
    ${MODULES_DIR}/DSDL/com
    )

# generate dsdl messages
include_directories(${CMAKE_CURRENT_BINARY_DIR}/dsdlc_generated/include)

# add local directory to include path
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/modules/libcanard/)

# add libcanard source files
file(GLOB LIBCANARD_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/modules/libcanard/*.c)
list(APPEND SRC_FILES ${LIBCANARD_SOURCES})

# glob all generated dsdlc files
file(GLOB DSDL_GENERATED_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/dsdlc_generated/src/*.c*)

# append DSDL_GENERATED_SOURCES_C to SRC_FILES
list(APPEND SRC_FILES ${DSDL_GENERATED_SOURCES})

# add_subdirectory(${MODULES_DIR}/googletest)

# disable GMOCK
# set(BUILD_GMOCK OFF BOOL FORCE)

# dry run chibios.mk to generate incdirlist.txt
execute_process(COMMAND make -f ${CMAKE_CURRENT_SOURCE_DIR}/chibios.mk -n lib
                PROJECT=chibios
                SRCDIR=${CMAKE_CURRENT_SOURCE_DIR}
                BUILDDIR=${CMAKE_CURRENT_BINARY_DIR}
)

# make a function to convert a file to a list
function(file_to_list filename listname)
    file(READ ${filename} ${listname})
    string(REPLACE "-I" "" ${listname} ${${listname}})
    string(REGEX REPLACE "\n$" "" ${listname} ${${listname}})
    set(${listname} ${${listname}} PARENT_SCOPE)
endfunction()

# read incdirlist.txt to get chibios include directories
file_to_list(${CMAKE_CURRENT_BINARY_DIR}/incdirlist.txt CHIBIOS_INCLUDE_DIRS)
#replace space with ;
string(REPLACE " " ";" CHIBIOS_INCLUDE_DIRS "${CHIBIOS_INCLUDE_DIRS}")
# add chibios include directories
include_directories(${CHIBIOS_INCLUDE_DIRS})

set(CHIBIOS_EXTRA_FLAGS "-mno-thumb-interwork -mthumb --specs=nano.specs --specs=nosys.specs")

# convert CHIBIOS_CXXFLAGS to list
file_to_list(${CMAKE_CURRENT_BINARY_DIR}/cxxflags.txt CHIBIOS_CXXFLAGS)
set(CHIBIOS_CXXFLAGS "${CHIBIOS_CXXFLAGS} ${CHIBIOS_EXTRA_FLAGS}")

# convert CHIBIOS_CFLAGS to list
file_to_list(${CMAKE_CURRENT_BINARY_DIR}/cflags.txt CHIBIOS_CFLAGS)

# convert CHIBIOS_LDFLAGS to list
file_to_list(${CMAKE_CURRENT_BINARY_DIR}/ldflags.txt CHIBIOS_LDFLAGS)
set(CHIBIOS_LDFLAGS "${CHIBIOS_LDFLAGS} ${CHIBIOS_EXTRA_FLAGS}")

# define CORE_CM4
add_definitions(-DCORE_CM4)

# call chibios.mk to build chibios library
add_custom_command(OUTPUT libchibios.a obj/
                   COMMAND make -f ${CMAKE_CURRENT_SOURCE_DIR}/chibios.mk lib
                   PROJECT=chibios
                   SRCDIR=${CMAKE_CURRENT_SOURCE_DIR}
                   BUILDDIR=${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(chibios DEPENDS libchibios.a)

# add target shared_mem executable depending on libchibios.a
add_executable(shared_mem ${CMAKE_CURRENT_SOURCE_DIR}/shared_mem/main.cpp)
add_dependencies(shared_mem chibios)
set_target_properties(shared_mem PROPERTIES LINK_FLAGS "${CHIBIOS_LDFLAGS}")
set_target_properties(shared_mem PROPERTIES COMPILE_FLAGS "${CHIBIOS_CXXFLAGS}")
# add chibios library to link
target_link_libraries(shared_mem ${CMAKE_CURRENT_BINARY_DIR}/libchibios.a)
