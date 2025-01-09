cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

project(ImGuiVulkanHppImage LANGUAGES C CXX)

if(APPLE)
    enable_language(OBJC)
endif()

add_subdirectory(lib)

find_package(Vulkan REQUIRED)
find_package(glslang REQUIRED)
find_package(SPIRV-Tools)
find_package(SPIRV-Tools-tools)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

set(IMGUI_DIR lib/imgui)

add_executable(${PROJECT_NAME}
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${SOURCES}
)
include_directories(
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${Vulkan_INCLUDE_DIRS}
    src
)

find_package(PkgConfig MODULE REQUIRED)
pkg_check_modules(SHADERC shaderc QUIET IMPORTED_TARGET)
# This is needed due to shaderc not providing a programmatic way to derive the path of the glslc executable
if(SHADERC_FOUND)
	cmake_path(GET SHADERC_INCLUDE_DIRS PARENT_PATH SHADERC_PREFIX)
endif()

find_library(SHADERC_LIB shaderc_combined $ENV{VULKAN_SDK}/lib)
message(STATUS "Found shaderc in: ${SHADERC_LIB}")
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3 ${Vulkan_LIBRARIES} ${SHADERC_LIB}
    Vulkan::Vulkan
    Vulkan::Headers
    glslang::OSDependent
    glslang::glslang
    glslang::MachineIndependent
    glslang::GenericCodeGen
    glslang::OGLCompiler
    glslang::SPVRemapper
    glslang::SPIRV
    SPIRV-Tools
    SPIRV-Tools-opt
    SPIRV-Tools-static
)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wno-elaborated-enum-base)
