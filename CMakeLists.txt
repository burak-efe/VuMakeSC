cmake_minimum_required(VERSION 3.30)
include(FetchContent)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

function(add_sources_recursively target_name source_dir)
    if (NOT TARGET ${target_name})
        message(FATAL_ERROR "Target ${target_name} does not exist. Please create the target before calling this function.")
    endif ()
    file(GLOB_RECURSE sources "${source_dir}/*.cpp" "${source_dir}/*.c" "${source_dir}/*.h" "${source_dir}/*.hpp")
    if (sources)
        target_sources(${target_name} PRIVATE ${sources})
    else ()
        message(WARNING "No source files found in directory: ${source_dir}")
    endif ()
endfunction()
####################################################################################################
project(VuMakeSC)
set(CMAKE_CXX_STANDARD 23)
add_compile_definitions(VULKANSC)
add_executable(VuMakeSC src/Main.cpp)
####################################################################################################
# Precompiled Headers
set(PCH
        src/common/pch.h
)
target_precompile_headers(${PROJECT_NAME} PRIVATE
        <chrono>
        <iostream>
        <print>
        <string>
        <format>
        <cstdint>
        <cassert>
        <cstdlib>
        <array>
        <vector>
        <stack>
        <algorithm>
        <functional>
        <filesystem>
        <mutex>
        <span>
        <ranges>
        ${PCH}
)
####################################################################################################
#Add Sources
add_sources_recursively(${PROJECT_NAME} src)
target_include_directories(${PROJECT_NAME} PRIVATE src/common)
target_include_directories(${PROJECT_NAME} PRIVATE src/components)
target_include_directories(${PROJECT_NAME} PRIVATE src/systems)
target_include_directories(${PROJECT_NAME} PRIVATE src/render)
target_include_directories(${PROJECT_NAME} PRIVATE src/material)
####################################################################################################
#Header Onlys
target_include_directories(${PROJECT_NAME} PRIVATE external/stb)
####################################################################################################
set(GLM_BUILD_LIBRARY OFF)
FetchContent_Declare(
        fetch_glm
        GIT_REPOSITORY https://github.com/g-truc/glm
        GIT_TAG 1.0.1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(fetch_glm)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
####################################################################################################
#FetchContent_Declare(
#        fetch_glfw
#        GIT_REPOSITORY https://github.com/glfw/glfw.git
#        GIT_TAG 3.4
#)
#set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
#set(GLFW_INSTALL OFF CACHE BOOL "")
#FetchContent_MakeAvailable(fetch_glfw)
#target_link_libraries(${PROJECT_NAME} PUBLIC glfw)
####################################################################################################
set(FASTGLTF_COMPILE_AS_CPP20 ON)
FetchContent_Declare(
        fetch_fastgltf
        GIT_REPOSITORY https://github.com/spnda/fastgltf
        GIT_TAG v0.8.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(fetch_fastgltf)
target_link_libraries(${PROJECT_NAME} PRIVATE fastgltf)
####################################################################################################
FetchContent_Declare(
        fetch_vksc_headers
        GIT_REPOSITORY https://github.com/KhronosGroup/VulkanSC-Headers
        GIT_TAG vksc1.0.17
)
FetchContent_MakeAvailable(fetch_vksc_headers)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Headers)
message(STATUS "Vulkan SC Headers Version: ${VulkanHeaders_VERSION}")
#####################################################################################################
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/loader/vulkansc-1.lib")
endif()


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/bin $<TARGET_FILE_DIR:${PROJECT_NAME}>/bin)
