// Copyright (c) 2026 Emmanuel Arias
#include <fmt/base.h>

#include <cstdint>
#include <cstring>
#include <fstream>

struct iNesHeader {
    // Must be $4E $45 $53 $1A ("NES\x1A")
    char m_MagicConstant[4];
    // Size of PRG ROM in 16 KB units
    uint8_t m_PrgRomChunks;
    // Size of CHR ROM in 8 KB units
    uint8_t m_ChrRomChunks;
    uint8_t m_Mapper1;
    uint8_t m_Mapper2;
    uint8_t m_PrgRamSize;
    uint8_t m_TvSystem1;
    uint8_t m_TvSystem2;
    char m_UnusedPadding[5];
};

struct CpuState {
    uint8_t reg_a;
    uint8_t reg_x;
    uint8_t reg_y;
    uint8_t sp;
    uint8_t status;
    uint16_t pc;
    uint8_t ram[0x10000];
};

enum Addr {
    NONE = 0,
    IMMEDIATE,
    ZERO_PAGE,
    INDEXED_ZERO_PAGE_X,
    INDEXED_ZERO_PAGE_Y,
    ABSOLUTE,
    INDEXED_ABSOLUTE_X,
    INDEXED_ABSOLUTE_Y,
    ABSOLUTE_INDIRECT,
    INDEXED_INDIRECT_X,
    INDIRECT_INDEXED_Y,
    RELATIVE,
};

enum InstrCode {
    NOP = 0,
    ADC,
    AND,
    ASL,
    ASL_ACCUM_ADDR,
    EXEC_BRANCH,
    BCC,
    BCS,
    BEQ,
    BIT,
    BMI,
    BNE,
    BPL,
    BRK,
    BVC,
    BVS,
    CLC,
    CLD,
    CLI,
    CLV,
    CMP,
    CPX,
    CPY,
    DEC,
    DEX,
    DEY,
    EOR,
    INC,
    INX,
    INY,
    JMP,
    JSR,
    LDA,
    LDX,
    LDY,
    LSR,
    LSR_ACCUM_ADDR,
    ORA,
    PHA,
    PHP,
    PLA,
    PLP,
    ROL,
    ROL_ACCUM_ADDR,
    ROR,
    ROR_ACCUM_ADDR,
    RTI,
    RTS,
    SBC,
    SEC,
    SED,
    SEI,
    STA,
    STX,
    STY,
    TAX,
    TAY,
    TSX,
    TXA,
    TXS,
    TYA
};

struct Instruction {
    const InstrCode code;
    Addr addressing_mode;
    uint8_t base_cycles;
};

static Instruction nes_instructions[256] = {
    {InstrCode::BRK, Addr::NONE, 7},                 // 0x00
    {InstrCode::ORA, Addr::INDEXED_INDIRECT_X, 6},   // 0x01
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x02
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x03
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x04
    {InstrCode::ORA, Addr::ZERO_PAGE, 3},            // 0x05
    {InstrCode::ASL, Addr::ZERO_PAGE, 5},            // 0x06
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x07
    {InstrCode::PHP, Addr::NONE, 3},                 // 0x08
    {InstrCode::ORA, Addr::IMMEDIATE, 2},            // 0x09
    {InstrCode::ASL_ACCUM_ADDR, Addr::NONE, 2},      // 0x0A
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x0B
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x0C
    {InstrCode::ORA, Addr::ABSOLUTE, 4},             // 0x0D
    {InstrCode::ASL, Addr::ABSOLUTE, 6},             // 0x0E
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x0F
    {InstrCode::BPL, Addr::RELATIVE, 2},             // 0x10
    {InstrCode::ORA, Addr::INDIRECT_INDEXED_Y, 5},   // 0x11
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x12
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x13
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x14
    {InstrCode::ORA, Addr::INDEXED_ZERO_PAGE_X, 4},  // 0x15
    {InstrCode::ASL, Addr::INDEXED_ZERO_PAGE_X, 6},  // 0x16
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x17
    {InstrCode::CLC, Addr::NONE, 2},                 // 0x18
    {InstrCode::ORA, Addr::INDEXED_ABSOLUTE_Y, 4},   // 0x19
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x1A
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x1B
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x1C
    {InstrCode::ORA, Addr::INDEXED_ABSOLUTE_X, 4},   // 0x1D
    {InstrCode::ASL, Addr::INDEXED_ABSOLUTE_X, 7},   // 0x1E
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x1F
    {InstrCode::JSR, Addr::ABSOLUTE, 6},             // 0x20
    {InstrCode::AND, Addr::INDEXED_INDIRECT_X, 6},   // 0x21
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x22
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x23
    {InstrCode::BIT, Addr::ZERO_PAGE, 3},            // 0x24
    {InstrCode::AND, Addr::ZERO_PAGE, 3},            // 0x25
    {InstrCode::ROL, Addr::ZERO_PAGE, 5},            // 0x26
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x27
    {InstrCode::PLP, Addr::NONE, 4},                 // 0x28
    {InstrCode::AND, Addr::IMMEDIATE, 2},            // 0x29
    {InstrCode::ROL_ACCUM_ADDR, Addr::NONE, 2},      // 0x2A
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x2B
    {InstrCode::BIT, Addr::ABSOLUTE, 4},             // 0x2C
    {InstrCode::AND, Addr::ABSOLUTE, 4},             // 0x2D
    {InstrCode::ROL, Addr::ABSOLUTE, 6},             // 0x2E
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x2F
    {InstrCode::BMI, Addr::RELATIVE, 2},             // 0x30
    {InstrCode::AND, Addr::INDIRECT_INDEXED_Y, 5},   // 0x31
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x32
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x33
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x34
    {InstrCode::AND, Addr::INDEXED_ZERO_PAGE_X, 4},  // 0x35
    {InstrCode::ROL, Addr::INDEXED_ZERO_PAGE_X, 6},  // 0x36
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x37
    {InstrCode::SEC, Addr::NONE, 2},                 // 0x38
    {InstrCode::AND, Addr::INDEXED_ABSOLUTE_Y, 4},   // 0x39
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x3A
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x3B
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x3C
    {InstrCode::AND, Addr::INDEXED_ABSOLUTE_X, 4},   // 0x3D
    {InstrCode::ROL, Addr::INDEXED_ABSOLUTE_X, 7},   // 0x3E
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x3F
    {InstrCode::RTI, Addr::NONE, 6},                 // 0x40
    {InstrCode::EOR, Addr::INDEXED_INDIRECT_X, 6},   // 0x41
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x42
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x43
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x44
    {InstrCode::EOR, Addr::ZERO_PAGE, 3},            // 0x45
    {InstrCode::LSR, Addr::ZERO_PAGE, 5},            // 0x46
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x47
    {InstrCode::PHA, Addr::NONE, 3},                 // 0x48
    {InstrCode::EOR, Addr::IMMEDIATE, 2},            // 0x49
    {InstrCode::LSR_ACCUM_ADDR, Addr::NONE, 2},      // 0x4A
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x4B
    {InstrCode::JMP, Addr::ABSOLUTE, 3},             // 0x4C
    {InstrCode::EOR, Addr::ABSOLUTE, 4},             // 0x4D
    {InstrCode::LSR, Addr::ABSOLUTE, 6},             // 0x4E
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x4F
    {InstrCode::BVC, Addr::RELATIVE, 2},             // 0x50
    {InstrCode::EOR, Addr::INDIRECT_INDEXED_Y, 5},   // 0x51
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x52
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x53
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x54
    {InstrCode::EOR, Addr::INDEXED_ZERO_PAGE_X, 4},  // 0x55
    {InstrCode::LSR, Addr::INDEXED_ZERO_PAGE_X, 6},  // 0x56
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x57
    {InstrCode::CLI, Addr::NONE, 2},                 // 0x58
    {InstrCode::EOR, Addr::INDEXED_ABSOLUTE_Y, 4},   // 0x59
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x5A
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x5B
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x5C
    {InstrCode::EOR, Addr::INDEXED_ABSOLUTE_X, 4},   // 0x5D
    {InstrCode::LSR, Addr::INDEXED_ABSOLUTE_X, 7},   // 0x5E
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x5F
    {InstrCode::RTS, Addr::NONE, 6},                 // 0x60
    {InstrCode::ADC, Addr::INDEXED_INDIRECT_X, 6},   // 0x61
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x62
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x63
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x64
    {InstrCode::ADC, Addr::ZERO_PAGE, 3},            // 0x65
    {InstrCode::ROR, Addr::ZERO_PAGE, 5},            // 0x66
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x67
    {InstrCode::PLA, Addr::NONE, 4},                 // 0x68
    {InstrCode::ADC, Addr::IMMEDIATE, 2},            // 0x69
    {InstrCode::ROR_ACCUM_ADDR, Addr::NONE, 2},      // 0x6A
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x6B
    {InstrCode::JMP, Addr::ABSOLUTE_INDIRECT, 5},    // 0x6C
    {InstrCode::ADC, Addr::ABSOLUTE, 4},             // 0x6D
    {InstrCode::ROR, Addr::ABSOLUTE, 6},             // 0x6E
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x6F
    {InstrCode::BVS, Addr::RELATIVE, 2},             // 0x70
    {InstrCode::ADC, Addr::INDIRECT_INDEXED_Y, 5},   // 0x71
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x72
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x73
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x74
    {InstrCode::ADC, Addr::INDEXED_ZERO_PAGE_X, 4},  // 0x75
    {InstrCode::ROR, Addr::INDEXED_ZERO_PAGE_X, 6},  // 0x76
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x77
    {InstrCode::SEI, Addr::NONE, 2},                 // 0x78
    {InstrCode::ADC, Addr::INDEXED_ABSOLUTE_Y, 4},   // 0x79
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x7A
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x7B
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x7C
    {InstrCode::ADC, Addr::INDEXED_ABSOLUTE_X, 4},   // 0x7D
    {InstrCode::ROR, Addr::INDEXED_ABSOLUTE_X, 7},   // 0x7E
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x7F
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x80
    {InstrCode::STA, Addr::INDEXED_INDIRECT_X, 6},   // 0x81
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x82
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x83
    {InstrCode::STY, Addr::ZERO_PAGE, 3},            // 0x84
    {InstrCode::STA, Addr::ZERO_PAGE, 3},            // 0x85
    {InstrCode::STX, Addr::ZERO_PAGE, 3},            // 0x86
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x87
    {InstrCode::DEY, Addr::NONE, 2},                 // 0x88
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x89
    {InstrCode::TXA, Addr::NONE, 2},                 // 0x8A
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x8B
    {InstrCode::STY, Addr::ABSOLUTE, 4},             // 0x8C
    {InstrCode::STA, Addr::ABSOLUTE, 4},             // 0x8D
    {InstrCode::STX, Addr::ABSOLUTE, 4},             // 0x8E
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x8F
    {InstrCode::BCC, Addr::RELATIVE, 2},             // 0x90
    {InstrCode::STA, Addr::INDIRECT_INDEXED_Y, 6},   // 0x91
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x92
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x93
    {InstrCode::STY, Addr::INDEXED_ZERO_PAGE_X, 4},  // 0x94
    {InstrCode::STA, Addr::INDEXED_ZERO_PAGE_X, 4},  // 0x95
    {InstrCode::STX, Addr::INDEXED_ZERO_PAGE_Y, 4},  // 0x96
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x97
    {InstrCode::TYA, Addr::NONE, 2},                 // 0x98
    {InstrCode::STA, Addr::INDEXED_ABSOLUTE_Y, 5},   // 0x99
    {InstrCode::TXS, Addr::NONE, 2},                 // 0x9A
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x9B
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x9C
    {InstrCode::STA, Addr::INDEXED_ABSOLUTE_X, 5},   // 0x9D
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x9E
    {InstrCode::NOP, Addr::NONE, 0},                 // 0x9F
    {InstrCode::LDY, Addr::IMMEDIATE, 2},            // 0xA0
    {InstrCode::LDA, Addr::INDEXED_INDIRECT_X, 6},   // 0xA1
    {InstrCode::LDX, Addr::IMMEDIATE, 2},            // 0xA2
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xA3
    {InstrCode::LDY, Addr::ZERO_PAGE, 3},            // 0xA4
    {InstrCode::LDA, Addr::ZERO_PAGE, 3},            // 0xA5
    {InstrCode::LDX, Addr::ZERO_PAGE, 3},            // 0xA6
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xA7
    {InstrCode::TAY, Addr::NONE, 2},                 // 0xA8
    {InstrCode::LDA, Addr::IMMEDIATE, 2},            // 0xA9
    {InstrCode::TAX, Addr::NONE, 2},                 // 0xAA
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xAB
    {InstrCode::LDY, Addr::ABSOLUTE, 4},             // 0xAC
    {InstrCode::LDA, Addr::ABSOLUTE, 4},             // 0xAD
    {InstrCode::LDX, Addr::ABSOLUTE, 4},             // 0xAE
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xAF
    {InstrCode::BCS, Addr::RELATIVE, 2},             // 0xB0
    {InstrCode::LDA, Addr::INDIRECT_INDEXED_Y, 5},   // 0xB1
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xB2
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xB3
    {InstrCode::LDY, Addr::INDEXED_ZERO_PAGE_X, 4},  // 0xB4
    {InstrCode::LDA, Addr::INDEXED_ZERO_PAGE_X, 4},  // 0xB5
    {InstrCode::LDX, Addr::INDEXED_ZERO_PAGE_Y, 4},  // 0xB6
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xB7
    {InstrCode::CLV, Addr::NONE, 2},                 // 0xB8
    {InstrCode::LDA, Addr::INDEXED_ABSOLUTE_Y, 4},   // 0xB9
    {InstrCode::TSX, Addr::NONE, 2},                 // 0xBA
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xBB
    {InstrCode::LDY, Addr::INDEXED_ABSOLUTE_X, 4},   // 0xBC
    {InstrCode::LDA, Addr::INDEXED_ABSOLUTE_X, 4},   // 0xBD
    {InstrCode::LDX, Addr::INDEXED_ABSOLUTE_Y, 4},   // 0xBE
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xBF
    {InstrCode::CPY, Addr::IMMEDIATE, 2},            // 0xC0
    {InstrCode::CMP, Addr::INDEXED_INDIRECT_X, 6},   // 0xC1
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xC2
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xC3
    {InstrCode::CPY, Addr::ZERO_PAGE, 3},            // 0xC4
    {InstrCode::CMP, Addr::ZERO_PAGE, 3},            // 0xC5
    {InstrCode::DEC, Addr::ZERO_PAGE, 5},            // 0xC6
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xC7
    {InstrCode::INY, Addr::NONE, 2},                 // 0xC8
    {InstrCode::CMP, Addr::IMMEDIATE, 2},            // 0xC9
    {InstrCode::DEX, Addr::NONE, 2},                 // 0xCA
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xCB
    {InstrCode::CPY, Addr::ABSOLUTE, 4},             // 0xCC
    {InstrCode::CMP, Addr::ABSOLUTE, 4},             // 0xCD
    {InstrCode::DEC, Addr::ABSOLUTE, 6},             // 0xCE
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xCF
    {InstrCode::BNE, Addr::RELATIVE, 2},             // 0xD0
    {InstrCode::CMP, Addr::INDIRECT_INDEXED_Y, 5},   // 0xD1
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xD2
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xD3
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xD4
    {InstrCode::CMP, Addr::INDEXED_ZERO_PAGE_X, 4},  // 0xD5
    {InstrCode::DEC, Addr::INDEXED_ZERO_PAGE_X, 6},  // 0xD6
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xD7
    {InstrCode::CLD, Addr::NONE, 2},                 // 0xD8
    {InstrCode::CMP, Addr::INDEXED_ABSOLUTE_Y, 4},   // 0xD9
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xDA
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xDB
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xDC
    {InstrCode::CMP, Addr::INDEXED_ABSOLUTE_X, 4},   // 0xDD
    {InstrCode::DEC, Addr::INDEXED_ABSOLUTE_X, 7},   // 0xDE
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xDF
    {InstrCode::CPX, Addr::IMMEDIATE, 2},            // 0xE0
    {InstrCode::SBC, Addr::INDEXED_INDIRECT_X, 6},   // 0xE1
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xE2
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xE3
    {InstrCode::CPX, Addr::ZERO_PAGE, 3},            // 0xE4
    {InstrCode::SBC, Addr::ZERO_PAGE, 3},            // 0xE5
    {InstrCode::INC, Addr::ZERO_PAGE, 5},            // 0xE6
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xE7
    {InstrCode::INX, Addr::NONE, 2},                 // 0xE8
    {InstrCode::SBC, Addr::IMMEDIATE, 2},            // 0xE9
    {InstrCode::NOP, Addr::NONE, 2},                 // 0xEA
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xEB
    {InstrCode::CPX, Addr::ABSOLUTE, 4},             // 0xEC
    {InstrCode::SBC, Addr::ABSOLUTE, 4},             // 0xED
    {InstrCode::INC, Addr::ABSOLUTE, 6},             // 0xEE
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xEF
    {InstrCode::BEQ, Addr::RELATIVE, 2},             // 0xF0
    {InstrCode::SBC, Addr::INDIRECT_INDEXED_Y, 5},   // 0xF1
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xF2
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xF3
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xF4
    {InstrCode::SBC, Addr::INDEXED_ZERO_PAGE_X, 4},  // 0xF5
    {InstrCode::INC, Addr::INDEXED_ZERO_PAGE_X, 6},  // 0xF6
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xF7
    {InstrCode::SED, Addr::NONE, 2},                 // 0xF8
    {InstrCode::SBC, Addr::INDEXED_ABSOLUTE_Y, 4},   // 0xF9
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xFA
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xFB
    {InstrCode::NOP, Addr::NONE, 0},                 // 0xFC
    {InstrCode::SBC, Addr::INDEXED_ABSOLUTE_X, 4},   // 0xFD
    {InstrCode::INC, Addr::INDEXED_ABSOLUTE_X, 7},   // 0xFE
    {InstrCode::NOP, Addr::NONE, 0}                  // 0xFF
};

int execute_instruction(uint8_t opcode, CpuState* cpu) {
    Instruction const& instruction = nes_instructions[opcode];
    switch (instruction.code) {
        case NOP:
        case ADC:
        case AND:
        case ASL:
        case ASL_ACCUM_ADDR:
        case EXEC_BRANCH:
        case BCC:
        case BCS:
        case BEQ:
        case BIT:
        case BMI:
        case BNE:
        case BPL:
        case BRK:
        case BVC:
        case BVS:
        case CLC:
        case CLD:
        case CLI:
        case CLV:
        case CMP:
        case CPX:
        case CPY:
        case DEC:
        case DEX:
        case DEY:
        case EOR:
        case INC:
        case INX:
        case INY:
        case JMP:
        case JSR:
        case LDA:
        case LDX:
        case LDY:
        case LSR:
        case LSR_ACCUM_ADDR:
        case ORA:
        case PHA:
        case PHP:
        case PLA:
        case PLP:
        case ROL:
        case ROL_ACCUM_ADDR:
        case ROR:
        case ROR_ACCUM_ADDR:
        case RTI:
        case RTS:
        case SBC:
        case SEC:
        case SED:
        case SEI:
        case STA:
        case STX:
        case STY:
        case TAX:
        case TAY:
        case TSX:
        case TXA:
        case TXS:
        case TYA:
            fmt::println("Unhandled opcode: {:02X} at PC: {:04X}",
                         opcode, cpu->pc - 1);

            break;
    }
    return 0;
}

int cpu_tick(CpuState* cpu) {
    uint8_t opcode = cpu->ram[cpu->pc++];
    return execute_instruction(opcode, cpu);
}

int main() {
    const std::string rom_path = "res/roms/nestest.nes";

    std::ifstream file(rom_path, std::ios::binary);
    if (!file.is_open()) {
        fmt::println("Failed to open ROM file: {}", rom_path);
        return 1;
    }

    iNesHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(iNesHeader));

    if (header.m_MagicConstant[0] != 'N' || header.m_MagicConstant[1] != 'E' ||
        header.m_MagicConstant[2] != 'S' || header.m_MagicConstant[3] != 0x1A) {
        fmt::println("Invalid iNES header magic bytes in {}", rom_path);
        return 1;
    }

    size_t prg_size = static_cast<size_t>(header.m_PrgRomChunks) * 16384;

    CpuState* cpu_state = new CpuState();

    file.read(reinterpret_cast<char*>(&cpu_state->ram[0x8000]), prg_size);
    if (prg_size == 16384) {
        std::memcpy(&cpu_state->ram[0xC000], &cpu_state->ram[0x8000], 16384);
    }

    cpu_state->pc = 0xC000;
    cpu_state->sp = 0xFD;
    cpu_state->status = 0x24;

    do {
        // TODO: Tick CPU
        int cycles = cpu_tick(cpu_state);
        if (cycles == 0) break;
    } while (cpu_state->ram[0x0002] != 0x00);

    delete cpu_state;
    return 0;
}
