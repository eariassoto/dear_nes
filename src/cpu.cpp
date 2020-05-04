// Copyright (c) 2020 Emmanuel Arias
#include "include/cpu.h"

#include <fmt/core.h>
#include <iostream>

#include "include/nes.h"

namespace cpuemulator {

Cpu::Cpu() : m_CpuWidget{this} {}

void Cpu::RegisterNesPointer(Nes* nes) {
    m_NesPtr = nes;
    RegisterAllInstructionSet();
}

void Cpu::RenderWidgets() { m_CpuWidget.Render(); }

void Cpu::Clock() {
    if (m_Cycles == 0) {
        m_OpCode = Read(m_ProgramCounter);

        SetFlag(FLAGS::U, 1);

        m_ProgramCounter++;

        // todo handle illegal ops
        std::optional<Instruction>& instr = m_InstrTable[m_OpCode];
        if (!instr.has_value()) {
            instr = m_InstrTable[0xEA];
        }

        m_Cycles = instr->m_Cycles;

        m_AddressingMode = instr->m_AddressingMode;
        uint8_t additionalCycle1 = ExecuteAddressing();

        uint8_t additionalCycle2 = instr->m_FuncOperate();

        if (additionalCycle1 & additionalCycle2) {
            ++m_Cycles;
        }

        SetFlag(U, true);
    }

    m_Cycles--;
}

void Cpu::Reset() {
    uint16_t addressToReadPC = 0xFFFC;
    uint16_t lo = Read(addressToReadPC);
    uint16_t hi = Read(addressToReadPC + 1);

    m_ProgramCounter = (hi << 8) | lo;

    m_RegisterA = 0;
    m_RegisterX = 0;
    m_RegisterY = 0;
    m_StackPointer = 0xFD;
    m_StatusRegister = 0x00 | FLAGS::U;

    m_AddressRelative = 0x0000;
    m_AddressAbsolute = 0x0000;
    m_ValueFetched = 0x00;

    m_Cycles = 8;
}

void Cpu::InterruptRequest() {
    if (GetFlag(I) == 1) {
        return;
    }

    Write(0x0100 + m_StackPointer, (m_ProgramCounter >> 8) & 0x00FF);
    m_StackPointer--;
    Write(0x0100 + m_StackPointer, m_ProgramCounter & 0x00FF);
    m_StackPointer--;

    SetFlag(B, 0);
    SetFlag(U, 1);
    SetFlag(I, 1);
    Write(0x0100 + m_StackPointer, m_StatusRegister);
    m_StackPointer--;

    uint16_t addresToReadPC = 0xFFFE;
    uint16_t lo = Read(addresToReadPC);
    uint16_t hi = Read(addresToReadPC + 1);
    m_ProgramCounter = (hi << 8) | lo;

    m_Cycles = 7;
}

void Cpu::NonMaskableInterrupt() {
    Write(0x0100 + m_StackPointer, (m_ProgramCounter >> 8) & 0x00FF);
    m_StackPointer--;
    Write(0x0100 + m_StackPointer, m_ProgramCounter & 0x00FF);
    m_StackPointer--;

    SetFlag(B, 0);
    SetFlag(U, 1);
    SetFlag(I, 1);
    Write(0x0100 + m_StackPointer, m_StatusRegister);
    m_StackPointer--;

    uint16_t addresToReadPC = 0xFFFA;
    uint16_t lo = Read(addresToReadPC + 0);
    uint16_t hi = Read(addresToReadPC + 1);
    m_ProgramCounter = (hi << 8) | lo;

    m_Cycles = 8;
}

bool Cpu::IsCurrentInstructionComplete() const { return m_Cycles == 0; }

uint8_t Cpu::GetFlag(FLAGS flag) const {
    if ((m_StatusRegister & flag) == 0x00) {
        return 0;
    }
    return 1;
}

void Cpu::SetFlag(FLAGS flag, bool value) {
    if (value) {
        m_StatusRegister |= flag;
    } else {
        m_StatusRegister &= ~flag;
    }
}

uint8_t Cpu::Read(uint16_t address) { return m_NesPtr->CpuRead(address); }

void Cpu::Write(uint16_t address, uint8_t data) {
    m_NesPtr->CpuWrite(address, data);
}

#pragma region ADDRESSIN_MODES

uint8_t Cpu::ExecuteAddressing() {
    switch (m_AddressingMode) {
        case ACCUMMULATOR_ADDRESSING:
            return AccumulatorAddressing();
        case IMPLIED_ADDRESSING:
            return ImpliedAddressing();
        case IMMEDIATE_ADDRESSING:
            return ImmediateAddressing();
        case ZERO_PAGE_ADDRESSING:
            return ZeroPageAddressing();
        case INDEXED_ZERO_PAGE_ADDRESSING_X:
            return IndexedZeroPageAddressingX();
        case INDEXED_ZERO_PAGE_ADDRESSING_Y:
            return IndexedZeroPageAddressingY();
        case ABSOLUTE_ADDRESSING:
            return AbsoluteAddressing();
        case INDEXED_ABSOLUTE_ADDRESSING_X:
            return IndexedAbsoluteAddressingX();
        case INDEXED_ABSOLUTE_ADDRESSING_Y:
            return IndexedAbsoluteAddressingY();
        case ABSOLUTE_INDIRECT_ADDRESSING:
            return AbsoluteIndirectAddressing();
        case INDEXED_INDIRECT_ADDRESSING_X:
            return IndexedIndirectAddressingX();
        case INDIRECT_INDEXED_ADDRESSING_Y:
            return IndirectIndexedAddressingY();
        case RELATIVE_ADDRESSING:
            return RelativeAddressing();
    }
    return 0;
}

uint8_t Cpu::AccumulatorAddressing() {
    m_AddressingMode = AddressingMode::ACCUMMULATOR_ADDRESSING;
    m_ValueFetched = m_RegisterA;
    return 0;
}

uint8_t Cpu::ImpliedAddressing() {
    m_AddressingMode = AddressingMode::IMPLIED_ADDRESSING;
    return 0;
}

uint8_t Cpu::ImmediateAddressing() {
    m_AddressingMode = AddressingMode::IMMEDIATE_ADDRESSING;
    m_AddressAbsolute = m_ProgramCounter++;
    return 0;
}

uint8_t Cpu::ZeroPageAddressing() {
    m_AddressingMode = AddressingMode::ZERO_PAGE_ADDRESSING;
    m_AddressAbsolute = Read(m_ProgramCounter);
    m_ProgramCounter++;

    m_AddressAbsolute &= 0x00FF;
    return 0;
}

uint8_t Cpu::IndexedZeroPageAddressingX() {
    m_AddressingMode = AddressingMode::INDEXED_ZERO_PAGE_ADDRESSING_X;
    m_AddressAbsolute = Read(m_ProgramCounter) + m_RegisterX;
    m_ProgramCounter++;

    m_AddressAbsolute &= 0x00FF;
    return 0;
}

uint8_t Cpu::IndexedZeroPageAddressingY() {
    m_AddressingMode = AddressingMode::INDEXED_ZERO_PAGE_ADDRESSING_Y;
    m_AddressAbsolute = Read(m_ProgramCounter) + m_RegisterY;
    m_ProgramCounter++;

    m_AddressAbsolute &= 0x00FF;
    return 0;
}

uint8_t Cpu::AbsoluteAddressing() {
    m_AddressingMode = AddressingMode::ABSOLUTE_ADDRESSING;
    uint16_t lowNibble = Read(m_ProgramCounter);
    m_ProgramCounter++;

    uint16_t highNibble = Read(m_ProgramCounter);
    m_ProgramCounter++;

    m_AddressAbsolute = (highNibble << 8) | lowNibble;

    return 0;
}

uint8_t Cpu::IndexedAbsoluteAddressingX() {
    m_AddressingMode = AddressingMode::INDEXED_ABSOLUTE_ADDRESSING_X;
    uint16_t lowNibble = Read(m_ProgramCounter);
    m_ProgramCounter++;

    uint16_t highNibble = Read(m_ProgramCounter);
    m_ProgramCounter++;

    m_AddressAbsolute = (highNibble << 8) | lowNibble;
    m_AddressAbsolute += m_RegisterX;

    // todo make constexpr function
    if ((m_AddressAbsolute & 0xFF00) != (highNibble << 8)) {
        return 1;
    } else {
        return 0;
    }
}

uint8_t Cpu::IndexedAbsoluteAddressingY() {
    m_AddressingMode = AddressingMode::INDEXED_ABSOLUTE_ADDRESSING_Y;
    uint16_t lowNibble = Read(m_ProgramCounter);
    m_ProgramCounter++;

    uint16_t highNibble = Read(m_ProgramCounter);
    m_ProgramCounter++;

    m_AddressAbsolute = (highNibble << 8) | lowNibble;
    m_AddressAbsolute += m_RegisterY;

    // todo make constexpr function
    if ((m_AddressAbsolute & 0xFF00) != (highNibble << 8)) {
        return 1;
    } else {
        return 0;
    }
}

uint8_t Cpu::AbsoluteIndirectAddressing() {
    m_AddressingMode = AddressingMode::ABSOLUTE_INDIRECT_ADDRESSING;
    uint16_t lowNibble = Read(m_ProgramCounter);
    m_ProgramCounter++;

    uint16_t highNibble = Read(m_ProgramCounter);
    m_ProgramCounter++;

    uint16_t pointer = (highNibble << 8) | lowNibble;

    // Simulate page boundary hardware bug
    if (lowNibble == 0x00FF) {
        m_AddressAbsolute = (Read(pointer & 0xFF00) << 8) | Read(pointer);
    } else {
        m_AddressAbsolute = (Read(pointer + 1) << 8) | Read(pointer);
    }
    return 0;
}

uint8_t Cpu::IndexedIndirectAddressingX() {
    m_AddressingMode = AddressingMode::INDEXED_INDIRECT_ADDRESSING_X;
    uint16_t t = Read(m_ProgramCounter);
    m_ProgramCounter++;

    uint16_t registerXValue = static_cast<uint16_t>(m_RegisterX);

    uint16_t lowNibbleAddress = (t + registerXValue) & 0x00FF;
    uint16_t lowNibble = Read(lowNibbleAddress);

    uint16_t highNibbleAddress = (t + registerXValue + 1) & 0x00FF;
    uint16_t highNibble = Read(highNibbleAddress);

    m_AddressAbsolute = (highNibble << 8) | lowNibble;

    return 0;
}

uint8_t Cpu::IndirectIndexedAddressingY() {
    m_AddressingMode = AddressingMode::INDIRECT_INDEXED_ADDRESSING_Y;
    uint16_t t = Read(m_ProgramCounter);
    m_ProgramCounter++;

    uint16_t lowNibble = Read(t & 0x00FF);
    uint16_t highNibble = Read((t + 1) & 0x00FF);

    m_AddressAbsolute = (highNibble << 8) | lowNibble;
    m_AddressAbsolute += m_RegisterY;

    if ((m_AddressAbsolute & 0xFF00) != (highNibble << 8)) {
        return 1;
    } else {
        return 0;
    }
}

uint8_t Cpu::RelativeAddressing() {
    m_AddressingMode = AddressingMode::RELATIVE_ADDRESSING;
    m_AddressRelative = Read(m_ProgramCounter);
    m_ProgramCounter++;

    if (m_AddressRelative & 0x80)  // if unsigned
    {
        m_AddressRelative |= 0xFF00;
    }

    return 0;
}

uint8_t Cpu::Fetch() {
    if (m_AddressingMode != AddressingMode::ACCUMMULATOR_ADDRESSING) {
        m_ValueFetched = Read(m_AddressAbsolute);
    }
    return m_ValueFetched;
}

#pragma endregion ADDRESSING_MODES

#pragma region INSTRUCTIONS

uint8_t Cpu::Instruction_ADC() {
    Fetch();

    uint16_t castedFetched = static_cast<uint16_t>(m_ValueFetched);
    uint16_t castedCarry = static_cast<uint16_t>(GetFlag(FLAGS::C));
    uint16_t castedAccum = static_cast<uint16_t>(m_RegisterA);

    uint16_t temp = castedAccum + castedFetched + castedCarry;
    SetFlag(FLAGS::C, temp > 255);
    SetFlag(FLAGS::Z, (temp & 0x00FF) == 0);
    SetFlag(FLAGS::N, temp & 0x80);
    SetFlag(V,
            (~(castedAccum ^ castedFetched) & (castedAccum ^ temp)) & 0x0080);

    m_RegisterA = temp & 0x00FF;

    return 1;
}

uint8_t Cpu::Instruction_AND() {
    Fetch();

    m_RegisterA &= m_ValueFetched;
    SetFlag(FLAGS::Z, m_RegisterA == 0x00);
    SetFlag(FLAGS::N, m_RegisterA & 0x80);

    return 1;
}

uint8_t Cpu::Instruction_ASL() {
    Fetch();

    uint16_t temp = static_cast<uint16_t>(m_ValueFetched) << 1;
    SetFlag(FLAGS::C, (temp & 0xFF00) > 0);
    SetFlag(FLAGS::Z, (temp & 0x00FF) == 0);
    SetFlag(FLAGS::N, temp & 0x80);

    if (m_AddressingMode == AddressingMode::ACCUMMULATOR_ADDRESSING) {
        m_RegisterA = temp & 0x00FF;
    } else {
        Write(m_AddressAbsolute, temp & 0x00FF);
    }
    return 0;
}

void Cpu::Instruction_ExecuteBranch() {
    m_Cycles++;
    m_AddressAbsolute = m_ProgramCounter + m_AddressRelative;

    if ((m_AddressAbsolute & 0xFF00) != (m_ProgramCounter & 0xFF00)) {
        m_Cycles++;
    }
    m_ProgramCounter = m_AddressAbsolute;
}

uint8_t Cpu::Instruction_BCC() {
    if (GetFlag(FLAGS::C) == 0) {
        Instruction_ExecuteBranch();
    }
    return 0;
}

uint8_t Cpu::Instruction_BCS() {
    if (GetFlag(FLAGS::C) == 1) {
        Instruction_ExecuteBranch();
    }
    return 0;
}

uint8_t Cpu::Instruction_BEQ() {
    if (GetFlag(FLAGS::Z) == 1) {
        Instruction_ExecuteBranch();
    }
    return 0;
}

uint8_t Cpu::Instruction_BIT() {
    Fetch();

    uint16_t temp = m_RegisterA & m_ValueFetched;
    SetFlag(Z, (temp & 0x00FF) == 0x00);
    SetFlag(N, m_ValueFetched & (1 << 7));
    SetFlag(V, m_ValueFetched & (1 << 6));
    return 0;
}

uint8_t Cpu::Instruction_BMI() {
    if (GetFlag(FLAGS::N) == 1) {
        Instruction_ExecuteBranch();
    }
    return 0;
}

uint8_t Cpu::Instruction_BNE() {
    if (GetFlag(FLAGS::Z) == 0) {
        Instruction_ExecuteBranch();
    }
    return 0;
}

uint8_t Cpu::Instruction_BPL() {
    if (GetFlag(FLAGS::N) == 0) {
        Instruction_ExecuteBranch();
    }
    return 0;
}

uint8_t Cpu::Instruction_BRK() {
    m_ProgramCounter++;

    SetFlag(I, 1);
    Write(0x0100 + m_StackPointer, (m_ProgramCounter >> 8) & 0x00FF);
    m_StackPointer--;
    Write(0x0100 + m_StackPointer, m_ProgramCounter & 0x00FF);
    m_StackPointer--;

    SetFlag(FLAGS::B, 1);
    Write(0x0100 + m_StackPointer, m_StatusRegister);
    m_StackPointer--;
    SetFlag(B, 0);

    m_ProgramCounter = static_cast<uint16_t>(Read(0xFFFE)) |
                       (static_cast<uint16_t>(Read(0xFFFF)) << 8);
    return 0;
}

uint8_t Cpu::Instruction_BVC() {
    if (GetFlag(FLAGS::V) == 0) {
        Instruction_ExecuteBranch();
    }
    return 0;
}

uint8_t Cpu::Instruction_BVS() {
    if (GetFlag(FLAGS::V) == 1) {
        Instruction_ExecuteBranch();
    }
    return 0;
}

uint8_t Cpu::Instruction_CLC() {
    SetFlag(FLAGS::C, false);
    return 0;
}

uint8_t Cpu::Instruction_CLD() {
    SetFlag(FLAGS::D, false);
    return 0;
}

uint8_t Cpu::Instruction_CLI() {
    SetFlag(FLAGS::I, false);
    return 0;
}

uint8_t Cpu::Instruction_CLV() {
    SetFlag(FLAGS::V, false);
    return 0;
}

uint8_t Cpu::Instruction_CMP() {
    Fetch();
    uint16_t temp = static_cast<uint16_t>(m_RegisterA) -
                    static_cast<uint16_t>(m_ValueFetched);
    SetFlag(C, m_RegisterA >= m_ValueFetched);
    SetFlag(Z, (temp & 0x00FF) == 0x0000);
    SetFlag(N, temp & 0x0080);
    return 1;
}

uint8_t Cpu::Instruction_CPX() {
    Fetch();
    uint16_t temp = static_cast<uint16_t>(m_RegisterX) -
                    static_cast<uint16_t>(m_ValueFetched);
    SetFlag(C, m_RegisterX >= m_ValueFetched);
    SetFlag(Z, (temp & 0x00FF) == 0x0000);
    SetFlag(N, temp & 0x0080);
    return 0;
}

uint8_t Cpu::Instruction_CPY() {
    Fetch();
    uint16_t temp = static_cast<uint16_t>(m_RegisterY) -
                    static_cast<uint16_t>(m_ValueFetched);
    SetFlag(C, m_RegisterY >= m_ValueFetched);
    SetFlag(Z, (temp & 0x00FF) == 0x0000);
    SetFlag(N, temp & 0x0080);
    return 0;
}

uint8_t Cpu::Instruction_DEC() {
    Fetch();
    uint16_t temp = m_ValueFetched - 1;
    Write(m_AddressAbsolute, temp & 0x00FF);
    SetFlag(Z, (temp & 0x00FF) == 0x0000);
    SetFlag(N, temp & 0x0080);
    return 0;
}

uint8_t Cpu::Instruction_DEX() {
    m_RegisterX--;
    SetFlag(Z, m_RegisterX == 0x00);
    SetFlag(N, m_RegisterX & 0x80);
    return 0;
}

uint8_t Cpu::Instruction_DEY() {
    m_RegisterY--;
    SetFlag(Z, m_RegisterY == 0x00);
    SetFlag(N, m_RegisterY & 0x80);
    return 0;
}

uint8_t Cpu::Instruction_EOR() {
    Fetch();
    m_RegisterA = m_RegisterA ^ m_ValueFetched;
    SetFlag(Z, m_RegisterA == 0x00);
    SetFlag(N, m_RegisterA & 0x80);
    return 1;
}

uint8_t Cpu::Instruction_INC() {
    Fetch();
    uint16_t temp = m_ValueFetched + 1;
    Write(m_AddressAbsolute, temp & 0x00FF);
    SetFlag(Z, (temp & 0x00FF) == 0x0000);
    SetFlag(N, temp & 0x0080);
    return 0;
}

uint8_t Cpu::Instruction_INX() {
    m_RegisterX++;
    SetFlag(Z, m_RegisterX == 0x00);
    SetFlag(N, m_RegisterX & 0x80);
    return 0;
}

uint8_t Cpu::Instruction_INY() {
    m_RegisterY++;
    SetFlag(Z, m_RegisterY == 0x00);
    SetFlag(N, m_RegisterY & 0x80);
    return 0;
}

uint8_t Cpu::Instruction_JMP() {
    m_ProgramCounter = m_AddressAbsolute;
    return 0;
}

uint8_t Cpu::Instruction_JSR() {
    m_ProgramCounter--;

    Write(0x0100 + m_StackPointer, (m_ProgramCounter >> 8) & 0x00FF);
    m_StackPointer--;
    Write(0x0100 + m_StackPointer, m_ProgramCounter & 0x00FF);
    m_StackPointer--;

    m_ProgramCounter = m_AddressAbsolute;
    return 0;
}

uint8_t Cpu::Instruction_LDA() {
    Fetch();
    m_RegisterA = m_ValueFetched;
    SetFlag(Z, m_RegisterA == 0x00);
    SetFlag(N, m_RegisterA & 0x80);
    return 1;
}

uint8_t Cpu::Instruction_LDX() {
    Fetch();
    m_RegisterX = m_ValueFetched;
    SetFlag(Z, m_RegisterX == 0x00);
    SetFlag(N, m_RegisterX & 0x80);
    return 1;
}

uint8_t Cpu::Instruction_LDY() {
    Fetch();
    m_RegisterY = m_ValueFetched;
    SetFlag(Z, m_RegisterY == 0x00);
    SetFlag(N, m_RegisterY & 0x80);
    return 1;
}

uint8_t Cpu::Instruction_LSR() {
    Fetch();
    SetFlag(C, m_ValueFetched & 0x0001);
    uint16_t temp = m_ValueFetched >> 1;
    SetFlag(Z, (temp & 0x00FF) == 0x0000);
    SetFlag(N, temp & 0x0080);
    if (m_AddressingMode == AddressingMode::ACCUMMULATOR_ADDRESSING)
        m_RegisterA = temp & 0x00FF;
    else
        Write(m_AddressAbsolute, temp & 0x00FF);
    return 0;
}

uint8_t Cpu::Instruction_NOP() {
    // todo: support
    // https://wiki.nesdev.com/w/index.php/CPU_unofficial_opcodes
    switch (m_OpCode) {
        case 0x1C:
        case 0x3C:
        case 0x5C:
        case 0x7C:
        case 0xDC:
        case 0xFC:
            return 1;
            break;
    }
    return 0;
}

uint8_t Cpu::Instruction_ORA() {
    Fetch();
    m_RegisterA = m_RegisterA | m_ValueFetched;
    SetFlag(Z, m_RegisterA == 0x00);
    SetFlag(N, m_RegisterA & 0x80);
    return 1;
}

uint8_t Cpu::Instruction_PHA() {
    Write(0x0100 + m_StackPointer, m_RegisterA);
    m_StackPointer--;
    return 0;
}

uint8_t Cpu::Instruction_PHP() {
    Write(0x0100 + m_StackPointer, m_StatusRegister | B | U);
    SetFlag(B, 0);
    SetFlag(U, 0);
    m_StackPointer--;
    return 0;
}

uint8_t Cpu::Instruction_PLA() {
    m_StackPointer++;
    m_RegisterA = Read(0x0100 + m_StackPointer);
    SetFlag(Z, m_RegisterA == 0x00);
    SetFlag(N, m_RegisterA & 0x80);
    return 0;
}

uint8_t Cpu::Instruction_PLP() {
    m_StackPointer++;
    m_StatusRegister = Read(0x0100 + m_StackPointer);
    SetFlag(U, 1);
    return 0;
}

uint8_t Cpu::Instruction_ROL() {
    Fetch();
    uint16_t temp = static_cast<uint16_t>(m_ValueFetched << 1) | GetFlag(C);
    SetFlag(C, temp & 0xFF00);
    SetFlag(Z, (temp & 0x00FF) == 0x0000);
    SetFlag(N, temp & 0x0080);
    if (m_AddressingMode == AddressingMode::ACCUMMULATOR_ADDRESSING)
        m_RegisterA = temp & 0x00FF;
    else
        Write(m_AddressAbsolute, temp & 0x00FF);
    return 0;
}

uint8_t Cpu::Instruction_ROR() {
    Fetch();
    uint16_t temp =
        static_cast<uint16_t>(GetFlag(C) << 7) | (m_ValueFetched >> 1);
    SetFlag(C, m_ValueFetched & 0x01);
    SetFlag(Z, (temp & 0x00FF) == 0x00);
    SetFlag(N, temp & 0x0080);
    if (m_AddressingMode == AddressingMode::ACCUMMULATOR_ADDRESSING)
        m_RegisterA = temp & 0x00FF;
    else
        Write(m_AddressAbsolute, temp & 0x00FF);
    return 0;
}

uint8_t Cpu::Instruction_RTI() {
    m_StackPointer++;
    m_StatusRegister = Read(0x0100 + m_StackPointer);
    m_StatusRegister &= ~B;
    m_StatusRegister &= ~U;

    m_StackPointer++;
    m_ProgramCounter = static_cast<uint16_t>(Read(0x0100 + m_StackPointer));
    m_StackPointer++;
    m_ProgramCounter |= static_cast<uint16_t>(Read(0x0100 + m_StackPointer))
                        << 8;
    return 0;
}

uint8_t Cpu::Instruction_RTS() {
    m_StackPointer++;
    m_ProgramCounter = static_cast<uint16_t>(Read(0x0100 + m_StackPointer));
    m_StackPointer++;
    m_ProgramCounter |= static_cast<uint16_t>(Read(0x0100 + m_StackPointer))
                        << 8;

    m_ProgramCounter++;
    return 0;
}

uint8_t Cpu::Instruction_SBC() {
    Fetch();

    // Operating in 16-bit domain to capture carry out

    // We can invert the bottom 8 bits with bitwise xor
    uint16_t value = static_cast<uint16_t>(m_ValueFetched) ^ 0x00FF;

    // Notice this is exactly the same as addition from here!
    uint16_t temp = static_cast<uint16_t>(m_RegisterA) + value +
                    static_cast<uint16_t>(GetFlag(C));
    SetFlag(C, temp & 0xFF00);
    SetFlag(Z, ((temp & 0x00FF) == 0));
    SetFlag(V, (temp ^ static_cast<uint16_t>(m_RegisterA)) & (temp ^ value) &
                   0x0080);
    SetFlag(N, temp & 0x0080);
    m_RegisterA = temp & 0x00FF;
    return 1;
}

uint8_t Cpu::Instruction_SEC() {
    SetFlag(C, true);
    return 0;
}

uint8_t Cpu::Instruction_SED() {
    SetFlag(D, true);
    return 0;
}

uint8_t Cpu::Instruction_SEI() {
    SetFlag(I, true);
    return 0;
}

uint8_t Cpu::Instruction_STA() {
    Write(m_AddressAbsolute, m_RegisterA);
    return 0;
}

uint8_t Cpu::Instruction_STX() {
    Write(m_AddressAbsolute, m_RegisterX);
    return 0;
}

uint8_t Cpu::Instruction_STY() {
    Write(m_AddressAbsolute, m_RegisterY);
    return 0;
}

uint8_t Cpu::Instruction_TAX() {
    m_RegisterX = m_RegisterA;
    SetFlag(Z, m_RegisterX == 0x00);
    SetFlag(N, m_RegisterX & 0x80);
    return 0;
}

uint8_t Cpu::Instruction_TAY() {
    m_RegisterY = m_RegisterA;
    SetFlag(Z, m_RegisterY == 0x00);
    SetFlag(N, m_RegisterY & 0x80);
    return 0;
}

uint8_t Cpu::Instruction_TSX() {
    m_RegisterX = m_StackPointer;
    SetFlag(Z, m_RegisterX == 0x00);
    SetFlag(N, m_RegisterX & 0x80);
    return 0;
}

uint8_t Cpu::Instruction_TXA() {
    m_RegisterA = m_RegisterX;
    SetFlag(Z, m_RegisterA == 0x00);
    SetFlag(N, m_RegisterA & 0x80);
    return 0;
}

uint8_t Cpu::Instruction_TXS() {
    m_StackPointer = m_RegisterX;
    return 0;
}

uint8_t Cpu::Instruction_TYA() {
    m_RegisterA = m_RegisterY;
    SetFlag(Z, m_RegisterA == 0x00);
    SetFlag(N, m_RegisterA & 0x80);
    return 0;
}

void Cpu::AppendAddressingModeString(uint16_t opCodeAddress,
                                     AddressingMode addressingMode,
                                     std::string& outStr) {
    switch (addressingMode) {
        case cpuemulator::Cpu::ACCUMMULATOR_ADDRESSING:
            outStr += " A [ACC]";
            break;
        case cpuemulator::Cpu::IMPLIED_ADDRESSING:
            outStr += " [IMP]";
            break;
        case cpuemulator::Cpu::IMMEDIATE_ADDRESSING:
            outStr += fmt::format(" #${:02X} [IMM]", Read(opCodeAddress + 1));
            break;
        case cpuemulator::Cpu::ZERO_PAGE_ADDRESSING:
            outStr += fmt::format(" ${:02X} [ZPG]", Read(opCodeAddress + 1));
            break;
        case cpuemulator::Cpu::INDEXED_ZERO_PAGE_ADDRESSING_X:
            outStr +=
                fmt::format(" ${:02X},X [ZPG,X]", Read(opCodeAddress + 1));
            break;
        case cpuemulator::Cpu::INDEXED_ZERO_PAGE_ADDRESSING_Y:
            outStr +=
                fmt::format(" ${:02X},Y [ZPG,Y]", Read(opCodeAddress + 1));
            break;
        case cpuemulator::Cpu::ABSOLUTE_ADDRESSING:
            outStr +=
                fmt::format(" ${:02X}{:02X} [ABS]", Read(opCodeAddress + 2),
                            Read(opCodeAddress + 1));
            break;
        case cpuemulator::Cpu::INDEXED_ABSOLUTE_ADDRESSING_X:
            outStr +=
                fmt::format(" ${:02X}{:02X},X [ABS,X]", Read(opCodeAddress + 2),
                            Read(opCodeAddress + 1));
            break;
        case cpuemulator::Cpu::INDEXED_ABSOLUTE_ADDRESSING_Y:
            outStr +=
                fmt::format(" ${:02X}{:02X},Y [ABS,Y]", Read(opCodeAddress + 2),
                            Read(opCodeAddress + 1));
            break;
        case cpuemulator::Cpu::ABSOLUTE_INDIRECT_ADDRESSING:
            outStr +=
                fmt::format(" (${:02X}{:02X}) [IND]", Read(opCodeAddress + 2),
                            Read(opCodeAddress + 1));
            break;
        case cpuemulator::Cpu::INDEXED_INDIRECT_ADDRESSING_X:
            outStr +=
                fmt::format(" (${:02X},X) [IND,X]", Read(opCodeAddress + 1));
            break;
        case cpuemulator::Cpu::INDIRECT_INDEXED_ADDRESSING_Y:
            outStr +=
                fmt::format(" (${:02X}),Y [IND,Y]", Read(opCodeAddress + 1));
            break;
        case cpuemulator::Cpu::RELATIVE_ADDRESSING:
            outStr += fmt::format(" ${:02X} [REL]",
                                  static_cast<int>(Read(opCodeAddress + 1)));
            break;
        default:
            break;
    }
}

std::string Cpu::GetInstructionString(uint16_t opCodeAddress) {
    std::string str;

    std::optional<Instruction>& instr = m_InstrTable[Read(opCodeAddress)];
    if (instr.has_value()) {
        str += instr->m_Name;
        AppendAddressingModeString(opCodeAddress, instr->m_AddressingMode, str);
    } else {
        str = "NOP [IMP]";
    }
    return str;
}

void Cpu::RegisterAllInstructionSet() {
    m_InstrTable.resize(0xFF, std::optional<Instruction>());

    using am = AddressingMode;
    m_InstrTable[0x00].emplace("BRK", std::bind(&Cpu::Instruction_BRK, this),
                               am::IMPLIED_ADDRESSING, 7);
    m_InstrTable[0x01].emplace("ORA", std::bind(&Cpu::Instruction_ORA, this),
                               am::INDEXED_INDIRECT_ADDRESSING_X, 6);
    m_InstrTable[0x05].emplace("ORA", std::bind(&Cpu::Instruction_ORA, this),
                               am::ZERO_PAGE_ADDRESSING, 3);
    m_InstrTable[0x06].emplace("ASL", std::bind(&Cpu::Instruction_ASL, this),
                               am::ZERO_PAGE_ADDRESSING, 5);
    m_InstrTable[0x08].emplace("PHP", std::bind(&Cpu::Instruction_PHP, this),
                               am::IMPLIED_ADDRESSING, 3);
    m_InstrTable[0x09].emplace("ORA", std::bind(&Cpu::Instruction_ORA, this),
                               am::IMMEDIATE_ADDRESSING, 2);
    m_InstrTable[0x0A].emplace("ASL", std::bind(&Cpu::Instruction_ASL, this),
                               am::ACCUMMULATOR_ADDRESSING, 2);
    m_InstrTable[0x0D].emplace("ORA", std::bind(&Cpu::Instruction_ORA, this),
                               am::ABSOLUTE_ADDRESSING, 4);
    m_InstrTable[0x0E].emplace("ASL", std::bind(&Cpu::Instruction_ASL, this),
                               am::ABSOLUTE_ADDRESSING, 6);
    m_InstrTable[0x10].emplace("BPL", std::bind(&Cpu::Instruction_BPL, this),
                               am::RELATIVE_ADDRESSING, 2);
    m_InstrTable[0x11].emplace("ORA", std::bind(&Cpu::Instruction_ORA, this),
                               am::INDIRECT_INDEXED_ADDRESSING_Y, 5);
    m_InstrTable[0x15].emplace("ORA", std::bind(&Cpu::Instruction_ORA, this),
                               am::INDEXED_ZERO_PAGE_ADDRESSING_X, 4);
    m_InstrTable[0x16].emplace("ASL", std::bind(&Cpu::Instruction_ASL, this),
                               am::INDEXED_ZERO_PAGE_ADDRESSING_X, 6);
    m_InstrTable[0x18].emplace("CLC", std::bind(&Cpu::Instruction_CLC, this),
                               am::IMPLIED_ADDRESSING, 2);
    m_InstrTable[0x19].emplace("ORA", std::bind(&Cpu::Instruction_ORA, this),
                               am::INDEXED_ABSOLUTE_ADDRESSING_Y, 4);
    m_InstrTable[0x1D].emplace("ORA", std::bind(&Cpu::Instruction_ORA, this),
                               am::INDEXED_ABSOLUTE_ADDRESSING_X, 4);
    m_InstrTable[0x1E].emplace("ASL", std::bind(&Cpu::Instruction_ASL, this),
                               am::INDEXED_ABSOLUTE_ADDRESSING_X, 7);
    m_InstrTable[0x20].emplace("JSR", std::bind(&Cpu::Instruction_JSR, this),
                               am::ABSOLUTE_ADDRESSING, 6);
    m_InstrTable[0x21].emplace("AND", std::bind(&Cpu::Instruction_AND, this),
                               am::INDEXED_INDIRECT_ADDRESSING_X, 6);
    m_InstrTable[0x24].emplace("BIT", std::bind(&Cpu::Instruction_BIT, this),
                               am::ZERO_PAGE_ADDRESSING, 3);
    m_InstrTable[0x25].emplace("AND", std::bind(&Cpu::Instruction_AND, this),
                               am::ZERO_PAGE_ADDRESSING, 3);
    m_InstrTable[0x26].emplace("ROL", std::bind(&Cpu::Instruction_ROL, this),
                               am::ZERO_PAGE_ADDRESSING, 5);
    m_InstrTable[0x28].emplace("PLP", std::bind(&Cpu::Instruction_PLP, this),
                               am::IMPLIED_ADDRESSING, 4);
    m_InstrTable[0x29].emplace("AND", std::bind(&Cpu::Instruction_AND, this),
                               am::IMMEDIATE_ADDRESSING, 2);
    m_InstrTable[0x2A].emplace("ROL", std::bind(&Cpu::Instruction_ROL, this),
                               am::ACCUMMULATOR_ADDRESSING, 2);
    m_InstrTable[0x2C].emplace("BIT", std::bind(&Cpu::Instruction_BIT, this),
                               am::ABSOLUTE_ADDRESSING, 4);
    m_InstrTable[0x2D].emplace("AND", std::bind(&Cpu::Instruction_AND, this),
                               am::ABSOLUTE_ADDRESSING, 4);
    m_InstrTable[0x2E].emplace("ROL", std::bind(&Cpu::Instruction_ROL, this),
                               am::ABSOLUTE_ADDRESSING, 6);
    m_InstrTable[0x30].emplace("BMI", std::bind(&Cpu::Instruction_BMI, this),
                               am::RELATIVE_ADDRESSING, 2);
    m_InstrTable[0x31].emplace("AND", std::bind(&Cpu::Instruction_AND, this),
                               am::INDIRECT_INDEXED_ADDRESSING_Y, 5);
    m_InstrTable[0x35].emplace("AND", std::bind(&Cpu::Instruction_AND, this),
                               am::INDEXED_ZERO_PAGE_ADDRESSING_X, 4);
    m_InstrTable[0x36].emplace("ROL", std::bind(&Cpu::Instruction_ROL, this),
                               am::INDEXED_ZERO_PAGE_ADDRESSING_X, 6);
    m_InstrTable[0x38].emplace("SEC", std::bind(&Cpu::Instruction_SEC, this),
                               am::IMPLIED_ADDRESSING, 2);
    m_InstrTable[0x39].emplace("AND", std::bind(&Cpu::Instruction_AND, this),
                               am::INDEXED_ABSOLUTE_ADDRESSING_Y, 4);
    m_InstrTable[0x3D].emplace("AND", std::bind(&Cpu::Instruction_AND, this),
                               am::INDEXED_ABSOLUTE_ADDRESSING_X, 4);
    m_InstrTable[0x3E].emplace("ROL", std::bind(&Cpu::Instruction_ROL, this),
                               am::INDEXED_ABSOLUTE_ADDRESSING_X, 7);
    m_InstrTable[0x40].emplace("RTI", std::bind(&Cpu::Instruction_RTI, this),
                               am::IMPLIED_ADDRESSING, 6);
    m_InstrTable[0x41].emplace("EOR", std::bind(&Cpu::Instruction_EOR, this),
                               am::INDEXED_INDIRECT_ADDRESSING_X, 6);
    m_InstrTable[0x45].emplace("EOR", std::bind(&Cpu::Instruction_EOR, this),
                               am::ZERO_PAGE_ADDRESSING, 3);
    m_InstrTable[0x46].emplace("LSR", std::bind(&Cpu::Instruction_LSR, this),
                               am::ZERO_PAGE_ADDRESSING, 5);
    m_InstrTable[0x48].emplace("PHA", std::bind(&Cpu::Instruction_PHA, this),
                               am::IMPLIED_ADDRESSING, 3);
    m_InstrTable[0x49].emplace("EOR", std::bind(&Cpu::Instruction_EOR, this),
                               am::IMMEDIATE_ADDRESSING, 2);
    m_InstrTable[0x4A].emplace("LSR", std::bind(&Cpu::Instruction_LSR, this),
                               am::ACCUMMULATOR_ADDRESSING, 2);
    m_InstrTable[0x4C].emplace("JMP", std::bind(&Cpu::Instruction_JMP, this),
                               am::ABSOLUTE_ADDRESSING, 3);
    m_InstrTable[0x4D].emplace("EOR", std::bind(&Cpu::Instruction_EOR, this),
                               am::ABSOLUTE_ADDRESSING, 4);
    m_InstrTable[0x4E].emplace("LSR", std::bind(&Cpu::Instruction_LSR, this),
                               am::ABSOLUTE_ADDRESSING, 6);
    m_InstrTable[0x50].emplace("BVC", std::bind(&Cpu::Instruction_BVC, this),
                               am::RELATIVE_ADDRESSING, 2);
    m_InstrTable[0x51].emplace("EOR", std::bind(&Cpu::Instruction_EOR, this),
                               am::INDIRECT_INDEXED_ADDRESSING_Y, 5);
    m_InstrTable[0x55].emplace("EOR", std::bind(&Cpu::Instruction_EOR, this),
                               am::INDEXED_ZERO_PAGE_ADDRESSING_X, 4);
    m_InstrTable[0x56].emplace("LSR", std::bind(&Cpu::Instruction_LSR, this),
                               am::INDEXED_ZERO_PAGE_ADDRESSING_X, 6);
    m_InstrTable[0x58].emplace("CLI", std::bind(&Cpu::Instruction_CLI, this),
                               am::IMPLIED_ADDRESSING, 2);
    m_InstrTable[0x59].emplace("EOR", std::bind(&Cpu::Instruction_EOR, this),
                               am::INDEXED_ABSOLUTE_ADDRESSING_Y, 4);
    m_InstrTable[0x5D].emplace("EOR", std::bind(&Cpu::Instruction_EOR, this),
                               am::INDEXED_ABSOLUTE_ADDRESSING_X, 4);
    m_InstrTable[0x5E].emplace("LSR", std::bind(&Cpu::Instruction_LSR, this),
                               am::INDEXED_ABSOLUTE_ADDRESSING_X, 7);
    m_InstrTable[0x60].emplace("RTS", std::bind(&Cpu::Instruction_RTS, this),
                               am::IMPLIED_ADDRESSING, 6);
    m_InstrTable[0x61].emplace("ADC", std::bind(&Cpu::Instruction_ADC, this),
                               am::INDEXED_INDIRECT_ADDRESSING_X, 6);
    m_InstrTable[0x65].emplace("ADC", std::bind(&Cpu::Instruction_ADC, this),
                               am::ZERO_PAGE_ADDRESSING, 3);
    m_InstrTable[0x66].emplace("ROR", std::bind(&Cpu::Instruction_ROR, this),
                               am::ZERO_PAGE_ADDRESSING, 5);
    m_InstrTable[0x68].emplace("PLA", std::bind(&Cpu::Instruction_PLA, this),
                               am::IMPLIED_ADDRESSING, 4);
    m_InstrTable[0x69].emplace("ADC", std::bind(&Cpu::Instruction_ADC, this),
                               am::IMMEDIATE_ADDRESSING, 2);
    m_InstrTable[0x6A].emplace("ROR", std::bind(&Cpu::Instruction_ROR, this),
                               am::ACCUMMULATOR_ADDRESSING, 2);
    m_InstrTable[0x6C].emplace("JMP", std::bind(&Cpu::Instruction_JMP, this),
                               am::ABSOLUTE_INDIRECT_ADDRESSING, 5);
    m_InstrTable[0x6D].emplace("ADC", std::bind(&Cpu::Instruction_ADC, this),
                               am::ABSOLUTE_ADDRESSING, 4);
    m_InstrTable[0x6E].emplace("ROR", std::bind(&Cpu::Instruction_ROR, this),
                               am::ABSOLUTE_ADDRESSING, 6);
    m_InstrTable[0x70].emplace("BVS", std::bind(&Cpu::Instruction_BVS, this),
                               am::RELATIVE_ADDRESSING, 2);
    m_InstrTable[0x71].emplace("ADC", std::bind(&Cpu::Instruction_ADC, this),
                               am::INDIRECT_INDEXED_ADDRESSING_Y, 5);
    m_InstrTable[0x75].emplace("ADC", std::bind(&Cpu::Instruction_ADC, this),
                               am::INDEXED_ZERO_PAGE_ADDRESSING_X, 4);
    m_InstrTable[0x76].emplace("ROR", std::bind(&Cpu::Instruction_ROR, this),
                               am::INDEXED_ZERO_PAGE_ADDRESSING_X, 6);
    m_InstrTable[0x78].emplace("SEI", std::bind(&Cpu::Instruction_SEI, this),
                               am::IMPLIED_ADDRESSING, 2);
    m_InstrTable[0x79].emplace("ADC", std::bind(&Cpu::Instruction_ADC, this),
                               am::INDEXED_ABSOLUTE_ADDRESSING_Y, 4);
    m_InstrTable[0x7D].emplace("ADC", std::bind(&Cpu::Instruction_ADC, this),
                               am::INDEXED_ABSOLUTE_ADDRESSING_X, 4);
    m_InstrTable[0x7E].emplace("ROR", std::bind(&Cpu::Instruction_ROR, this),
                               am::INDEXED_ABSOLUTE_ADDRESSING_X, 7);
    m_InstrTable[0x81].emplace("STA", std::bind(&Cpu::Instruction_STA, this),
                               am::INDEXED_INDIRECT_ADDRESSING_X, 6);
    m_InstrTable[0x84].emplace("STY", std::bind(&Cpu::Instruction_STY, this),
                               am::ZERO_PAGE_ADDRESSING, 3);
    m_InstrTable[0x85].emplace("STA", std::bind(&Cpu::Instruction_STA, this),
                               am::ZERO_PAGE_ADDRESSING, 3);
    m_InstrTable[0x86].emplace("STX", std::bind(&Cpu::Instruction_STX, this),
                               am::ZERO_PAGE_ADDRESSING, 3);
    m_InstrTable[0x88].emplace("DEY", std::bind(&Cpu::Instruction_DEY, this),
                               am::IMPLIED_ADDRESSING, 2);
    m_InstrTable[0x8A].emplace("TXA", std::bind(&Cpu::Instruction_TXA, this),
                               am::IMPLIED_ADDRESSING, 2);
    m_InstrTable[0x8C].emplace("STY", std::bind(&Cpu::Instruction_STY, this),
                               am::ABSOLUTE_ADDRESSING, 4);
    m_InstrTable[0x8D].emplace("STA", std::bind(&Cpu::Instruction_STA, this),
                               am::ABSOLUTE_ADDRESSING, 4);
    m_InstrTable[0x8E].emplace("STX", std::bind(&Cpu::Instruction_STX, this),
                               am::ABSOLUTE_ADDRESSING, 4);
    m_InstrTable[0x90].emplace("BCC", std::bind(&Cpu::Instruction_BCC, this),
                               am::RELATIVE_ADDRESSING, 2);
    m_InstrTable[0x91].emplace("STA", std::bind(&Cpu::Instruction_STA, this),
                               am::INDIRECT_INDEXED_ADDRESSING_Y, 6);
    m_InstrTable[0x94].emplace("STY", std::bind(&Cpu::Instruction_STY, this),
                               am::INDEXED_ZERO_PAGE_ADDRESSING_X, 4);
    m_InstrTable[0x95].emplace("STA", std::bind(&Cpu::Instruction_STA, this),
                               am::INDEXED_ZERO_PAGE_ADDRESSING_X, 4);
    m_InstrTable[0x96].emplace("STX", std::bind(&Cpu::Instruction_STX, this),
                               am::INDEXED_ZERO_PAGE_ADDRESSING_Y, 4);
    m_InstrTable[0x98].emplace("TYA", std::bind(&Cpu::Instruction_TYA, this),
                               am::IMPLIED_ADDRESSING, 2);
    m_InstrTable[0x99].emplace("STA", std::bind(&Cpu::Instruction_STA, this),
                               am::INDEXED_ABSOLUTE_ADDRESSING_Y, 5);
    m_InstrTable[0x9A].emplace("TXS", std::bind(&Cpu::Instruction_TXS, this),
                               am::IMPLIED_ADDRESSING, 2);
    m_InstrTable[0x9D].emplace("STA", std::bind(&Cpu::Instruction_STA, this),
                               am::INDEXED_ABSOLUTE_ADDRESSING_X, 5);
    m_InstrTable[0xA0].emplace("LDY", std::bind(&Cpu::Instruction_LDY, this),
                               am::IMMEDIATE_ADDRESSING, 2);
    m_InstrTable[0xA1].emplace("LDA", std::bind(&Cpu::Instruction_LDA, this),
                               am::INDEXED_INDIRECT_ADDRESSING_X, 6);
    m_InstrTable[0xA2].emplace("LDX", std::bind(&Cpu::Instruction_LDX, this),
                               am::IMMEDIATE_ADDRESSING, 2);
    m_InstrTable[0xA4].emplace("LDY", std::bind(&Cpu::Instruction_LDY, this),
                               am::ZERO_PAGE_ADDRESSING, 3);
    m_InstrTable[0xA5].emplace("LDA", std::bind(&Cpu::Instruction_LDA, this),
                               am::ZERO_PAGE_ADDRESSING, 3);
    m_InstrTable[0xA6].emplace("LDX", std::bind(&Cpu::Instruction_LDX, this),
                               am::ZERO_PAGE_ADDRESSING, 3);
    m_InstrTable[0xA8].emplace("TAY", std::bind(&Cpu::Instruction_TAY, this),
                               am::IMPLIED_ADDRESSING, 2);
    m_InstrTable[0xA9].emplace("LDA", std::bind(&Cpu::Instruction_LDA, this),
                               am::IMMEDIATE_ADDRESSING, 2);
    m_InstrTable[0xAA].emplace("TAX", std::bind(&Cpu::Instruction_TAX, this),
                               am::IMPLIED_ADDRESSING, 2);
    m_InstrTable[0xAC].emplace("LDY", std::bind(&Cpu::Instruction_LDY, this),
                               am::ABSOLUTE_ADDRESSING, 4);
    m_InstrTable[0xAD].emplace("LDA", std::bind(&Cpu::Instruction_LDA, this),
                               am::ABSOLUTE_ADDRESSING, 4);
    m_InstrTable[0xAE].emplace("LDX", std::bind(&Cpu::Instruction_LDX, this),
                               am::ABSOLUTE_ADDRESSING, 4);
    m_InstrTable[0xB0].emplace("BCS", std::bind(&Cpu::Instruction_BCS, this),
                               am::RELATIVE_ADDRESSING, 2);
    m_InstrTable[0xB1].emplace("LDA", std::bind(&Cpu::Instruction_LDA, this),
                               am::INDIRECT_INDEXED_ADDRESSING_Y, 5);
    m_InstrTable[0xB4].emplace("LDY", std::bind(&Cpu::Instruction_LDY, this),
                               am::INDEXED_ZERO_PAGE_ADDRESSING_X, 4);
    m_InstrTable[0xB5].emplace("LDA", std::bind(&Cpu::Instruction_LDA, this),
                               am::INDEXED_ZERO_PAGE_ADDRESSING_X, 4);
    m_InstrTable[0xB6].emplace("LDX", std::bind(&Cpu::Instruction_LDX, this),
                               am::INDEXED_ZERO_PAGE_ADDRESSING_Y, 4);
    m_InstrTable[0xB8].emplace("CLV", std::bind(&Cpu::Instruction_CLV, this),
                               am::IMPLIED_ADDRESSING, 2);
    m_InstrTable[0xB9].emplace("LDA", std::bind(&Cpu::Instruction_LDA, this),
                               am::INDEXED_ABSOLUTE_ADDRESSING_Y, 4);
    m_InstrTable[0xBA].emplace("TSX", std::bind(&Cpu::Instruction_TSX, this),
                               am::IMPLIED_ADDRESSING, 2);
    m_InstrTable[0xBC].emplace("LDY", std::bind(&Cpu::Instruction_LDY, this),
                               am::INDEXED_ABSOLUTE_ADDRESSING_X, 4);
    m_InstrTable[0xBD].emplace("LDA", std::bind(&Cpu::Instruction_LDA, this),
                               am::INDEXED_ABSOLUTE_ADDRESSING_X, 4);
    m_InstrTable[0xBE].emplace("LDX", std::bind(&Cpu::Instruction_LDX, this),
                               am::INDEXED_ABSOLUTE_ADDRESSING_Y, 4);
    m_InstrTable[0xC0].emplace("CPY", std::bind(&Cpu::Instruction_CPY, this),
                               am::IMMEDIATE_ADDRESSING, 2);
    m_InstrTable[0xC1].emplace("CMP", std::bind(&Cpu::Instruction_CMP, this),
                               am::INDEXED_INDIRECT_ADDRESSING_X, 6);
    m_InstrTable[0xC4].emplace("CPY", std::bind(&Cpu::Instruction_CPY, this),
                               am::ZERO_PAGE_ADDRESSING, 3);
    m_InstrTable[0xC5].emplace("CMP", std::bind(&Cpu::Instruction_CMP, this),
                               am::ZERO_PAGE_ADDRESSING, 3);
    m_InstrTable[0xC6].emplace("DEC", std::bind(&Cpu::Instruction_DEC, this),
                               am::ZERO_PAGE_ADDRESSING, 5);
    m_InstrTable[0xC8].emplace("INY", std::bind(&Cpu::Instruction_INY, this),
                               am::IMPLIED_ADDRESSING, 2);
    m_InstrTable[0xC9].emplace("CMP", std::bind(&Cpu::Instruction_CMP, this),
                               am::IMMEDIATE_ADDRESSING, 2);
    m_InstrTable[0xCA].emplace("DEX", std::bind(&Cpu::Instruction_DEX, this),
                               am::IMPLIED_ADDRESSING, 2);
    m_InstrTable[0xCC].emplace("CPY", std::bind(&Cpu::Instruction_CPY, this),
                               am::ABSOLUTE_ADDRESSING, 4);
    m_InstrTable[0xCD].emplace("CMP", std::bind(&Cpu::Instruction_CMP, this),
                               am::ABSOLUTE_ADDRESSING, 4);
    m_InstrTable[0xCE].emplace("DEC", std::bind(&Cpu::Instruction_DEC, this),
                               am::ABSOLUTE_ADDRESSING, 6);
    m_InstrTable[0xD0].emplace("BNE", std::bind(&Cpu::Instruction_BNE, this),
                               am::RELATIVE_ADDRESSING, 2);
    m_InstrTable[0xD1].emplace("CMP", std::bind(&Cpu::Instruction_CMP, this),
                               am::INDIRECT_INDEXED_ADDRESSING_Y, 5);
    m_InstrTable[0xD5].emplace("CMP", std::bind(&Cpu::Instruction_CMP, this),
                               am::INDEXED_ZERO_PAGE_ADDRESSING_X, 4);
    m_InstrTable[0xD6].emplace("DEC", std::bind(&Cpu::Instruction_DEC, this),
                               am::INDEXED_ZERO_PAGE_ADDRESSING_X, 6);
    m_InstrTable[0xD8].emplace("CLD", std::bind(&Cpu::Instruction_CLD, this),
                               am::IMPLIED_ADDRESSING, 2);
    m_InstrTable[0xD9].emplace("CMP", std::bind(&Cpu::Instruction_CMP, this),
                               am::INDEXED_ABSOLUTE_ADDRESSING_Y, 4);
    m_InstrTable[0xDD].emplace("CMP", std::bind(&Cpu::Instruction_CMP, this),
                               am::INDEXED_ABSOLUTE_ADDRESSING_X, 4);
    m_InstrTable[0xDE].emplace("DEC", std::bind(&Cpu::Instruction_DEC, this),
                               am::INDEXED_ABSOLUTE_ADDRESSING_X, 7);
    m_InstrTable[0xE0].emplace("CPX", std::bind(&Cpu::Instruction_CPX, this),
                               am::IMMEDIATE_ADDRESSING, 2);
    m_InstrTable[0xE1].emplace("SBC", std::bind(&Cpu::Instruction_SBC, this),
                               am::INDEXED_INDIRECT_ADDRESSING_X, 6);
    m_InstrTable[0xE4].emplace("CPX", std::bind(&Cpu::Instruction_CPX, this),
                               am::ZERO_PAGE_ADDRESSING, 3);
    m_InstrTable[0xE5].emplace("SBC", std::bind(&Cpu::Instruction_SBC, this),
                               am::ZERO_PAGE_ADDRESSING, 3);
    m_InstrTable[0xE6].emplace("INC", std::bind(&Cpu::Instruction_INC, this),
                               am::ZERO_PAGE_ADDRESSING, 5);
    m_InstrTable[0xE8].emplace("INX", std::bind(&Cpu::Instruction_INX, this),
                               am::IMPLIED_ADDRESSING, 2);
    m_InstrTable[0xE9].emplace("SBC", std::bind(&Cpu::Instruction_SBC, this),
                               am::IMMEDIATE_ADDRESSING, 2);
    m_InstrTable[0xEA].emplace("NOP", std::bind(&Cpu::Instruction_NOP, this),
                               am::IMPLIED_ADDRESSING, 2);
    m_InstrTable[0xEC].emplace("CPX", std::bind(&Cpu::Instruction_CPX, this),
                               am::ABSOLUTE_ADDRESSING, 4);
    m_InstrTable[0xED].emplace("SBC", std::bind(&Cpu::Instruction_SBC, this),
                               am::ABSOLUTE_ADDRESSING, 4);
    m_InstrTable[0xEE].emplace("INC", std::bind(&Cpu::Instruction_INC, this),
                               am::ABSOLUTE_ADDRESSING, 6);
    m_InstrTable[0xF0].emplace("BEQ", std::bind(&Cpu::Instruction_BEQ, this),
                               am::RELATIVE_ADDRESSING, 2);
    m_InstrTable[0xF1].emplace("SBC", std::bind(&Cpu::Instruction_SBC, this),
                               am::INDIRECT_INDEXED_ADDRESSING_Y, 5);
    m_InstrTable[0xF5].emplace("SBC", std::bind(&Cpu::Instruction_SBC, this),
                               am::INDEXED_ZERO_PAGE_ADDRESSING_X, 4);
    m_InstrTable[0xF6].emplace("INC", std::bind(&Cpu::Instruction_INC, this),
                               am::INDEXED_ZERO_PAGE_ADDRESSING_X, 6);
    m_InstrTable[0xF8].emplace("SED", std::bind(&Cpu::Instruction_SED, this),
                               am::IMPLIED_ADDRESSING, 2);
    m_InstrTable[0xF9].emplace("SBC", std::bind(&Cpu::Instruction_SBC, this),
                               am::INDEXED_ABSOLUTE_ADDRESSING_Y, 4);
    m_InstrTable[0xFD].emplace("SBC", std::bind(&Cpu::Instruction_SBC, this),
                               am::INDEXED_ABSOLUTE_ADDRESSING_X, 4);
    m_InstrTable[0xFE].emplace("INC", std::bind(&Cpu::Instruction_INC, this),
                               am::INDEXED_ABSOLUTE_ADDRESSING_X, 7);
}
}  // namespace cpuemulator

#pragma endregion INSTRUCTIONS
