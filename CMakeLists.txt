cmake_minimum_required(VERSION 3.25)
project(Adrenaline-Engine)

set(CMAKE_CXX_STANDARD 20)

# Comment this out to enable release mode.
add_compile_definitions(ADREN_DEBUG)

file(GLOB SOURCE_FILES 
	"main.cpp" 
	"engine/renderer/*.cpp" 
	"engine/discord/*.cpp" 
	"engine/editor/*.cpp" 
	"engine/*.cpp" 
	"lib/imgui/*.cpp" 
	"lib/discord/*.cpp" 
	"lib/fastgltf/*.cpp"
)

file(GLOB HEADER_FILES 
	"engine/renderer/*.h" 
	"engine/discord" 
	"engine/editor/*.h" 
	"engine/*.h" 
	"lib/stb/*.h" 
	"lib/vma/*.h" 
	"lib/glm/*.hpp" 
	"lib/imgui/*.h"  
	"lib/discord/*.h" 
	"lib/fastgltf/*.hpp"
	"lib/fastgltf/*.h"
)

include_directories(${CMAKE_CURRENT_DIRECTORY} 
	"engine/renderer" 
	"engine/discord" 
	"engine/editor" 
	"engine/" 
	"lib/imgui" 
	"lib/vma" 
	"lib/" 
	"lib/discord" 
	"lib/fastgltf"
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory("lib/glfw")
add_subdirectory("lib/nfd")

find_package(Vulkan REQUIRED FATAL_ERROR)

if(Vulkan_FOUND)
	message(STATUS "Found Vulkan, Including and Linking now")
    include_directories(${Vulkan_INCLUDE_DIRS})
	target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES} glfw nfd)
endif()
