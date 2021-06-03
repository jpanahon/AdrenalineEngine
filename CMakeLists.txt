cmake_minimum_required(VERSION 3.19)
project(Adrenaline-Engine)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fsanitize=address -std=c++17")
set(RENDERER "engine/renderer")
set(CMAKE_BUILD_TYPE Debug)

file(GLOB SOURCE_FILES "main.cpp" "engine/renderer/*.cpp" "engine/*.cpp" "lib/imgui/*.cpp")
file(GLOB HEADER_FILES "engine/renderer/*.h" "engine/*.h" "lib/imgui/*.h" "lib/stb/*.h" "lib/vma/*.h" "lib/tinyobjloader/*.h")

include_directories(${CMAKE_CURRENT_DIRECTORY} "engine/renderer" "engine/" "lib/imgui" "lib/vma" "lib/tinyobjloader" "lib/stb")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)

if (VULKAN_FOUND)
	message(STATUS "Found Vulkan, Including and Linking now")
  	include_directories(
	${Vulkan_INCLUDE_DIRS}
    	/usr/local/lib
  	)
	target_link_libraries (${PROJECT_NAME} ${Vulkan_LIBRARIES} glfw)
endif (VULKAN_FOUND)
 