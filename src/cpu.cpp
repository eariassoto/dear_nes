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
        m_OpCode = ReadWordFromProgramCounter();

        SetFlag(FLAGS::U, 1);

        // todo handle illegal ops
        std::optional<Instruction>& instr = m_InstrTable[m_OpCode];
        if (!instr.has_value()) {
            instr = m_InstrTable[0xEA];
        }

        m_Cycles = instr->m_Cycles;

        bool addrRequiresAdditionalCycle = false;
        if (instr->m_AddressingMode) {
            addrRequiresAdditionalCycle = instr->m_AddressingMode(this);
        }

        bool instrRequiresAdditionalCycle = instr->m_FuncOperate(this);

        if (addrRequiresAdditionalCycle && instrRequiresAdditionalCycle) {
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

uint8_t Cpu::ReadWordFromProgramCounter() {
    return m_NesPtr->CpuRead(m_ProgramCounter++);
}

uint16_t Cpu::ReadDoubleWordFromProgramCounter() {
    uint16_t lowNibble = Read(m_ProgramCounter++);
    uint16_t highNibble = Read(m_ProgramCounter++);

    return (highNibble << 8) | lowNibble;
}

void Cpu::Write(uint16_t address, uint8_t data) {
    m_NesPtr->CpuWrite(address, data);
}

#pragma region ADDRESSING_MODES
bool Cpu::ImmediateAddressing() {
    m_AddressAbsolute = m_ProgramCounter++;
    return false;
}

bool Cpu::ZeroPageAddressing() {
    m_AddressAbsolute = ReadWordFromProgramCounter();
    m_AddressAbsolute &= 0x00FF;
    return false;
}

bool Cpu::IndexedZeroPageAddressingX() {
    m_AddressAbsolute = ReadWordFromProgramCounter();
    m_AddressAbsolute += m_RegisterX;
    m_AddressAbsolute &= 0x00FF;
    return false;
}

bool Cpu::IndexedZeroPageAddressingY() {
    m_AddressAbsolute = ReadWordFromProgramCounter();
    m_AddressAbsolute += m_RegisterY;
    m_AddressAbsolute &= 0x00FF;
    return false;
}

bool Cpu::AbsoluteAddressing() {
    m_AddressAbsolute = ReadDoubleWordFromProgramCounter();
    return false;
}

bool Cpu::IndexedAbsoluteAddressingX() {
    m_AddressAbsolute = ReadDoubleWordFromProgramCounter();
    uint16_t highNibble = m_AddressAbsolute & 0xFF00;

    m_AddressAbsolute += m_RegisterX;

    return (m_AddressAbsolute & 0xFF00) != highNibble;
}

bool Cpu::IndexedAbsoluteAddressingY() {
    m_AddressAbsolute = ReadDoubleWordFromProgramCounter();
    uint16_t highNibble = m_AddressAbsolute & 0xFF00;

    m_AddressAbsolute += m_RegisterY;

    return (m_AddressAbsolute & 0xFF00) != highNibble;
}

bool Cpu::AbsoluteIndirectAddressing() {
    uint16_t pointer = ReadDoubleWordFromProgramCounter();

    // Simulate page boundary hardware bug
    if ((pointer & 0x00FF) == 0x00FF) {
        m_AddressAbsolute = (Read(pointer & 0xFF00) << 8) | Read(pointer);
    } else {
        m_AddressAbsolute = (Read(pointer + 1) << 8) | Read(pointer);
    }
    return false;
}

bool Cpu::IndexedIndirectAddressingX() {
    uint16_t t = ReadWordFromProgramCounter();

    uint16_t registerXValue = static_cast<uint16_t>(m_RegisterX);

    uint16_t lowNibbleAddress = (t + registerXValue) & 0x00FF;
    uint16_t lowNibble = Read(lowNibbleAddress);

    uint16_t highNibbleAddress = (t + registerXValue + 1) & 0x00FF;
    uint16_t highNibble = Read(highNibbleAddress);

    m_AddressAbsolute = (highNibble << 8) | lowNibble;

    return false;
}

bool Cpu::IndirectIndexedAddressingY() {
    uint16_t t = ReadWordFromProgramCounter();

    uint16_t lowNibble = Read(t & 0x00FF);
    uint16_t highNibble = Read((t + 1) & 0x00FF);

    m_AddressAbsolute = (highNibble << 8) | lowNibble;
    m_AddressAbsolute += m_RegisterY;

    if ((m_AddressAbsolute & 0xFF00) != (highNibble << 8)) {
        return true;
    } else {
        return false;
    }
}

bool Cpu::RelativeAddressing() {
    m_AddressRelative = ReadWordFromProgramCounter();

    if (m_AddressRelative & 0x80)  // if unsigned
    {
        m_AddressRelative |= 0xFF00;
    }

    return false;
}

#pragma endregion ADDRESSING_MODES

#pragma region INSTRUCTIONS

bool Cpu::Instruction_ADC() {
    uint8_t valueFetched = Read(m_AddressAbsolute);

    uint16_t castedFetched = static_cast<uint16_t>(valueFetched);
    uint16_t castedCarry = static_cast<uint16_t>(GetFlag(FLAGS::C));
    uint16_t castedAccum = static_cast<uint16_t>(m_RegisterA);

    uint16_t temp = castedAccum + castedFetched + castedCarry;
    SetFlag(FLAGS::C, temp > 255);
    SetFlag(FLAGS::Z, (temp & 0x00FF) == 0);
    SetFlag(FLAGS::N, temp & 0x80);
    SetFlag(V,
            (~(castedAccum ^ castedFetched) & (castedAccum ^ temp)) & 0x0080);

    m_RegisterA = temp & 0x00FF;

    return true;
}

bool Cpu::Instruction_AND() {
    uint8_t valueFetched = Read(m_AddressAbsolute);

    m_RegisterA &= valueFetched;
    SetFlag(FLAGS::Z, m_RegisterA == 0x00);
    SetFlag(FLAGS::N, m_RegisterA & 0x80);

    return true;
}

bool Cpu::Instruction_ASL() {
    uint8_t valueFetched = Read(m_AddressAbsolute);

    uint16_t temp = static_cast<uint16_t>(valueFetched) << 1;
    SetFlag(FLAGS::C, (temp & 0xFF00) > 0);
    SetFlag(FLAGS::Z, (temp & 0x00FF) == 0);
    SetFlag(FLAGS::N, temp & 0x80);

    Write(m_AddressAbsolute, temp & 0x00FF);
    return false;
}

bool Cpu::Instruction_ASL_AcummAddr() {
    uint16_t temp = static_cast<uint16_t>(m_RegisterA) << 1;
    SetFlag(FLAGS::C, (temp & 0xFF00) > 0);
    SetFlag(FLAGS::Z, (temp & 0x00FF) == 0);
    SetFlag(FLAGS::N, temp & 0x80);

    m_RegisterA = temp & 0x00FF;
    return false;
}

void Cpu::Instruction_ExecuteBranch() {
    m_Cycles++;
    m_AddressAbsolute = m_ProgramCounter + m_AddressRelative;

    if ((m_AddressAbsolute & 0xFF00) != (m_ProgramCounter & 0xFF00)) {
        m_Cycles++;
    }
    m_ProgramCounter = m_AddressAbsolute;
}

bool Cpu::Instruction_BCC() {
    if (GetFlag(FLAGS::C) == 0) {
        Instruction_ExecuteBranch();
    }
    return false;
}

bool Cpu::Instruction_BCS() {
    if (GetFlag(FLAGS::C) == 1) {
        Instruction_ExecuteBranch();
    }
    return false;
}

bool Cpu::Instruction_BEQ() {
    if (GetFlag(FLAGS::Z) == 1) {
        Instruction_ExecuteBranch();
    }
    return false;
}

bool Cpu::Instruction_BIT() {
    uint8_t valueFetched = Read(m_AddressAbsolute);

    uint16_t temp = m_RegisterA & valueFetched;
    SetFlag(Z, (temp & 0x00FF) == 0x00);
    SetFlag(N, valueFetched & (1 << 7));
    SetFlag(V, valueFetched & (1 << 6));
    return false;
}

bool Cpu::Instruction_BMI() {
    if (GetFlag(FLAGS::N) == 1) {
        Instruction_ExecuteBranch();
    }
    return false;
}

bool Cpu::Instruction_BNE() {
    if (GetFlag(FLAGS::Z) == 0) {
        Instruction_ExecuteBranch();
    }
    return false;
}

bool Cpu::Instruction_BPL() {
    if (GetFlag(FLAGS::N) == 0) {
        Instruction_ExecuteBranch();
    }
    return false;
}

bool Cpu::Instruction_BRK() {
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
    return false;
}

bool Cpu::Instruction_BVC() {
    if (GetFlag(FLAGS::V) == 0) {
        Instruction_ExecuteBranch();
    }
    return false;
}

bool Cpu::Instruction_BVS() {
    if (GetFlag(FLAGS::V) == 1) {
        Instruction_ExecuteBranch();
    }
    return false;
}

bool Cpu::Instruction_CLC() {
    SetFlag(FLAGS::C, false);
    return false;
}

bool Cpu::Instruction_CLD() {
    SetFlag(FLAGS::D, false);
    return false;
}

bool Cpu::Instruction_CLI() {
    SetFlag(FLAGS::I, false);
    return false;
}

bool Cpu::Instruction_CLV() {
    SetFlag(FLAGS::V, false);
    return false;
}

bool Cpu::Instruction_CMP() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    uint16_t temp = static_cast<uint16_t>(m_RegisterA) -
                    static_cast<uint16_t>(valueFetched);
    SetFlag(C, m_RegisterA >= valueFetched);
    SetFlag(Z, (temp & 0x00FF) == 0x0000);
    SetFlag(N, temp & 0x0080);
    return true;
}

bool Cpu::Instruction_CPX() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    uint16_t temp = static_cast<uint16_t>(m_RegisterX) -
                    static_cast<uint16_t>(valueFetched);
    SetFlag(C, m_RegisterX >= valueFetched);
    SetFlag(Z, (temp & 0x00FF) == 0x0000);
    SetFlag(N, temp & 0x0080);
    return false;
}

bool Cpu::Instruction_CPY() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    uint16_t temp = static_cast<uint16_t>(m_RegisterY) -
                    static_cast<uint16_t>(valueFetched);
    SetFlag(C, m_RegisterY >= valueFetched);
    SetFlag(Z, (temp & 0x00FF) == 0x0000);
    SetFlag(N, temp & 0x0080);
    return false;
}

bool Cpu::Instruction_DEC() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    uint16_t temp = valueFetched - 1;
    Write(m_AddressAbsolute, temp & 0x00FF);
    SetFlag(Z, (temp & 0x00FF) == 0x0000);
    SetFlag(N, temp & 0x0080);
    return false;
}

bool Cpu::Instruction_DEX() {
    m_RegisterX--;
    SetFlag(Z, m_RegisterX == 0x00);
    SetFlag(N, m_RegisterX & 0x80);
    return false;
}

bool Cpu::Instruction_DEY() {
    m_RegisterY--;
    SetFlag(Z, m_RegisterY == 0x00);
    SetFlag(N, m_RegisterY & 0x80);
    return false;
}

bool Cpu::Instruction_EOR() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    m_RegisterA = m_RegisterA ^ valueFetched;
    SetFlag(Z, m_RegisterA == 0x00);
    SetFlag(N, m_RegisterA & 0x80);
    return true;
}

bool Cpu::Instruction_INC() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    uint16_t temp = valueFetched + 1;
    Write(m_AddressAbsolute, temp & 0x00FF);
    SetFlag(Z, (temp & 0x00FF) == 0x0000);
    SetFlag(N, temp & 0x0080);
    return false;
}

bool Cpu::Instruction_INX() {
    m_RegisterX++;
    SetFlag(Z, m_RegisterX == 0x00);
    SetFlag(N, m_RegisterX & 0x80);
    return false;
}

bool Cpu::Instruction_INY() {
    m_RegisterY++;
    SetFlag(Z, m_RegisterY == 0x00);
    SetFlag(N, m_RegisterY & 0x80);
    return false;
}

bool Cpu::Instruction_JMP() {
    m_ProgramCounter = m_AddressAbsolute;
    return false;
}

bool Cpu::Instruction_JSR() {
    m_ProgramCounter--;

    Write(0x0100 + m_StackPointer, (m_ProgramCounter >> 8) & 0x00FF);
    m_StackPointer--;
    Write(0x0100 + m_StackPointer, m_ProgramCounter & 0x00FF);
    m_StackPointer--;

    m_ProgramCounter = m_AddressAbsolute;
    return false;
}

bool Cpu::Instruction_LDA() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    m_RegisterA = valueFetched;
    SetFlag(Z, m_RegisterA == 0x00);
    SetFlag(N, m_RegisterA & 0x80);
    return true;
}

bool Cpu::Instruction_LDX() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    m_RegisterX = valueFetched;
    SetFlag(Z, m_RegisterX == 0x00);
    SetFlag(N, m_RegisterX & 0x80);
    return true;
}

bool Cpu::Instruction_LDY() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    m_RegisterY = valueFetched;
    SetFlag(Z, m_RegisterY == 0x00);
    SetFlag(N, m_RegisterY & 0x80);
    return true;
}

bool Cpu::Instruction_LSR() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    SetFlag(C, valueFetched & 0x0001);
    uint16_t temp = valueFetched >> 1;
    SetFlag(Z, (temp & 0x00FF) == 0x0000);
    SetFlag(N, temp & 0x0080);

    Write(m_AddressAbsolute, temp & 0x00FF);

    return false;
}

bool Cpu::Instruction_LSR_AcummAddr() {
    SetFlag(C, m_RegisterA & 0x0001);
    uint16_t temp = m_RegisterA >> 1;
    SetFlag(Z, (temp & 0x00FF) == 0x0000);
    SetFlag(N, temp & 0x0080);

    m_RegisterA = temp & 0x00FF;

    return false;
}

bool Cpu::Instruction_NOP() {
    // todo: support
    // https://wiki.nesdev.com/w/index.php/CPU_unofficial_opcodes
    switch (m_OpCode) {
        case 0x1C:
        case 0x3C:
        case 0x5C:
        case 0x7C:
        case 0xDC:
        case 0xFC:
            return true;
            break;
    }
    return false;
}

bool Cpu::Instruction_ORA() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    m_RegisterA = m_RegisterA | valueFetched;
    SetFlag(Z, m_RegisterA == 0x00);
    SetFlag(N, m_RegisterA & 0x80);
    return true;
}

bool Cpu::Instruction_PHA() {
    Write(0x0100 + m_StackPointer, m_RegisterA);
    m_StackPointer--;
    return false;
}

bool Cpu::Instruction_PHP() {
    Write(0x0100 + m_StackPointer, m_StatusRegister | B | U);
    SetFlag(B, 0);
    SetFlag(U, 0);
    m_StackPointer--;
    return false;
}

bool Cpu::Instruction_PLA() {
    m_StackPointer++;
    m_RegisterA = Read(0x0100 + m_StackPointer);
    SetFlag(Z, m_RegisterA == 0x00);
    SetFlag(N, m_RegisterA & 0x80);
    return false;
}

bool Cpu::Instruction_PLP() {
    m_StackPointer++;
    m_StatusRegister = Read(0x0100 + m_StackPointer);
    SetFlag(U, 1);
    return false;
}

bool Cpu::Instruction_ROL() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    uint16_t temp = static_cast<uint16_t>(valueFetched << 1) | GetFlag(C);
    SetFlag(C, temp & 0xFF00);
    SetFlag(Z, (temp & 0x00FF) == 0x0000);
    SetFlag(N, temp & 0x0080);

    Write(m_AddressAbsolute, temp & 0x00FF);
    return false;
}

bool Cpu::Instruction_ROL_AcummAddr() {
    uint16_t temp = static_cast<uint16_t>(m_RegisterA << 1) | GetFlag(C);
    SetFlag(C, temp & 0xFF00);
    SetFlag(Z, (temp & 0x00FF) == 0x0000);
    SetFlag(N, temp & 0x0080);

    m_RegisterA = temp & 0x00FF;
    return false;
}

bool Cpu::Instruction_ROR() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    uint16_t temp =
        static_cast<uint16_t>(GetFlag(C) << 7) | (valueFetched >> 1);
    SetFlag(C, valueFetched & 0x01);
    SetFlag(Z, (temp & 0x00FF) == 0x00);
    SetFlag(N, temp & 0x0080);

    Write(m_AddressAbsolute, temp & 0x00FF);
    return false;
}
bool Cpu::Instruction_ROR_AcummAddr() {
    uint16_t temp = static_cast<uint16_t>(GetFlag(C) << 7) | (m_RegisterA >> 1);
    SetFlag(C, m_RegisterA & 0x01);
    SetFlag(Z, (temp & 0x00FF) == 0x00);
    SetFlag(N, temp & 0x0080);

    m_RegisterA = temp & 0x00FF;
    return false;
}

bool Cpu::Instruction_RTI() {
    m_StackPointer++;
    m_StatusRegister = Read(0x0100 + m_StackPointer);
    m_StatusRegister &= ~B;
    m_StatusRegister &= ~U;

    m_StackPointer++;
    m_ProgramCounter = static_cast<uint16_t>(Read(0x0100 + m_StackPointer));
    m_StackPointer++;
    m_ProgramCounter |= static_cast<uint16_t>(Read(0x0100 + m_StackPointer))
                        << 8;
    return false;
}

bool Cpu::Instruction_RTS() {
    m_StackPointer++;
    m_ProgramCounter = static_cast<uint16_t>(Read(0x0100 + m_StackPointer));
    m_StackPointer++;
    m_ProgramCounter |= static_cast<uint16_t>(Read(0x0100 + m_StackPointer))
                        << 8;

    m_ProgramCounter++;
    return false;
}

bool Cpu::Instruction_SBC() {
    uint8_t valueFetched = Read(m_AddressAbsolute);

    // Operating in 16-bit domain to capture carry out

    // We can invert the bottom 8 bits with bitwise xor
    uint16_t value = static_cast<uint16_t>(valueFetched) ^ 0x00FF;

    // Notice this is exactly the same as addition from here!
    uint16_t temp = static_cast<uint16_t>(m_RegisterA) + value +
                    static_cast<uint16_t>(GetFlag(C));
    SetFlag(C, temp & 0xFF00);
    SetFlag(Z, ((temp & 0x00FF) == 0));
    SetFlag(V, (temp ^ static_cast<uint16_t>(m_RegisterA)) & (temp ^ value) &
                   0x0080);
    SetFlag(N, temp & 0x0080);
    m_RegisterA = temp & 0x00FF;
    return true;
}

bool Cpu::Instruction_SEC() {
    SetFlag(C, true);
    return false;
}

bool Cpu::Instruction_SED() {
    SetFlag(D, true);
    return false;
}

bool Cpu::Instruction_SEI() {
    SetFlag(I, true);
    return false;
}

bool Cpu::Instruction_STA() {
    Write(m_AddressAbsolute, m_RegisterA);
    return false;
}

bool Cpu::Instruction_STX() {
    Write(m_AddressAbsolute, m_RegisterX);
    return false;
}

bool Cpu::Instruction_STY() {
    Write(m_AddressAbsolute, m_RegisterY);
    return false;
}

bool Cpu::Instruction_TAX() {
    m_RegisterX = m_RegisterA;
    SetFlag(Z, m_RegisterX == 0x00);
    SetFlag(N, m_RegisterX & 0x80);
    return false;
}

bool Cpu::Instruction_TAY() {
    m_RegisterY = m_RegisterA;
    SetFlag(Z, m_RegisterY == 0x00);
    SetFlag(N, m_RegisterY & 0x80);
    return false;
}

bool Cpu::Instruction_TSX() {
    m_RegisterX = m_StackPointer;
    SetFlag(Z, m_RegisterX == 0x00);
    SetFlag(N, m_RegisterX & 0x80);
    return false;
}

bool Cpu::Instruction_TXA() {
    m_RegisterA = m_RegisterX;
    SetFlag(Z, m_RegisterA == 0x00);
    SetFlag(N, m_RegisterA & 0x80);
    return false;
}

bool Cpu::Instruction_TXS() {
    m_StackPointer = m_RegisterX;
    return false;
}

bool Cpu::Instruction_TYA() {
    m_RegisterA = m_RegisterY;
    SetFlag(Z, m_RegisterA == 0x00);
    SetFlag(N, m_RegisterA & 0x80);
    return false;
}
/*
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
*/
std::string Cpu::GetInstructionString(uint16_t opCodeAddress) {
    std::string str;

    std::optional<Instruction>& instr = m_InstrTable[Read(opCodeAddress)];
    if (instr.has_value()) {
        str += instr->m_Name;
        // todo fix
        // AppendAddressingModeString(opCodeAddress, instr->m_AddressingMode,
        // str);
    } else {
        str = "NOP [IMP]";
    }
    return str;
}

void Cpu::RegisterAllInstructionSet() {
    m_InstrTable.resize(0xFF, std::optional<Instruction>());

    m_InstrTable[0x00].emplace("BRK", &Cpu::Instruction_BRK, nullptr, 7);
    m_InstrTable[0x01].emplace("ORA", &Cpu::Instruction_ORA,
                               &Cpu::IndexedIndirectAddressingX, 6);
    m_InstrTable[0x05].emplace("ORA", &Cpu::Instruction_ORA,
                               &Cpu::ZeroPageAddressing, 3);
    m_InstrTable[0x06].emplace("ASL", &Cpu::Instruction_ASL,
                               &Cpu::ZeroPageAddressing, 5);
    m_InstrTable[0x08].emplace("PHP", &Cpu::Instruction_PHP, nullptr, 3);
    m_InstrTable[0x09].emplace("ORA", &Cpu::Instruction_ORA,
                               &Cpu::ImmediateAddressing, 2);
    m_InstrTable[0x0A].emplace("ASL", &Cpu::Instruction_ASL_AcummAddr, nullptr,
                               2);
    m_InstrTable[0x0D].emplace("ORA", &Cpu::Instruction_ORA,
                               &Cpu::AbsoluteAddressing, 4);
    m_InstrTable[0x0E].emplace("ASL", &Cpu::Instruction_ASL,
                               &Cpu::AbsoluteAddressing, 6);
    m_InstrTable[0x10].emplace("BPL", &Cpu::Instruction_BPL,
                               &Cpu::RelativeAddressing, 2);
    m_InstrTable[0x11].emplace("ORA", &Cpu::Instruction_ORA,
                               &Cpu::IndirectIndexedAddressingY, 5);
    m_InstrTable[0x15].emplace("ORA", &Cpu::Instruction_ORA,
                               &Cpu::IndexedZeroPageAddressingX, 4);
    m_InstrTable[0x16].emplace("ASL", &Cpu::Instruction_ASL,
                               &Cpu::IndexedZeroPageAddressingX, 6);
    m_InstrTable[0x18].emplace("CLC", &Cpu::Instruction_CLC, nullptr, 2);
    m_InstrTable[0x19].emplace("ORA", &Cpu::Instruction_ORA,
                               &Cpu::IndexedAbsoluteAddressingY, 4);
    m_InstrTable[0x1D].emplace("ORA", &Cpu::Instruction_ORA,
                               &Cpu::IndexedAbsoluteAddressingX, 4);
    m_InstrTable[0x1E].emplace("ASL", &Cpu::Instruction_ASL,
                               &Cpu::IndexedAbsoluteAddressingX, 7);
    m_InstrTable[0x20].emplace("JSR", &Cpu::Instruction_JSR,
                               &Cpu::AbsoluteAddressing, 6);
    m_InstrTable[0x21].emplace("AND", &Cpu::Instruction_AND,
                               &Cpu::IndexedIndirectAddressingX, 6);
    m_InstrTable[0x24].emplace("BIT", &Cpu::Instruction_BIT,
                               &Cpu::ZeroPageAddressing, 3);
    m_InstrTable[0x25].emplace("AND", &Cpu::Instruction_AND,
                               &Cpu::ZeroPageAddressing, 3);
    m_InstrTable[0x26].emplace("ROL", &Cpu::Instruction_ROL,
                               &Cpu::ZeroPageAddressing, 5);
    m_InstrTable[0x28].emplace("PLP", &Cpu::Instruction_PLP, nullptr, 4);
    m_InstrTable[0x29].emplace("AND", &Cpu::Instruction_AND,
                               &Cpu::ImmediateAddressing, 2);
    m_InstrTable[0x2A].emplace("ROL", &Cpu::Instruction_ROL_AcummAddr, nullptr,
                               2);
    m_InstrTable[0x2C].emplace("BIT", &Cpu::Instruction_BIT,
                               &Cpu::AbsoluteAddressing, 4);
    m_InstrTable[0x2D].emplace("AND", &Cpu::Instruction_AND,
                               &Cpu::AbsoluteAddressing, 4);
    m_InstrTable[0x2E].emplace("ROL", &Cpu::Instruction_ROL,
                               &Cpu::AbsoluteAddressing, 6);
    m_InstrTable[0x30].emplace("BMI", &Cpu::Instruction_BMI,
                               &Cpu::RelativeAddressing, 2);
    m_InstrTable[0x31].emplace("AND", &Cpu::Instruction_AND,
                               &Cpu::IndirectIndexedAddressingY, 5);
    m_InstrTable[0x35].emplace("AND", &Cpu::Instruction_AND,
                               &Cpu::IndexedZeroPageAddressingX, 4);
    m_InstrTable[0x36].emplace("ROL", &Cpu::Instruction_ROL,
                               &Cpu::IndexedZeroPageAddressingX, 6);
    m_InstrTable[0x38].emplace("SEC", &Cpu::Instruction_SEC, nullptr, 2);
    m_InstrTable[0x39].emplace("AND", &Cpu::Instruction_AND,
                               &Cpu::IndexedAbsoluteAddressingY, 4);
    m_InstrTable[0x3D].emplace("AND", &Cpu::Instruction_AND,
                               &Cpu::IndexedAbsoluteAddressingX, 4);
    m_InstrTable[0x3E].emplace("ROL", &Cpu::Instruction_ROL,
                               &Cpu::IndexedAbsoluteAddressingX, 7);
    m_InstrTable[0x40].emplace("RTI", &Cpu::Instruction_RTI, nullptr, 6);
    m_InstrTable[0x41].emplace("EOR", &Cpu::Instruction_EOR,
                               &Cpu::IndexedIndirectAddressingX, 6);
    m_InstrTable[0x45].emplace("EOR", &Cpu::Instruction_EOR,
                               &Cpu::ZeroPageAddressing, 3);
    m_InstrTable[0x46].emplace("LSR", &Cpu::Instruction_LSR,
                               &Cpu::ZeroPageAddressing, 5);
    m_InstrTable[0x48].emplace("PHA", &Cpu::Instruction_PHA, nullptr, 3);
    m_InstrTable[0x49].emplace("EOR", &Cpu::Instruction_EOR,
                               &Cpu::ImmediateAddressing, 2);
    m_InstrTable[0x4A].emplace("LSR", &Cpu::Instruction_LSR_AcummAddr, nullptr,
                               2);
    m_InstrTable[0x4C].emplace("JMP", &Cpu::Instruction_JMP,
                               &Cpu::AbsoluteAddressing, 3);
    m_InstrTable[0x4D].emplace("EOR", &Cpu::Instruction_EOR,
                               &Cpu::AbsoluteAddressing, 4);
    m_InstrTable[0x4E].emplace("LSR", &Cpu::Instruction_LSR,
                               &Cpu::AbsoluteAddressing, 6);
    m_InstrTable[0x50].emplace("BVC", &Cpu::Instruction_BVC,
                               &Cpu::RelativeAddressing, 2);
    m_InstrTable[0x51].emplace("EOR", &Cpu::Instruction_EOR,
                               &Cpu::IndirectIndexedAddressingY, 5);
    m_InstrTable[0x55].emplace("EOR", &Cpu::Instruction_EOR,
                               &Cpu::IndexedZeroPageAddressingX, 4);
    m_InstrTable[0x56].emplace("LSR", &Cpu::Instruction_LSR,
                               &Cpu::IndexedZeroPageAddressingX, 6);
    m_InstrTable[0x58].emplace("CLI", &Cpu::Instruction_CLI, nullptr, 2);
    m_InstrTable[0x59].emplace("EOR", &Cpu::Instruction_EOR,
                               &Cpu::IndexedAbsoluteAddressingY, 4);
    m_InstrTable[0x5D].emplace("EOR", &Cpu::Instruction_EOR,
                               &Cpu::IndexedAbsoluteAddressingX, 4);
    m_InstrTable[0x5E].emplace("LSR", &Cpu::Instruction_LSR,
                               &Cpu::IndexedAbsoluteAddressingX, 7);
    m_InstrTable[0x60].emplace("RTS", &Cpu::Instruction_RTS, nullptr, 6);
    m_InstrTable[0x61].emplace("ADC", &Cpu::Instruction_ADC,
                               &Cpu::IndexedIndirectAddressingX, 6);
    m_InstrTable[0x65].emplace("ADC", &Cpu::Instruction_ADC,
                               &Cpu::ZeroPageAddressing, 3);
    m_InstrTable[0x66].emplace("ROR", &Cpu::Instruction_ROR,
                               &Cpu::ZeroPageAddressing, 5);
    m_InstrTable[0x68].emplace("PLA", &Cpu::Instruction_PLA, nullptr, 4);
    m_InstrTable[0x69].emplace("ADC", &Cpu::Instruction_ADC,
                               &Cpu::ImmediateAddressing, 2);
    m_InstrTable[0x6A].emplace("ROR", &Cpu::Instruction_ROR_AcummAddr, nullptr,
                               2);
    m_InstrTable[0x6C].emplace("JMP", &Cpu::Instruction_JMP,
                               &Cpu::AbsoluteIndirectAddressing, 5);
    m_InstrTable[0x6D].emplace("ADC", &Cpu::Instruction_ADC,
                               &Cpu::AbsoluteAddressing, 4);
    m_InstrTable[0x6E].emplace("ROR", &Cpu::Instruction_ROR,
                               &Cpu::AbsoluteAddressing, 6);
    m_InstrTable[0x70].emplace("BVS", &Cpu::Instruction_BVS,
                               &Cpu::RelativeAddressing, 2);
    m_InstrTable[0x71].emplace("ADC", &Cpu::Instruction_ADC,
                               &Cpu::IndirectIndexedAddressingY, 5);
    m_InstrTable[0x75].emplace("ADC", &Cpu::Instruction_ADC,
                               &Cpu::IndexedZeroPageAddressingX, 4);
    m_InstrTable[0x76].emplace("ROR", &Cpu::Instruction_ROR,
                               &Cpu::IndexedZeroPageAddressingX, 6);
    m_InstrTable[0x78].emplace("SEI", &Cpu::Instruction_SEI, nullptr, 2);
    m_InstrTable[0x79].emplace("ADC", &Cpu::Instruction_ADC,
                               &Cpu::IndexedAbsoluteAddressingY, 4);
    m_InstrTable[0x7D].emplace("ADC", &Cpu::Instruction_ADC,
                               &Cpu::IndexedAbsoluteAddressingX, 4);
    m_InstrTable[0x7E].emplace("ROR", &Cpu::Instruction_ROR,
                               &Cpu::IndexedAbsoluteAddressingX, 7);
    m_InstrTable[0x81].emplace("STA", &Cpu::Instruction_STA,
                               &Cpu::IndexedIndirectAddressingX, 6);
    m_InstrTable[0x84].emplace("STY", &Cpu::Instruction_STY,
                               &Cpu::ZeroPageAddressing, 3);
    m_InstrTable[0x85].emplace("STA", &Cpu::Instruction_STA,
                               &Cpu::ZeroPageAddressing, 3);
    m_InstrTable[0x86].emplace("STX", &Cpu::Instruction_STX,
                               &Cpu::ZeroPageAddressing, 3);
    m_InstrTable[0x88].emplace("DEY", &Cpu::Instruction_DEY, nullptr, 2);
    m_InstrTable[0x8A].emplace("TXA", &Cpu::Instruction_TXA, nullptr, 2);
    m_InstrTable[0x8C].emplace("STY", &Cpu::Instruction_STY,
                               &Cpu::AbsoluteAddressing, 4);
    m_InstrTable[0x8D].emplace("STA", &Cpu::Instruction_STA,
                               &Cpu::AbsoluteAddressing, 4);
    m_InstrTable[0x8E].emplace("STX", &Cpu::Instruction_STX,
                               &Cpu::AbsoluteAddressing, 4);
    m_InstrTable[0x90].emplace("BCC", &Cpu::Instruction_BCC,
                               &Cpu::RelativeAddressing, 2);
    m_InstrTable[0x91].emplace("STA", &Cpu::Instruction_STA,
                               &Cpu::IndirectIndexedAddressingY, 6);
    m_InstrTable[0x94].emplace("STY", &Cpu::Instruction_STY,
                               &Cpu::IndexedZeroPageAddressingX, 4);
    m_InstrTable[0x95].emplace("STA", &Cpu::Instruction_STA,
                               &Cpu::IndexedZeroPageAddressingX, 4);
    m_InstrTable[0x96].emplace("STX", &Cpu::Instruction_STX,
                               &Cpu::IndexedZeroPageAddressingY, 4);
    m_InstrTable[0x98].emplace("TYA", &Cpu::Instruction_TYA, nullptr, 2);
    m_InstrTable[0x99].emplace("STA", &Cpu::Instruction_STA,
                               &Cpu::IndexedAbsoluteAddressingY, 5);
    m_InstrTable[0x9A].emplace("TXS", &Cpu::Instruction_TXS, nullptr, 2);
    m_InstrTable[0x9D].emplace("STA", &Cpu::Instruction_STA,
                               &Cpu::IndexedAbsoluteAddressingX, 5);
    m_InstrTable[0xA0].emplace("LDY", &Cpu::Instruction_LDY,
                               &Cpu::ImmediateAddressing, 2);
    m_InstrTable[0xA1].emplace("LDA", &Cpu::Instruction_LDA,
                               &Cpu::IndexedIndirectAddressingX, 6);
    m_InstrTable[0xA2].emplace("LDX", &Cpu::Instruction_LDX,
                               &Cpu::ImmediateAddressing, 2);
    m_InstrTable[0xA4].emplace("LDY", &Cpu::Instruction_LDY,
                               &Cpu::ZeroPageAddressing, 3);
    m_InstrTable[0xA5].emplace("LDA", &Cpu::Instruction_LDA,
                               &Cpu::ZeroPageAddressing, 3);
    m_InstrTable[0xA6].emplace("LDX", &Cpu::Instruction_LDX,
                               &Cpu::ZeroPageAddressing, 3);
    m_InstrTable[0xA8].emplace("TAY", &Cpu::Instruction_TAY, nullptr, 2);
    m_InstrTable[0xA9].emplace("LDA", &Cpu::Instruction_LDA,
                               &Cpu::ImmediateAddressing, 2);
    m_InstrTable[0xAA].emplace("TAX", &Cpu::Instruction_TAX, nullptr, 2);
    m_InstrTable[0xAC].emplace("LDY", &Cpu::Instruction_LDY,
                               &Cpu::AbsoluteAddressing, 4);
    m_InstrTable[0xAD].emplace("LDA", &Cpu::Instruction_LDA,
                               &Cpu::AbsoluteAddressing, 4);
    m_InstrTable[0xAE].emplace("LDX", &Cpu::Instruction_LDX,
                               &Cpu::AbsoluteAddressing, 4);
    m_InstrTable[0xB0].emplace("BCS", &Cpu::Instruction_BCS,
                               &Cpu::RelativeAddressing, 2);
    m_InstrTable[0xB1].emplace("LDA", &Cpu::Instruction_LDA,
                               &Cpu::IndirectIndexedAddressingY, 5);
    m_InstrTable[0xB4].emplace("LDY", &Cpu::Instruction_LDY,
                               &Cpu::IndexedZeroPageAddressingX, 4);
    m_InstrTable[0xB5].emplace("LDA", &Cpu::Instruction_LDA,
                               &Cpu::IndexedZeroPageAddressingX, 4);
    m_InstrTable[0xB6].emplace("LDX", &Cpu::Instruction_LDX,
                               &Cpu::IndexedZeroPageAddressingY, 4);
    m_InstrTable[0xB8].emplace("CLV", &Cpu::Instruction_CLV, nullptr, 2);
    m_InstrTable[0xB9].emplace("LDA", &Cpu::Instruction_LDA,
                               &Cpu::IndexedAbsoluteAddressingY, 4);
    m_InstrTable[0xBA].emplace("TSX", &Cpu::Instruction_TSX, nullptr, 2);
    m_InstrTable[0xBC].emplace("LDY", &Cpu::Instruction_LDY,
                               &Cpu::IndexedAbsoluteAddressingX, 4);
    m_InstrTable[0xBD].emplace("LDA", &Cpu::Instruction_LDA,
                               &Cpu::IndexedAbsoluteAddressingX, 4);
    m_InstrTable[0xBE].emplace("LDX", &Cpu::Instruction_LDX,
                               &Cpu::IndexedAbsoluteAddressingY, 4);
    m_InstrTable[0xC0].emplace("CPY", &Cpu::Instruction_CPY,
                               &Cpu::ImmediateAddressing, 2);
    m_InstrTable[0xC1].emplace("CMP", &Cpu::Instruction_CMP,
                               &Cpu::IndexedIndirectAddressingX, 6);
    m_InstrTable[0xC4].emplace("CPY", &Cpu::Instruction_CPY,
                               &Cpu::ZeroPageAddressing, 3);
    m_InstrTable[0xC5].emplace("CMP", &Cpu::Instruction_CMP,
                               &Cpu::ZeroPageAddressing, 3);
    m_InstrTable[0xC6].emplace("DEC", &Cpu::Instruction_DEC,
                               &Cpu::ZeroPageAddressing, 5);
    m_InstrTable[0xC8].emplace("INY", &Cpu::Instruction_INY, nullptr, 2);
    m_InstrTable[0xC9].emplace("CMP", &Cpu::Instruction_CMP,
                               &Cpu::ImmediateAddressing, 2);
    m_InstrTable[0xCA].emplace("DEX", &Cpu::Instruction_DEX, nullptr, 2);
    m_InstrTable[0xCC].emplace("CPY", &Cpu::Instruction_CPY,
                               &Cpu::AbsoluteAddressing, 4);
    m_InstrTable[0xCD].emplace("CMP", &Cpu::Instruction_CMP,
                               &Cpu::AbsoluteAddressing, 4);
    m_InstrTable[0xCE].emplace("DEC", &Cpu::Instruction_DEC,
                               &Cpu::AbsoluteAddressing, 6);
    m_InstrTable[0xD0].emplace("BNE", &Cpu::Instruction_BNE,
                               &Cpu::RelativeAddressing, 2);
    m_InstrTable[0xD1].emplace("CMP", &Cpu::Instruction_CMP,
                               &Cpu::IndirectIndexedAddressingY, 5);
    m_InstrTable[0xD5].emplace("CMP", &Cpu::Instruction_CMP,
                               &Cpu::IndexedZeroPageAddressingX, 4);
    m_InstrTable[0xD6].emplace("DEC", &Cpu::Instruction_DEC,
                               &Cpu::IndexedZeroPageAddressingX, 6);
    m_InstrTable[0xD8].emplace("CLD", &Cpu::Instruction_CLD, nullptr, 2);
    m_InstrTable[0xD9].emplace("CMP", &Cpu::Instruction_CMP,
                               &Cpu::IndexedAbsoluteAddressingY, 4);
    m_InstrTable[0xDD].emplace("CMP", &Cpu::Instruction_CMP,
                               &Cpu::IndexedAbsoluteAddressingX, 4);
    m_InstrTable[0xDE].emplace("DEC", &Cpu::Instruction_DEC,
                               &Cpu::IndexedAbsoluteAddressingX, 7);
    m_InstrTable[0xE0].emplace("CPX", &Cpu::Instruction_CPX,
                               &Cpu::ImmediateAddressing, 2);
    m_InstrTable[0xE1].emplace("SBC", &Cpu::Instruction_SBC,
                               &Cpu::IndexedIndirectAddressingX, 6);
    m_InstrTable[0xE4].emplace("CPX", &Cpu::Instruction_CPX,
                               &Cpu::ZeroPageAddressing, 3);
    m_InstrTable[0xE5].emplace("SBC", &Cpu::Instruction_SBC,
                               &Cpu::ZeroPageAddressing, 3);
    m_InstrTable[0xE6].emplace("INC", &Cpu::Instruction_INC,
                               &Cpu::ZeroPageAddressing, 5);
    m_InstrTable[0xE8].emplace("INX", &Cpu::Instruction_INX, nullptr, 2);
    m_InstrTable[0xE9].emplace("SBC", &Cpu::Instruction_SBC,
                               &Cpu::ImmediateAddressing, 2);
    m_InstrTable[0xEA].emplace("NOP", &Cpu::Instruction_NOP, nullptr, 2);
    m_InstrTable[0xEC].emplace("CPX", &Cpu::Instruction_CPX,
                               &Cpu::AbsoluteAddressing, 4);
    m_InstrTable[0xED].emplace("SBC", &Cpu::Instruction_SBC,
                               &Cpu::AbsoluteAddressing, 4);
    m_InstrTable[0xEE].emplace("INC", &Cpu::Instruction_INC,
                               &Cpu::AbsoluteAddressing, 6);
    m_InstrTable[0xF0].emplace("BEQ", &Cpu::Instruction_BEQ,
                               &Cpu::RelativeAddressing, 2);
    m_InstrTable[0xF1].emplace("SBC", &Cpu::Instruction_SBC,
                               &Cpu::IndirectIndexedAddressingY, 5);
    m_InstrTable[0xF5].emplace("SBC", &Cpu::Instruction_SBC,
                               &Cpu::IndexedZeroPageAddressingX, 4);
    m_InstrTable[0xF6].emplace("INC", &Cpu::Instruction_INC,
                               &Cpu::IndexedZeroPageAddressingX, 6);
    m_InstrTable[0xF8].emplace("SED", &Cpu::Instruction_SED, nullptr, 2);
    m_InstrTable[0xF9].emplace("SBC", &Cpu::Instruction_SBC,
                               &Cpu::IndexedAbsoluteAddressingY, 4);
    m_InstrTable[0xFD].emplace("SBC", &Cpu::Instruction_SBC,
                               &Cpu::IndexedAbsoluteAddressingX, 4);
    m_InstrTable[0xFE].emplace("INC", &Cpu::Instruction_INC,
                               &Cpu::IndexedAbsoluteAddressingX, 7);
}
}  // namespace cpuemulator

#pragma endregion INSTRUCTIONS
