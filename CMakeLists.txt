cmake_minimum_required (VERSION 3.5)
project(CubeFramework)

# set cross compilation
include(arm-gcc-toolchain.cmake)


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
    execute_process(COMMAND make -f ${CMAKE_CURRENT_SOURCE_DIR}/chibios.mk -n lib
        PROJECT=chibios_${platform}
        SRCDIR=${CMAKE_CURRENT_SOURCE_DIR}
        BUILDDIR=${CMAKE_CURRENT_BINARY_DIR}
        PLATFORM=${platform}
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
                       DEPENDS ${CHIBIOS_SRC_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/chibios.mk
    )
    add_custom_target(chibios_${platform} DEPENDS ${platform}/libchibios_${platform}.a)

    # add all sources under project directory
    file(GLOB PROJECT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${project}/*.cpp)
    # add all files under core directory
    file(GLOB CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp)
    list(APPEND CPP_SOURCES ${PROJECT_SOURCES} ${CORE_SOURCES})
    # set CXX flags for CPP_SOURCES
    set_source_files_properties(${CPP_SOURCES} PROPERTIES COMPILE_FLAGS ${CHIBIOS_CXXFLAGS})
    # set C flags for C_SRC_FILES
    set_source_files_properties(${C_SRC_FILES} PROPERTIES COMPILE_FLAGS ${CHIBIOS_CFLAGS})
    # create executable
    add_executable(${project}_${platform} ${CPP_SOURCES} ${C_SRC_FILES})
    add_dependencies(${project}_${platform} chibios_${platform})
    # add chibios library to link
    target_link_libraries(${project}_${platform} ${CMAKE_CURRENT_BINARY_DIR}/${platform}/libchibios_${platform}.a)
    # add chibios include directories
    target_include_directories(${project}_${platform} PUBLIC ${CHIBIOS_INCLUDE_DIRS})
    # also add cpp wrapper include directory
    target_include_directories(${project}_${platform} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/modules/ChibiOS/os/various/cpp_wrappers)

    if(${M4_FOUND} GREATER -1)
        set_target_properties(${project}_${platform} PROPERTIES COMPILE_DEFINITIONS "CORE_CM4")
    else()
        set_target_properties(${project}_${platform} PROPERTIES COMPILE_DEFINITIONS "CORE_CM7")
    endif()
    set_target_properties(${project}_${platform} PROPERTIES LINK_FLAGS "${CHIBIOS_LDFLAGS}")
    # generate hex file
    add_custom_command(TARGET ${project}_${platform} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${project}_${platform} ${project}_${platform}.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary ${project}_${platform} ${project}_${platform}.bin
    )
    add_custom_target(${project}_${platform}_hex ALL DEPENDS ${project}_${platform})
    # show size
    add_custom_command(TARGET ${project}_${platform} POST_BUILD
        COMMAND ${CMAKE_SIZE_UTIL} -B ${project}_${platform}
    )
endfunction()

# set compile options
set(CMAKE_CXX_FLAGS "-mno-thumb-interwork -mthumb --specs=nano.specs --specs=nosys.specs")
set(CMAKE_C_FLAGS "-mno-thumb-interwork -mthumb --specs=nano.specs --specs=nosys.specs")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-mno-thumb-interwork -mthumb --specs=nano.specs --specs=nosys.specs")

# add shared_mem project
add_chibios_build(stm32h7xx shared_mem_test)
add_chibios_build(stm32h7xx_m4 shared_mem_test)

# combine hex files
add_custom_command(OUTPUT shared_mem_test.hex
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/combine_hex.py ${CMAKE_CURRENT_BINARY_DIR}/shared_mem_test.hex ${CMAKE_CURRENT_BINARY_DIR}/shared_mem_test_stm32h7xx.hex ${CMAKE_CURRENT_BINARY_DIR}/shared_mem_test_stm32h7xx_m4.hex
    DEPENDS shared_mem_test_stm32h7xx.hex shared_mem_test_stm32h7xx_m4.hex
)
add_custom_target(shared_mem_test ALL DEPENDS shared_mem_test.hex)
add_dependencies(shared_mem_test shared_mem_test_stm32h7xx shared_mem_test_stm32h7xx_m4)
