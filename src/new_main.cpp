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

// Base cycle table for 6502 opcodes (0x00 to 0xFF)
constexpr uint8_t base_cycles[256] = {
    7, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 0, 4, 6, 0, // 0x00 - 0x0F
    2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0, // 0x10 - 0x1F
    6, 6, 0, 0, 3, 3, 5, 0, 4, 2, 2, 0, 4, 4, 6, 0, // 0x20 - 0x2F
    2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0, // 0x30 - 0x3F
    6, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 3, 4, 6, 0, // 0x40 - 0x4F
    2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0, // 0x50 - 0x5F
    6, 6, 0, 0, 0, 3, 5, 0, 4, 2, 2, 0, 5, 4, 6, 0, // 0x60 - 0x6F
    2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0, // 0x70 - 0x7F
    0, 6, 0, 0, 3, 3, 3, 0, 2, 0, 2, 0, 4, 4, 4, 0, // 0x80 - 0x8F
    2, 6, 0, 0, 4, 4, 4, 0, 2, 5, 2, 0, 0, 5, 0, 0, // 0x90 - 0x9F
    2, 6, 2, 0, 3, 3, 3, 0, 2, 2, 2, 0, 4, 4, 4, 0, // 0xA0 - 0xAF
    2, 5, 0, 0, 4, 4, 4, 0, 2, 4, 2, 0, 4, 4, 4, 0, // 0xB0 - 0xBF
    2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0, // 0xC0 - 0xCF
    2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0, // 0xD0 - 0xDF
    2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0, // 0xE0 - 0xEF
    2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0  // 0xF0 - 0xFF
};

// ============================================================================
// Flag Helper Functions
// ============================================================================
void set_flag(CpuState* cpu, CpuFlag flag, bool value) {
    if (value) {
        cpu->status |= flag;
    } else {
        cpu->status &= ~flag;
    }
}

bool get_flag(const CpuState* cpu, CpuFlag flag) {
    return (cpu->status & flag) != 0;
}

// ============================================================================
// Addressing Mode Helper Functions (Stubs for implementation)
// ============================================================================
uint8_t read_byte(CpuState* cpu, uint16_t addr) {
    return cpu->ram[addr];
}

uint16_t addr_immediate(CpuState* cpu) {
    // TODO: Implement Immediate addressing mode
    return 0;
}

uint16_t addr_zero_page(CpuState* cpu) {
    // TODO: Implement Zero Page addressing mode
    return 0;
}

uint16_t addr_zero_page_x(CpuState* cpu) {
    // TODO: Implement Indexed Zero Page X addressing mode
    return 0;
}

uint16_t addr_zero_page_y(CpuState* cpu) {
    // TODO: Implement Indexed Zero Page Y addressing mode
    return 0;
}

uint16_t addr_absolute(CpuState* cpu) {
    // TODO: Implement Absolute addressing mode
    return 0;
}

uint16_t addr_absolute_x(CpuState* cpu, int& cycles) {
    // TODO: Implement Indexed Absolute X addressing mode (may add +1 cycle on page boundary cross)
    return 0;
}

uint16_t addr_absolute_y(CpuState* cpu, int& cycles) {
    // TODO: Implement Indexed Absolute Y addressing mode (may add +1 cycle on page boundary cross)
    return 0;
}

uint16_t addr_absolute_indirect(CpuState* cpu) {
    // TODO: Implement Absolute Indirect addressing mode
    return 0;
}

uint16_t addr_indexed_indirect_x(CpuState* cpu) {
    // TODO: Implement Indexed Indirect X addressing mode
    return 0;
}

uint16_t addr_indirect_indexed_y(CpuState* cpu, int& cycles) {
    // TODO: Implement Indirect Indexed Y addressing mode (may add +1 cycle on page boundary cross)
    return 0;
}

int8_t addr_relative(CpuState* cpu) {
    // TODO: Implement Relative addressing mode
    return 0;
}

// ============================================================================
// Core Instruction Handlers (Stubs grouped by common topics)
// ============================================================================

// --- Load & Store Operations ---
void op_lda(CpuState* cpu, uint8_t value) {
    // TODO: Implement LDA (Load Accumulator)
}

void op_ldx(CpuState* cpu, uint8_t value) {
    // TODO: Implement LDX (Load X Register)
}

void op_ldy(CpuState* cpu, uint8_t value) {
    // TODO: Implement LDY (Load Y Register)
}

void op_sta(CpuState* cpu, uint16_t addr) {
    // TODO: Implement STA (Store Accumulator)
}

void op_stx(CpuState* cpu, uint16_t addr) {
    // TODO: Implement STX (Store X Register)
}

void op_sty(CpuState* cpu, uint16_t addr) {
    // TODO: Implement STY (Store Y Register)
}

// --- Register Transfer Operations ---
void op_tax(CpuState* cpu) {
    // TODO: Implement TAX (Transfer Accumulator to X)
}

void op_txa(CpuState* cpu) {
    // TODO: Implement TXA (Transfer X to Accumulator)
}

void op_tay(CpuState* cpu) {
    // TODO: Implement TAY (Transfer Accumulator to Y)
}

void op_tya(CpuState* cpu) {
    // TODO: Implement TYA (Transfer Y to Accumulator)
}

void op_tsx(CpuState* cpu) {
    // TODO: Implement TSX (Transfer Stack Pointer to X)
}

void op_txs(CpuState* cpu) {
    // TODO: Implement TXS (Transfer X to Stack Pointer)
}

// --- Arithmetic & Bitwise Logic Operations ---
void op_adc(CpuState* cpu, uint8_t value) {
    // TODO: Implement ADC (Add with Carry)
}

void op_sbc(CpuState* cpu, uint8_t value) {
    // TODO: Implement SBC (Subtract with Carry)
}

void op_and(CpuState* cpu, uint8_t value) {
    // TODO: Implement AND (Logical AND)
}

void op_ora(CpuState* cpu, uint8_t value) {
    // TODO: Implement ORA (Logical Inclusive OR)
}

void op_eor(CpuState* cpu, uint8_t value) {
    // TODO: Implement EOR (Logical Exclusive OR)
}

void op_bit(CpuState* cpu, uint8_t value) {
    // TODO: Implement BIT (Bit Test)
}

// --- Comparison Operations ---
void op_cmp(CpuState* cpu, uint8_t value) {
    // TODO: Implement CMP (Compare Accumulator)
}

void op_cpx(CpuState* cpu, uint8_t value) {
    // TODO: Implement CPX (Compare X Register)
}

void op_cpy(CpuState* cpu, uint8_t value) {
    // TODO: Implement CPY (Compare Y Register)
}

// --- Increment & Decrement Operations ---
void op_inc(CpuState* cpu, uint16_t addr) {
    // TODO: Implement INC (Increment Memory)
}

void op_dec(CpuState* cpu, uint16_t addr) {
    // TODO: Implement DEC (Decrement Memory)
}

void op_inx(CpuState* cpu) {
    // TODO: Implement INX (Increment X)
}

void op_dex(CpuState* cpu) {
    // TODO: Implement DEX (Decrement X)
}

void op_iny(CpuState* cpu) {
    // TODO: Implement INY (Increment Y)
}

void op_dey(CpuState* cpu) {
    // TODO: Implement DEY (Decrement Y)
}

// --- Shift & Rotate Operations ---
void op_asl_accum(CpuState* cpu) {
    // TODO: Implement ASL (Arithmetic Shift Left Accumulator)
}

void op_asl_mem(CpuState* cpu, uint16_t addr) {
    // TODO: Implement ASL (Arithmetic Shift Left Memory)
}

void op_lsr_accum(CpuState* cpu) {
    // TODO: Implement LSR (Logical Shift Right Accumulator)
}

void op_lsr_mem(CpuState* cpu, uint16_t addr) {
    // TODO: Implement LSR (Logical Shift Right Memory)
}

void op_rol_accum(CpuState* cpu) {
    // TODO: Implement ROL (Rotate Left Accumulator)
}

void op_rol_mem(CpuState* cpu, uint16_t addr) {
    // TODO: Implement ROL (Rotate Left Memory)
}

void op_ror_accum(CpuState* cpu) {
    // TODO: Implement ROR (Rotate Right Accumulator)
}

void op_ror_mem(CpuState* cpu, uint16_t addr) {
    // TODO: Implement ROR (Rotate Right Memory)
}

// --- Branching & Jumps ---
void op_branch(CpuState* cpu, bool condition, int8_t offset, int& cycles) {
    // TODO: Implement Branch logic (adds +1 cycle if branch taken, +1 extra if page crossed)
}

void op_jmp(CpuState* cpu, uint16_t addr) {
    // TODO: Implement JMP (Jump)
}

void op_jsr(CpuState* cpu, uint16_t addr) {
    // TODO: Implement JSR (Jump to Subroutine)
}

void op_rts(CpuState* cpu) {
    // TODO: Implement RTS (Return from Subroutine)
}

void op_rti(CpuState* cpu) {
    // TODO: Implement RTI (Return from Interrupt)
}

void op_brk(CpuState* cpu) {
    // TODO: Implement BRK (Force Interrupt)
}

// --- Stack Operations ---
void op_pha(CpuState* cpu) {
    // TODO: Implement PHA (Push Accumulator)
}

void op_php(CpuState* cpu) {
    // TODO: Implement PHP (Push Processor Status)
}

void op_pla(CpuState* cpu) {
    // TODO: Implement PLA (Pull Accumulator)
}

void op_plp(CpuState* cpu) {
    // TODO: Implement PLP (Pull Processor Status)
}

// ============================================================================
// Instruction Execution (Direct Switch on Opcode with Cycle Tracking)
// ============================================================================
int execute_instruction(CpuState* cpu, uint8_t opcode) {
    int cycles = base_cycles[opcode];

    switch (opcode) {
        // --- LDA Variants ---
        case 0xA9: op_lda(cpu, read_byte(cpu, addr_immediate(cpu))); break;
        case 0xA5: op_lda(cpu, read_byte(cpu, addr_zero_page(cpu))); break;
        case 0xB5: op_lda(cpu, read_byte(cpu, addr_zero_page_x(cpu))); break;
        case 0xAD: op_lda(cpu, read_byte(cpu, addr_absolute(cpu))); break;
        case 0xBD: op_lda(cpu, read_byte(cpu, addr_absolute_x(cpu, cycles))); break;
        case 0xB9: op_lda(cpu, read_byte(cpu, addr_absolute_y(cpu, cycles))); break;
        case 0xA1: op_lda(cpu, read_byte(cpu, addr_indexed_indirect_x(cpu))); break;
        case 0xB1: op_lda(cpu, read_byte(cpu, addr_indirect_indexed_y(cpu, cycles))); break;

        // --- LDX Variants ---
        case 0xA2: op_ldx(cpu, read_byte(cpu, addr_immediate(cpu))); break;
        case 0xA6: op_ldx(cpu, read_byte(cpu, addr_zero_page(cpu))); break;
        case 0xB6: op_ldx(cpu, read_byte(cpu, addr_zero_page_y(cpu))); break;
        case 0xAE: op_ldx(cpu, read_byte(cpu, addr_absolute(cpu))); break;
        case 0xBE: op_ldx(cpu, read_byte(cpu, addr_absolute_y(cpu, cycles))); break;

        // --- LDY Variants ---
        case 0xA0: op_ldy(cpu, read_byte(cpu, addr_immediate(cpu))); break;
        case 0xA4: op_ldy(cpu, read_byte(cpu, addr_zero_page(cpu))); break;
        case 0xB4: op_ldy(cpu, read_byte(cpu, addr_zero_page_x(cpu))); break;
        case 0xAC: op_ldy(cpu, read_byte(cpu, addr_absolute(cpu))); break;
        case 0xBC: op_ldy(cpu, read_byte(cpu, addr_absolute_x(cpu, cycles))); break;

        // --- STA Variants ---
        case 0x85: op_sta(cpu, addr_zero_page(cpu)); break;
        case 0x95: op_sta(cpu, addr_zero_page_x(cpu)); break;
        case 0x8D: op_sta(cpu, addr_absolute(cpu)); break;
        case 0x9D: op_sta(cpu, addr_absolute_x(cpu, cycles)); break;
        case 0x99: op_sta(cpu, addr_absolute_y(cpu, cycles)); break;
        case 0x81: op_sta(cpu, addr_indexed_indirect_x(cpu)); break;
        case 0x91: op_sta(cpu, addr_indirect_indexed_y(cpu, cycles)); break;

        // --- STX Variants ---
        case 0x86: op_stx(cpu, addr_zero_page(cpu)); break;
        case 0x96: op_stx(cpu, addr_zero_page_y(cpu)); break;
        case 0x8E: op_stx(cpu, addr_absolute(cpu)); break;

        // --- STY Variants ---
        case 0x84: op_sty(cpu, addr_zero_page(cpu)); break;
        case 0x94: op_sty(cpu, addr_zero_page_x(cpu)); break;
        case 0x8C: op_sty(cpu, addr_absolute(cpu)); break;

        // --- Register Transfers ---
        case 0xAA: op_tax(cpu); break;
        case 0x8A: op_txa(cpu); break;
        case 0xA8: op_tay(cpu); break;
        case 0x98: op_tya(cpu); break;
        case 0xBA: op_tsx(cpu); break;
        case 0x9A: op_txs(cpu); break;

        // --- ADC Variants ---
        case 0x69: op_adc(cpu, read_byte(cpu, addr_immediate(cpu))); break;
        case 0x65: op_adc(cpu, read_byte(cpu, addr_zero_page(cpu))); break;
        case 0x75: op_adc(cpu, read_byte(cpu, addr_zero_page_x(cpu))); break;
        case 0x6D: op_adc(cpu, read_byte(cpu, addr_absolute(cpu))); break;
        case 0x7D: op_adc(cpu, read_byte(cpu, addr_absolute_x(cpu, cycles))); break;
        case 0x79: op_adc(cpu, read_byte(cpu, addr_absolute_y(cpu, cycles))); break;
        case 0x61: op_adc(cpu, read_byte(cpu, addr_indexed_indirect_x(cpu))); break;
        case 0x71: op_adc(cpu, read_byte(cpu, addr_indirect_indexed_y(cpu, cycles))); break;

        // --- SBC Variants ---
        case 0xE9: op_sbc(cpu, read_byte(cpu, addr_immediate(cpu))); break;
        case 0xE5: op_sbc(cpu, read_byte(cpu, addr_zero_page(cpu))); break;
        case 0xF5: op_sbc(cpu, read_byte(cpu, addr_zero_page_x(cpu))); break;
        case 0xED: op_sbc(cpu, read_byte(cpu, addr_absolute(cpu))); break;
        case 0xFD: op_sbc(cpu, read_byte(cpu, addr_absolute_x(cpu, cycles))); break;
        case 0xF9: op_sbc(cpu, read_byte(cpu, addr_absolute_y(cpu, cycles))); break;
        case 0xE1: op_sbc(cpu, read_byte(cpu, addr_indexed_indirect_x(cpu))); break;
        case 0xF1: op_sbc(cpu, read_byte(cpu, addr_indirect_indexed_y(cpu, cycles))); break;

        // --- AND Variants ---
        case 0x29: op_and(cpu, read_byte(cpu, addr_immediate(cpu))); break;
        case 0x25: op_and(cpu, read_byte(cpu, addr_zero_page(cpu))); break;
        case 0x35: op_and(cpu, read_byte(cpu, addr_zero_page_x(cpu))); break;
        case 0x2D: op_and(cpu, read_byte(cpu, addr_absolute(cpu))); break;
        case 0x3D: op_and(cpu, read_byte(cpu, addr_absolute_x(cpu, cycles))); break;
        case 0x39: op_and(cpu, read_byte(cpu, addr_absolute_y(cpu, cycles))); break;
        case 0x21: op_and(cpu, read_byte(cpu, addr_indexed_indirect_x(cpu))); break;
        case 0x31: op_and(cpu, read_byte(cpu, addr_indirect_indexed_y(cpu, cycles))); break;

        // --- ORA Variants ---
        case 0x09: op_ora(cpu, read_byte(cpu, addr_immediate(cpu))); break;
        case 0x05: op_ora(cpu, read_byte(cpu, addr_zero_page(cpu))); break;
        case 0x15: op_ora(cpu, read_byte(cpu, addr_zero_page_x(cpu))); break;
        case 0x0D: op_ora(cpu, read_byte(cpu, addr_absolute(cpu))); break;
        case 0x1D: op_ora(cpu, read_byte(cpu, addr_absolute_x(cpu, cycles))); break;
        case 0x19: op_ora(cpu, read_byte(cpu, addr_absolute_y(cpu, cycles))); break;
        case 0x01: op_ora(cpu, read_byte(cpu, addr_indexed_indirect_x(cpu))); break;
        case 0x11: op_ora(cpu, read_byte(cpu, addr_indirect_indexed_y(cpu, cycles))); break;

        // --- EOR Variants ---
        case 0x49: op_eor(cpu, read_byte(cpu, addr_immediate(cpu))); break;
        case 0x45: op_eor(cpu, read_byte(cpu, addr_zero_page(cpu))); break;
        case 0x55: op_eor(cpu, read_byte(cpu, addr_zero_page_x(cpu))); break;
        case 0x4D: op_eor(cpu, read_byte(cpu, addr_absolute(cpu))); break;
        case 0x5D: op_eor(cpu, read_byte(cpu, addr_absolute_x(cpu, cycles))); break;
        case 0x59: op_eor(cpu, read_byte(cpu, addr_absolute_y(cpu, cycles))); break;
        case 0x41: op_eor(cpu, read_byte(cpu, addr_indexed_indirect_x(cpu))); break;
        case 0x51: op_eor(cpu, read_byte(cpu, addr_indirect_indexed_y(cpu, cycles))); break;

        // --- BIT Variants ---
        case 0x24: op_bit(cpu, read_byte(cpu, addr_zero_page(cpu))); break;
        case 0x2C: op_bit(cpu, read_byte(cpu, addr_absolute(cpu))); break;

        // --- CMP Variants ---
        case 0xC9: op_cmp(cpu, read_byte(cpu, addr_immediate(cpu))); break;
        case 0xC5: op_cmp(cpu, read_byte(cpu, addr_zero_page(cpu))); break;
        case 0xD5: op_cmp(cpu, read_byte(cpu, addr_zero_page_x(cpu))); break;
        case 0xCD: op_cmp(cpu, read_byte(cpu, addr_absolute(cpu))); break;
        case 0xDD: op_cmp(cpu, read_byte(cpu, addr_absolute_x(cpu, cycles))); break;
        case 0xD9: op_cmp(cpu, read_byte(cpu, addr_absolute_y(cpu, cycles))); break;
        case 0xC1: op_cmp(cpu, read_byte(cpu, addr_indexed_indirect_x(cpu))); break;
        case 0xD1: op_cmp(cpu, read_byte(cpu, addr_indirect_indexed_y(cpu, cycles))); break;

        // --- CPX Variants ---
        case 0xE0: op_cpx(cpu, read_byte(cpu, addr_immediate(cpu))); break;
        case 0xE4: op_cpx(cpu, read_byte(cpu, addr_zero_page(cpu))); break;
        case 0xEC: op_cpx(cpu, read_byte(cpu, addr_absolute(cpu))); break;

        // --- CPY Variants ---
        case 0xC0: op_cpy(cpu, read_byte(cpu, addr_immediate(cpu))); break;
        case 0xC4: op_cpy(cpu, read_byte(cpu, addr_zero_page(cpu))); break;
        case 0xCC: op_cpy(cpu, read_byte(cpu, addr_absolute(cpu))); break;

        // --- Increment & Decrement ---
        case 0xE6: op_inc(cpu, addr_zero_page(cpu)); break;
        case 0xF6: op_inc(cpu, addr_zero_page_x(cpu)); break;
        case 0xEE: op_inc(cpu, addr_absolute(cpu)); break;
        case 0xFE: op_inc(cpu, addr_absolute_x(cpu, cycles)); break;

        case 0xC6: op_dec(cpu, addr_zero_page(cpu)); break;
        case 0xD6: op_dec(cpu, addr_zero_page_x(cpu)); break;
        case 0xCE: op_dec(cpu, addr_absolute(cpu)); break;
        case 0xDE: op_dec(cpu, addr_absolute_x(cpu, cycles)); break;

        case 0xE8: op_inx(cpu); break;
        case 0xCA: op_dex(cpu); break;
        case 0xC8: op_iny(cpu); break;
        case 0x88: op_dey(cpu); break;

        // --- Shift & Rotate ---
        case 0x0A: op_asl_accum(cpu); break;
        case 0x06: op_asl_mem(cpu, addr_zero_page(cpu)); break;
        case 0x16: op_asl_mem(cpu, addr_zero_page_x(cpu)); break;
        case 0x0E: op_asl_mem(cpu, addr_absolute(cpu)); break;
        case 0x1E: op_asl_mem(cpu, addr_absolute_x(cpu, cycles)); break;

        case 0x4A: op_lsr_accum(cpu); break;
        case 0x46: op_lsr_mem(cpu, addr_zero_page(cpu)); break;
        case 0x56: op_lsr_mem(cpu, addr_zero_page_x(cpu)); break;
        case 0x4E: op_lsr_mem(cpu, addr_absolute(cpu)); break;
        case 0x5E: op_lsr_mem(cpu, addr_absolute_x(cpu, cycles)); break;

        case 0x2A: op_rol_accum(cpu); break;
        case 0x26: op_rol_mem(cpu, addr_zero_page(cpu)); break;
        case 0x36: op_rol_mem(cpu, addr_zero_page_x(cpu)); break;
        case 0x2E: op_rol_mem(cpu, addr_absolute(cpu)); break;
        case 0x3E: op_rol_mem(cpu, addr_absolute_x(cpu, cycles)); break;

        case 0x6A: op_ror_accum(cpu); break;
        case 0x66: op_ror_mem(cpu, addr_zero_page(cpu)); break;
        case 0x76: op_ror_mem(cpu, addr_zero_page_x(cpu)); break;
        case 0x6E: op_ror_mem(cpu, addr_absolute(cpu)); break;
        case 0x7E: op_ror_mem(cpu, addr_absolute_x(cpu, cycles)); break;

        // --- Branching ---
        case 0x90: op_branch(cpu, !get_flag(cpu, CpuFlag::C), addr_relative(cpu), cycles); break; // BCC
        case 0xB0: op_branch(cpu, get_flag(cpu, CpuFlag::C), addr_relative(cpu), cycles); break;  // BCS
        case 0xF0: op_branch(cpu, get_flag(cpu, CpuFlag::Z), addr_relative(cpu), cycles); break;  // BEQ
        case 0xD0: op_branch(cpu, !get_flag(cpu, CpuFlag::Z), addr_relative(cpu), cycles); break; // BNE
        case 0x30: op_branch(cpu, get_flag(cpu, CpuFlag::N), addr_relative(cpu), cycles); break;  // BMI
        case 0x10: op_branch(cpu, !get_flag(cpu, CpuFlag::N), addr_relative(cpu), cycles); break; // BPL
        case 0x50: op_branch(cpu, !get_flag(cpu, CpuFlag::V), addr_relative(cpu), cycles); break; // BVC
        case 0x70: op_branch(cpu, get_flag(cpu, CpuFlag::V), addr_relative(cpu), cycles); break;  // BVS

        // --- Jumps & Returns ---
        case 0x4C: op_jmp(cpu, addr_absolute(cpu)); break;
        case 0x6C: op_jmp(cpu, addr_absolute_indirect(cpu)); break;
        case 0x20: op_jsr(cpu, addr_absolute(cpu)); break;
        case 0x60: op_rts(cpu); break;
        case 0x40: op_rti(cpu); break;
        case 0x00: op_brk(cpu); break;

        // --- Flag Operations ---
        case 0x18: set_flag(cpu, CpuFlag::C, false); break; // CLC
        case 0x38: set_flag(cpu, CpuFlag::C, true); break;  // SEC
        case 0x58: set_flag(cpu, CpuFlag::I, false); break; // CLI
        case 0x78: set_flag(cpu, CpuFlag::I, true); break;  // SEI
        case 0xD8: set_flag(cpu, CpuFlag::D, false); break; // CLD
        case 0xF8: set_flag(cpu, CpuFlag::D, true); break;  // SED
        case 0xB8: set_flag(cpu, CpuFlag::V, false); break; // CLV

        // --- Stack Operations ---
        case 0x48: op_pha(cpu); break;
        case 0x08: op_php(cpu); break;
        case 0x68: op_pla(cpu); break;
        case 0x28: op_plp(cpu); break;

        // --- NOP ---
        case 0xEA: break;

        default:
            fmt::println("Unhandled opcode: {:02X} at PC: {:04X}", opcode, cpu->pc - 1);
            break;
    }
    return cycles;
}

int cpu_tick(CpuState* cpu) {
    uint8_t opcode = cpu->ram[cpu->pc++];
    return execute_instruction(cpu, opcode);
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
        int cycles = cpu_tick(cpu_state);
        if (cycles == 0) break;
    } while (cpu_state->ram[0x0002] == 0x00);

    uint8_t error_code = cpu_state->ram[0x0002];
    uint8_t unofficial_code = cpu_state->ram[0x0003];
    fmt::println("Execution stopped at PC: 0x{:04X}", cpu_state->pc);
    fmt::println("NESTEST official error code ($02): 0x{:02X}", error_code);
    fmt::println("NESTEST unofficial error code ($03): 0x{:02X}", unofficial_code);

    if (error_code == 0x00) {
        fmt::println("NESTEST official instructions PASSED!");
    } else {
        fmt::println("NESTEST official instructions FAILED with error: 0x{:02X}", error_code);
    }

    delete cpu_state;
    return 0;
}
