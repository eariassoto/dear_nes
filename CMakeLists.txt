# Copyright (c) 2020 Emmanuel Arias
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(dear_nes)

link_directories(${CMAKE_SOURCE_DIR}/lib)

file(GLOB_RECURSE SOURCE_FILES 
	${CMAKE_SOURCE_DIR}/src/*.c
	${CMAKE_SOURCE_DIR}/src/*.cpp)
	
file(GLOB_RECURSE HEADER_FILES 
	${CMAKE_SOURCE_DIR}/src/*.h
	${CMAKE_SOURCE_DIR}/src/*.hpp)

# Configure assets header file
configure_file(src/helpers/RootDir.h.in src/helpers/RootDir.h)
include_directories(${CMAKE_BINARY_DIR}/src)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

# OpenGL
find_package(OpenGL REQUIRED)

# GLFW
add_subdirectory("external/glfw")

# GLAD
add_library(GLAD "external/glad/glad.c")

add_subdirectory(external/fmt)

add_subdirectory(libs/dear_nes_lib)
get_target_property(DEAR_NES_LIB_PUBLIC_HEADER_DIR dear_nes_lib INTERFACE_INCLUDE_DIRECTORIES)
include_directories("${DEAR_NES_LIB_PUBLIC_HEADER_DIR}")

# Imgui
add_library(IMGUI
	"external/imgui/imgui.cpp"
	"external/imgui/imgui_demo.cpp"
	"external/imgui/imgui_draw.cpp"
	"external/imgui/examples/imgui_impl_glfw.cpp"
	"external/imgui/examples/imgui_impl_opengl3.cpp"
	"external/imgui/imgui_widgets.cpp"
)

set(LIBS glfw opengl32 GLAD IMGUI fmt::fmt dear_nes_lib)

include_directories(
	"${CMAKE_SOURCE_DIR}/src"
	"${CMAKE_SOURCE_DIR}/external"
	"${CMAKE_SOURCE_DIR}/external/glfw/include"
	"${CMAKE_SOURCE_DIR}/external/imgui"
	"${CMAKE_SOURCE_DIR}/external/imgui/examples"
)

target_link_libraries(${PROJECT_NAME} ${LIBS})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

# Create virtual folders to make it look nicer in VS
if(MSVC_IDE)
	# Macro to preserve source files hierarchy in the IDE
	macro(GroupSources curdir)
		file(GLOB children RELATIVE ${PROJECT_SOURCE_DIR}/${curdir} ${PROJECT_SOURCE_DIR}/${curdir}/*)

		foreach(child ${children})
			if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${curdir}/${child})
				GroupSources(${curdir}/${child})
			else()
				string(REPLACE "/" "\\" groupname ${curdir})
				string(REPLACE "src" "Sources" groupname ${groupname})
				source_group(${groupname} FILES ${PROJECT_SOURCE_DIR}/${curdir}/${child})
			endif()
		endforeach()
	endmacro()

	# Run macro
	GroupSources(src)
endif()
