# Copyright (c) 2020-2021 Emmanuel Arias
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(dear_nes)

link_directories(${CMAKE_SOURCE_DIR}/lib)

set(
	source_files_list
	${CMAKE_SOURCE_DIR}/src/base_widget.cpp
	${CMAKE_SOURCE_DIR}/src/controller_widget.cpp
	${CMAKE_SOURCE_DIR}/src/cpu_widget.cpp
	${CMAKE_SOURCE_DIR}/src/dearnes_base_widget.cpp
	${CMAKE_SOURCE_DIR}/src/dearnes_window_manager.cpp
	${CMAKE_SOURCE_DIR}/src/imgui_window_manager.cpp
	${CMAKE_SOURCE_DIR}/src/main.cpp
	${CMAKE_SOURCE_DIR}/src/ppu_nametable_widget.cpp
	${CMAKE_SOURCE_DIR}/src/ppu_palettes_widget.cpp
	${CMAKE_SOURCE_DIR}/src/ppu_pattern_table_widget.cpp
	${CMAKE_SOURCE_DIR}/src/screen_widget.cpp
	${CMAKE_SOURCE_DIR}/src/status_widget.cpp
	${CMAKE_SOURCE_DIR}/src/texture_image.cpp	
)
	
set(
	header_files_list
	${CMAKE_SOURCE_DIR}/src/include/base_widget.h
	${CMAKE_SOURCE_DIR}/src/include/controller_widget.h
	${CMAKE_SOURCE_DIR}/src/include/cpu_widget.h
	${CMAKE_SOURCE_DIR}/src/include/dearnes_base_widget.h
	${CMAKE_SOURCE_DIR}/src/include/dearnes_window_manager.h
	${CMAKE_SOURCE_DIR}/src/include/imgui_window_manager.h
	${CMAKE_SOURCE_DIR}/src/include/ppu_nametable_widget.h
	${CMAKE_SOURCE_DIR}/src/include/ppu_palettes_widget.h
	${CMAKE_SOURCE_DIR}/src/include/ppu_pattern_table_widget.h
	${CMAKE_SOURCE_DIR}/src/include/screen_widget.h
	${CMAKE_SOURCE_DIR}/src/include/status_widget.h
	${CMAKE_SOURCE_DIR}/src/include/texture_image.h
)

# Configure assets header file
configure_file(src/helpers/RootDir.h.in src/helpers/RootDir.h)
include_directories(${CMAKE_BINARY_DIR}/src)

add_executable(${PROJECT_NAME} ${header_files_list} ${source_files_list})

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

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)

set(LIBS glfw GLAD IMGUI fmt::fmt dear_nes_lib)

if(WIN32)
	list(APPEND LIBS opengl32)
elseif(UNIX AND NOT APPLE)
	list(APPEND LIBS GL)
endif()

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

# Create filter groups for VS solutions
foreach(source IN LISTS source_files_list)
	get_filename_component(source_path "${source}" PATH)
	source_group("Sources" FILES "${source}")
endforeach()
foreach(source IN LISTS header_files_list)
	get_filename_component(source_path "${source}" PATH)
	source_group("Includes" FILES "${source}")
endforeach()

