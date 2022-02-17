cmake_minimum_required(VERSION 3.19)
project(Adrenaline-Engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

file(GLOB SOURCE_FILES "main.cpp" "engine/renderer/*.cpp" "engine/*.cpp" "lib/imgui/*.cpp")
file(GLOB HEADER_FILES "engine/renderer/*.h" "engine/*.h" "lib/stb/*.h" "lib/vma/*.h" "lib/tinyobjloader/*.h" "lib/glm/*.hpp" "lib/imgui/*.h")

include_directories(${CMAKE_CURRENT_DIRECTORY} "engine/renderer" "engine/" "lib/imgui" "lib/vma" "lib/tinyobjloader" "lib/stb" "lib/")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

find_package(Vulkan REQUIRED FATAL_ERROR)
add_subdirectory("lib/glfw-3.3.6")

if (VULKAN_FOUND)
	message(STATUS "Found Vulkan, Including and Linking now")
  	include_directories(${Vulkan_INCLUDE_DIRS})
	target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES} glfw)
endif (VULKAN_FOUND)


