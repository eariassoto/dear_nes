# Copyright (c) 2020 Emmanuel Arias
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)

set(DEAR_NES_LIB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src PARENT_SCOPE)

set(
    source_files_list
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bus.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cartridge.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cartridge_header.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cartridge_loader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dma.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mapper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mapper_000.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ppu.cpp
)

set(
    header_files_list
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include/dear_nes_lib/bus.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include/dear_nes_lib/cartridge.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include/dear_nes_lib/cartridge_header.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include/dear_nes_lib/cartridge_loader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include/dear_nes_lib/cpu.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include/dear_nes_lib/dma.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include/dear_nes_lib/enums.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include/dear_nes_lib/mapper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include/dear_nes_lib/mapper_000.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include/dear_nes_lib/nes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include/dear_nes_lib/ppu.h
)

add_library(${PROJECT_NAME} STATIC ${header_files_list} ${source_files_list})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/include)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/include)

install(
	TARGETS ${PROJECT_NAME}
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

install(FILES ${HEADER_FILES} DESTINATION include)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

# Create filter groups for VS solutions
foreach(source IN LISTS source_files_list)
	get_filename_component(source_path "${source}" PATH)
	source_group("Sources" FILES "${source}")
endforeach()
foreach(source IN LISTS header_files_list)
	get_filename_component(source_path "${source}" PATH)
	source_group("Includes" FILES "${source}")
endforeach()

