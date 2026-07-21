# Copyright (c) 2020-2026 Emmanuel Arias
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(dear_nes)

# Enable parallel compilation for Visual Studio
if(MSVC)
	add_compile_options(/MP)
endif()

option(BUILD_LEGACY_V1 "Build legacy v1 targets" OFF)

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

# ImGui (docking branch)
FetchContent_Declare(
	imgui
	GIT_REPOSITORY https://github.com/ocornut/imgui.git
	GIT_TAG        docking
)

FetchContent_MakeAvailable(fmt glfw imgui)

add_executable(dear_nes_v2 src/new_main.cpp)

target_include_directories(dear_nes_v2 PRIVATE
	"${CMAKE_SOURCE_DIR}/src"
	"${CMAKE_SOURCE_DIR}/src/include"
)

target_link_libraries(dear_nes_v2 PRIVATE
	fmt::fmt
)

set_property(TARGET dear_nes_v2 PROPERTY CXX_STANDARD 17)
set_property(TARGET dear_nes_v2 PROPERTY CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

if(BUILD_LEGACY_V1)
	set(
		dear_nes_lib_source_files
		${CMAKE_SOURCE_DIR}/src/bus.cpp
		${CMAKE_SOURCE_DIR}/src/cartridge.cpp
		${CMAKE_SOURCE_DIR}/src/cartridge_header.cpp
		${CMAKE_SOURCE_DIR}/src/cartridge_loader.cpp
		${CMAKE_SOURCE_DIR}/src/cpu.cpp
		${CMAKE_SOURCE_DIR}/src/dma.cpp
		${CMAKE_SOURCE_DIR}/src/mapper.cpp
		${CMAKE_SOURCE_DIR}/src/mapper_000.cpp
		${CMAKE_SOURCE_DIR}/src/nes.cpp
		${CMAKE_SOURCE_DIR}/src/ppu.cpp
	)

	set(
		dear_nes_lib_header_files
		${CMAKE_SOURCE_DIR}/src/include/dear_nes_lib/bus.h
		${CMAKE_SOURCE_DIR}/src/include/dear_nes_lib/cartridge.h
		${CMAKE_SOURCE_DIR}/src/include/dear_nes_lib/cartridge_header.h
		${CMAKE_SOURCE_DIR}/src/include/dear_nes_lib/cartridge_loader.h
		${CMAKE_SOURCE_DIR}/src/include/dear_nes_lib/cpu.h
		${CMAKE_SOURCE_DIR}/src/include/dear_nes_lib/dma.h
		${CMAKE_SOURCE_DIR}/src/include/dear_nes_lib/enums.h
		${CMAKE_SOURCE_DIR}/src/include/dear_nes_lib/mapper.h
		${CMAKE_SOURCE_DIR}/src/include/dear_nes_lib/mapper_000.h
		${CMAKE_SOURCE_DIR}/src/include/dear_nes_lib/nes.h
		${CMAKE_SOURCE_DIR}/src/include/dear_nes_lib/ppu.h
	)

	add_library(dear_nes_lib STATIC ${dear_nes_lib_header_files} ${dear_nes_lib_source_files})
	target_include_directories(dear_nes_lib PUBLIC ${CMAKE_SOURCE_DIR}/src/include)
	set_property(TARGET dear_nes_lib PROPERTY CXX_STANDARD 17)
	set_property(TARGET dear_nes_lib PROPERTY CXX_STANDARD_REQUIRED ON)

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

	set(LIBS glfw GLAD IMGUI fmt::fmt dear_nes_lib)

	if(WIN32)
		list(APPEND LIBS opengl32)
	elseif(UNIX AND NOT APPLE)
		list(APPEND LIBS GL)
	endif()

	include_directories(
		"${CMAKE_SOURCE_DIR}/src"
		"${CMAKE_SOURCE_DIR}/src/include"
		"${CMAKE_SOURCE_DIR}/external"
		"${glfw_SOURCE_DIR}/include"
		"${imgui_SOURCE_DIR}"
		"${imgui_SOURCE_DIR}/backends"
	)

	target_link_libraries(${PROJECT_NAME} ${LIBS})

	set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
	set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
endif()
