cmake_minimum_required (VERSION 3.5)
project(DroneCAN)

# set cross compilation
include(arm-gcc-toolchain.cmake)


#check if LIBCANARD_DIR is set
if(NOT DEFINED ENV{LIBCANARD_DIR})
    set(LIBCANARD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libcanard)
endif()

if(NOT DEFINED ENV{DRONECAN_DSDL_DIR})
    set(DRONECAN_DSDL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../DSDL)
endif()

if(NOT DEFINED ENV{CHIBIOS_DIR})
    set(CHIBIOS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ChibiOS)
endif()

if(NOT DEFINED ENV{DRONECAN_DSDLC})
    set(DRONECAN_DSDLC ${CMAKE_CURRENT_SOURCE_DIR}/../dronecan_dsdlc)
endif()

set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)


# execute process to generate libcanard headers using dronecan_dsdlc/dronecan_dsdlc.py
execute_process(COMMAND python3 ${DRONECAN_DSDLC}/dronecan_dsdlc.py
    -O ${CMAKE_CURRENT_BINARY_DIR}/dsdlc_generated
    ${DRONECAN_DSDL_DIR}/uavcan
    ${DRONECAN_DSDL_DIR}/dronecan
    ${DRONECAN_DSDL_DIR}/ardupilot
    ${DRONECAN_DSDL_DIR}/com
    )

# generate dsdl messages
include_directories(${CMAKE_CURRENT_BINARY_DIR}/dsdlc_generated/include)

# add local directory to include path
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${LIBCANARD_DIR})

# add libcanard source files
file(GLOB LIBCANARD_SOURCES ${LIBCANARD_DIR}/*.c)
list(APPEND SRC_FILES ${LIBCANARD_SOURCES})

# glob all generated dsdlc files
file(GLOB DSDL_GENERATED_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/dsdlc_generated/src/*.c*)

# append DSDL_GENERATED_SOURCES_C to SRC_FILES
list(APPEND C_SRC_FILES ${DSDL_GENERATED_SOURCES})

# add_subdirectory(${MODULES_DIR}/googletest)

# disable GMOCK
# set(BUILD_GMOCK OFF BOOL FORCE)

# make a function to convert a file to a list
function(file_to_list filename listname)
    file(READ ${filename} ${listname})
    string(REPLACE "-I" "" ${listname} ${${listname}})
    string(REGEX REPLACE "\n$" "" ${listname} ${${listname}})
    set(${listname} ${${listname}} PARENT_SCOPE)
endfunction()

function(add_chibios_build platform project)
    string(REPLACE "/" "_" project_name ${project})

    execute_process(COMMAND make -f ${CMAKE_CURRENT_SOURCE_DIR}/chibios.mk -n lib
        PROJECT=chibios_${platform}
        SRCDIR=${CMAKE_CURRENT_SOURCE_DIR}
        BUILDDIR=${CMAKE_CURRENT_BINARY_DIR}
        PLATFORM=${platform}
        CHIBIOS=${CHIBIOS_DIR}
        OUTPUT_QUIET
    )
    # read incdirlist.txt to get chibios include directories
    file_to_list(${CMAKE_CURRENT_BINARY_DIR}/incdirlist_${platform}.txt CHIBIOS_INCLUDE_DIRS)
    #replace space with ;
    string(REPLACE " " ";" CHIBIOS_INCLUDE_DIRS "${CHIBIOS_INCLUDE_DIRS}")

    # read cxxflags_${platform}.txt to get chibios cxxflags
    file_to_list(${CMAKE_CURRENT_BINARY_DIR}/cxxflags_${platform}.txt CHIBIOS_CXXFLAGS)

    # read cflags_${platform}.txt to get chibios cflags
    file_to_list(${CMAKE_CURRENT_BINARY_DIR}/cflags_${platform}.txt CHIBIOS_CFLAGS)

    # read ldflags_${platform}.txt to get chibios ldflags
    file_to_list(${CMAKE_CURRENT_BINARY_DIR}/ldflags_${platform}.txt CHIBIOS_LDFLAGS)

    # read srcfiles_${platform}.txt to get chibios source files
    file_to_list(${CMAKE_CURRENT_BINARY_DIR}/srcfiles_${platform}.txt CHIBIOS_SRC_FILES)
    # replace space with ;
    string(REPLACE " " ";" CHIBIOS_SRC_FILES "${CHIBIOS_SRC_FILES}")

    # check if platform contains m4
    string(FIND ${platform} "m4" M4_FOUND)
    if(${M4_FOUND} GREATER -1)
        set(UDEFS "-DCORE_CM4")
    else()
        set(UDEFS "-DCORE_CM7")
    endif()

    # add chibios library target
    add_custom_command(OUTPUT ${platform}/libchibios_${platform}.a ${platform}/obj
                       COMMAND make -f ${CMAKE_CURRENT_SOURCE_DIR}/chibios.mk lib
                       PROJECT=chibios_${platform}
                       SRCDIR=${CMAKE_CURRENT_SOURCE_DIR}
                       BUILDDIR=${CMAKE_CURRENT_BINARY_DIR}/${platform}
                       PLATFORM=${platform}
                       UDEFS=${UDEFS}
                       CHIBIOS=${CHIBIOS_DIR}
                       DEPENDS ${CHIBIOS_SRC_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/chibios.mk
    )
    add_custom_target(chibios_${platform} ALL DEPENDS ${platform}/libchibios_${platform}.a)

    # add all sources under project directory
    file(GLOB PROJECT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${project}/*.cpp)
    # if project sources are empty, error
    if(NOT PROJECT_SOURCES)
        message(FATAL_ERROR "No source files found for project ${project}")
    endif()
    # add all files under dronecan directory
    file(GLOB CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/dronecan/*.cpp)
    list(APPEND CPP_SOURCES ${PROJECT_SOURCES} ${CORE_SOURCES})
    # set CXX flags for CPP_SOURCES
    string(REGEX REPLACE "-Wa,-alms=.*" "" CHIBIOS_CXXFLAGS "${CHIBIOS_CXXFLAGS}")
    set_source_files_properties(${CPP_SOURCES} PROPERTIES COMPILE_FLAGS ${CHIBIOS_CXXFLAGS})

    # set C flags for C_SRC_FILES
    string(REGEX REPLACE "-Wa,-alms=.*" "" CHIBIOS_CFLAGS "${CHIBIOS_CFLAGS}")
    set_source_files_properties(${C_SRC_FILES} PROPERTIES COMPILE_FLAGS ${CHIBIOS_CFLAGS})

    # create executable
    add_executable(${project_name}_${platform} ${CPP_SOURCES} ${C_SRC_FILES})
    add_dependencies(${project_name}_${platform} chibios_${platform})
    # add chibios library to link
    target_link_libraries(${project_name}_${platform} ${CMAKE_CURRENT_BINARY_DIR}/${platform}/libchibios_${platform}.a)
    # add chibios include directories
    target_include_directories(${project_name}_${platform} PUBLIC ${CHIBIOS_INCLUDE_DIRS})
    # also add cpp wrapper include directory
    target_include_directories(${project_name}_${platform} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/modules/ChibiOS/os/various/cpp_wrappers)

    if(${M4_FOUND} GREATER -1)
        set_target_properties(${project_name}_${platform} PROPERTIES COMPILE_DEFINITIONS "CORE_CM4")
    else()
        set_target_properties(${project_name}_${platform} PROPERTIES COMPILE_DEFINITIONS "CORE_CM7")
    endif()
    set_target_properties(${project_name}_${platform} PROPERTIES LINK_FLAGS "${CHIBIOS_LDFLAGS}")
    # generate hex file
    add_custom_command(TARGET ${project_name}_${platform} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${project_name}_${platform} ${project_name}_${platform}.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary ${project_name}_${platform} ${project_name}_${platform}.bin
    )
    add_custom_target(${project_name}_${platform}_hex ALL DEPENDS ${project_name}_${platform})
    # show size
    add_custom_command(TARGET ${project_name}_${platform} POST_BUILD
        COMMAND ${CMAKE_SIZE_UTIL} -B ${project_name}_${platform}
    )
endfunction()

# add shared_mem project
function(add_dual_core_build platform project)
    string(REPLACE "/" "_" project_name ${project})
    add_chibios_build(${platform} ${project})
    add_chibios_build(${platform}_m4 ${project})

    add_custom_command(OUTPUT ${project_name}.hex
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/combine_hex.py ${CMAKE_CURRENT_BINARY_DIR}/${project_name}.hex ${CMAKE_CURRENT_BINARY_DIR}/${project_name}_${platform}.hex ${CMAKE_CURRENT_BINARY_DIR}/${project_name}_${platform}_m4.hex
        DEPENDS ${project_name}_${platform}.hex ${project_name}_${platform}_m4.hex
    )
    add_custom_target(${project_name} ALL DEPENDS ${project_name}.hex)
    add_dependencies(${project_name} ${project_name}_${platform} ${project_name}_${platform}_m4)
endfunction()

# Common compile options
set(CMAKE_CXX_FLAGS "-mno-thumb-interwork -mthumb --specs=nano.specs --specs=nosys.specs")
set(CMAKE_C_FLAGS "-mno-thumb-interwork -mthumb --specs=nano.specs --specs=nosys.specs")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-mno-thumb-interwork -mthumb --specs=nano.specs --specs=nosys.specs")

# Build Targets
add_dual_core_build(stm32h7xx tests/shared_mem)
