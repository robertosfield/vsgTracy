cmake_minimum_required(VERSION 3.7)

project(vsgTracy
    VERSION 0.0.0
    DESCRIPTION "Tracy profiler integration with VulkanSceneGraph"
    LANGUAGES CXX
)
set(VSGTRACY_SOVERSION 0)
SET(VSGTRACY_RELEASE_CANDIDATE 0)

set(VSGTRACY_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "Root source directory of vsgTracy")
set(VSGTRACY_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE INTERNAL "Root binary directory of vsgTracy")

# set the use of C++17 globally as all examples require it
set(CMAKE_CXX_STANDARD 17)

# Find Vulkan and the VSG
if (VULKAN_SDK)
    set(ENV{VULKAN_SDK} ${VULKAN_SDK})
endif()

find_package(vsg 1.0.9 REQUIRED)
find_package(vsgXchange 1.0.5)

vsg_setup_dir_vars()
vsg_setup_build_vars()

if ( (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/tracy/CMakeLists.txt))
    find_package(Git)

    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)

    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()
endif()

option(TRACY_ENABLE "" ON)
option(TRACY_ON_DEMAND "" ON)
add_subdirectory(3rdparty/tracy)

if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})

    vsg_add_option_maintainer(
        PREFIX v
        RCLEVEL ${VSGTRACY_RELEASE_CANDIDATE}
    )

endif()

# source directory for main vsgTracy library
# add_subdirectory(src)

add_subdirectory(examples)

vsg_add_feature_summary()
