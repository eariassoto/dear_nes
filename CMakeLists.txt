# Copyright (c) 2020-2026 Emmanuel Arias
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(dear_nes)

# Enable parallel compilation for Visual Studio
if(MSVC)
	add_compile_options(/MP)
endif()

include(FetchContent)

# GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
	glfw
	GIT_REPOSITORY https://github.com/glfw/glfw.git
	GIT_TAG        3.4
)

# fmt
set(FMT_TEST OFF CACHE BOOL "" FORCE)
set(FMT_DOC OFF CACHE BOOL "" FORCE)
set(FMT_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
	fmt
	GIT_REPOSITORY https://github.com/fmtlib/fmt.git
	GIT_TAG        10.2.1
)

# spdlog
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_INSTALL OFF CACHE BOOL "" FORCE)
set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "" FORCE)
FetchContent_Declare(
	spdlog
	GIT_REPOSITORY https://github.com/gabime/spdlog.git
	GIT_TAG        v1.13.0
)

# ImGui (docking branch)
FetchContent_Declare(
	imgui
	GIT_REPOSITORY https://github.com/ocornut/imgui.git
	GIT_TAG        docking
)

FetchContent_MakeAvailable(fmt glfw spdlog imgui)

add_subdirectory(libs/dear_nes_lib)

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

# GLAD
add_library(GLAD "external/glad/glad.c")

add_library(IMGUI STATIC
	"${imgui_SOURCE_DIR}/imgui.cpp"
	"${imgui_SOURCE_DIR}/imgui_demo.cpp"
	"${imgui_SOURCE_DIR}/imgui_draw.cpp"
	"${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
	"${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp"
	"${imgui_SOURCE_DIR}/imgui_widgets.cpp"
	"${imgui_SOURCE_DIR}/imgui_tables.cpp"
)

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)

set(LIBS glfw GLAD IMGUI fmt::fmt spdlog dear_nes_lib)

if(WIN32)
	list(APPEND LIBS opengl32)
elseif(UNIX AND NOT APPLE)
	list(APPEND LIBS GL)
endif()

include_directories(
	"${CMAKE_SOURCE_DIR}/src"
	"${CMAKE_SOURCE_DIR}/external"
	"${glfw_SOURCE_DIR}/include"
	"${imgui_SOURCE_DIR}"
	"${imgui_SOURCE_DIR}/backends"
)
# ==============================================================================

target_link_libraries(${PROJECT_NAME} ${LIBS})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
