// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <cinttypes>
#include <cstddef>

namespace dearnes {

enum CpuFlag : uint8_t {
    C = (0b1 << 0),  // Carry Bit
    Z = (0b1 << 1),  // Zero
    I = (0b1 << 2),  // Disable Interrupts
    D = (0b1 << 3),  // Decimal Mode
    B = (0b1 << 4),  // Break
    U = (0b1 << 5),  // Unused
    V = (0b1 << 6),  // Overflow
    N = (0b1 << 7),  // Negative
};

static constexpr size_t SIZE_CPU_RAM = 0x0800;

static constexpr size_t NUM_CONTROLLERS = 2;
static constexpr size_t CONTROLLER_PLAYER_1_IDX = 0;
static constexpr size_t CONTROLLER_PLAYER_2_IDX = 1;

enum class CartridgeLoaderError {
    FILE_NOT_FOUND,
    MAPPER_NOT_SUPPORTED,
    OK
};

}  // namespace dearnes