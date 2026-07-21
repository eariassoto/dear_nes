// Copyright (c) 2020 Emmanuel Arias
#include "dear_nes_lib/cpu.h"

#include <cassert>

#include "dear_nes_lib/bus.h"

namespace dearnes {

void Cpu::SetBus(Bus* bus) {
    assert(bus != nullptr);
    m_Bus = bus;
}

void Cpu::Reset() {
    m_RegisterA = 0;
    m_RegisterX = 0;
    m_RegisterY = 0;
    m_StatusRegister = 0x00 | CpuFlag::U;

    constexpr uint16_t addressToReadPC = 0xFFFC;
    uint16_t lo = m_Bus->CpuRead(addressToReadPC);
    uint16_t hi = m_Bus->CpuRead(addressToReadPC + 1);
    m_ProgramCounter = (hi << 8) | lo;

    m_StackPointer = 0xFD;

    m_Cycles = 8;
}

void Cpu::Clock() {
    if (m_Cycles == 0) {
        m_OpCode = ReadWordFromProgramCounter();

        SetFlag(CpuFlag::U, 1);

        // TODO: Catch exception illegal instruction
        const Instruction& instr = FindInstruction(m_OpCode);

        m_Cycles = instr.m_Cycles;

        if (instr.m_ExecureAddressingMode) {
            (this->*instr.m_ExecureAddressingMode)();
        }

        assert(instr.m_ExecuteInstruction != nullptr);
        (this->*instr.m_ExecuteInstruction)();

        if (m_AddressingModeNeedsAdditionalCycle &&
            m_InstructionNeedsAdditionalCycle) {
            ++m_Cycles;
        }

        SetFlag(U, true);
    }

    m_Cycles--;
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

uint8_t Cpu::Read(uint16_t address) { return m_Bus->CpuRead(address); }

void Cpu::Write(uint16_t address, uint8_t data) {
    m_Bus->CpuWrite(address, data);
}

uint8_t Cpu::ReadWordFromProgramCounter() {
    return m_Bus->CpuRead(m_ProgramCounter++);
}

uint16_t Cpu::ReadDoubleWordFromProgramCounter() {
    uint16_t lowNibble = ReadWordFromProgramCounter();
    uint16_t highNibble = ReadWordFromProgramCounter();

    return (highNibble << 8) | lowNibble;
}

void Cpu::AddrImmediate() { m_AddressAbsolute = m_ProgramCounter++; }

void Cpu::AddrZeroPage() {
    m_AddressAbsolute = ReadWordFromProgramCounter();
    m_AddressAbsolute &= 0x00FF;
}

void Cpu::AddrIndexedZeroPageX() {
    m_AddressAbsolute = ReadWordFromProgramCounter();
    m_AddressAbsolute += m_RegisterX;
    m_AddressAbsolute &= 0x00FF;
}

void Cpu::AddrIndexedZeroPageY() {
    m_AddressAbsolute = ReadWordFromProgramCounter();
    m_AddressAbsolute += m_RegisterY;
    m_AddressAbsolute &= 0x00FF;
}

void Cpu::AddrAbsolute() {
    m_AddressAbsolute = ReadDoubleWordFromProgramCounter();
}

void Cpu::AddrIndexedAbsoluteX() {
    m_AddressAbsolute = ReadDoubleWordFromProgramCounter();
    uint16_t highNibble = m_AddressAbsolute & 0xFF00;

    m_AddressAbsolute += m_RegisterX;

    m_AddressingModeNeedsAdditionalCycle =
        (m_AddressAbsolute & 0xFF00) != highNibble;
}

void Cpu::AddrIndexedAbsoluteY() {
    m_AddressAbsolute = ReadDoubleWordFromProgramCounter();
    uint16_t highNibble = m_AddressAbsolute & 0xFF00;

    m_AddressAbsolute += m_RegisterY;

    m_AddressingModeNeedsAdditionalCycle =
        (m_AddressAbsolute & 0xFF00) != highNibble;
}

void Cpu::AddrAbsoluteIndirect() {
    uint16_t pointer = ReadDoubleWordFromProgramCounter();

    // Simulate page boundary hardware bug
    if ((pointer & 0x00FF) == 0x00FF) {
        m_AddressAbsolute = (Read(pointer & 0xFF00) << 8) | Read(pointer);
    } else {
        m_AddressAbsolute = (Read(pointer + 1) << 8) | Read(pointer);
    }
}

void Cpu::AddrIndexedIndirectX() {
    uint16_t t = ReadWordFromProgramCounter();

    uint16_t registerXValue = static_cast<uint16_t>(m_RegisterX);

    uint16_t lowNibbleAddress = (t + registerXValue) & 0x00FF;
    uint16_t lowNibble = Read(lowNibbleAddress);

    uint16_t highNibbleAddress = (t + registerXValue + 1) & 0x00FF;
    uint16_t highNibble = Read(highNibbleAddress);

    m_AddressAbsolute = (highNibble << 8) | lowNibble;
}

void Cpu::AddrIndirectIndexedY() {
    uint16_t t = ReadWordFromProgramCounter();

    uint16_t lowNibble = Read(t & 0x00FF);
    uint16_t highNibble = Read((t + 1) & 0x00FF);

    m_AddressAbsolute = (highNibble << 8) | lowNibble;
    m_AddressAbsolute += m_RegisterY;

    m_AddressingModeNeedsAdditionalCycle =
        (m_AddressAbsolute & 0xFF00) != (highNibble << 8);
}

void Cpu::AddrRelative() {
    m_AddressRelative = ReadWordFromProgramCounter();

    if (m_AddressRelative & 0x80)  // if unsigned
    {
        m_AddressRelative |= 0xFF00;
    }
}

void Cpu::InstrADC() {
    uint8_t valueFetched = Read(m_AddressAbsolute);

    uint16_t castedFetched = static_cast<uint16_t>(valueFetched);
    uint16_t castedCarry = static_cast<uint16_t>(GetFlag(CpuFlag::C));
    uint16_t castedAccum = static_cast<uint16_t>(m_RegisterA);

    uint16_t temp = castedAccum + castedFetched + castedCarry;
    SetFlag(CpuFlag::C, temp > 255);
    SetFlag(CpuFlag::Z, (temp & 0x00FF) == 0);
    SetFlag(CpuFlag::N, temp & 0x80);
    SetFlag(V,
            (~(castedAccum ^ castedFetched) & (castedAccum ^ temp)) & 0x0080);

    m_RegisterA = temp & 0x00FF;

    m_InstructionNeedsAdditionalCycle = true;
}

void Cpu::InstrAND() {
    uint8_t valueFetched = Read(m_AddressAbsolute);

    m_RegisterA &= valueFetched;
    SetFlag(CpuFlag::Z, m_RegisterA == 0x00);
    SetFlag(CpuFlag::N, m_RegisterA & 0x80);

    m_InstructionNeedsAdditionalCycle = true;
}

void Cpu::InstrASL() {
    uint8_t valueFetched = Read(m_AddressAbsolute);

    uint16_t temp = static_cast<uint16_t>(valueFetched) << 1;
    SetFlag(CpuFlag::C, (temp & 0xFF00) > 0);
    SetFlag(CpuFlag::Z, (temp & 0x00FF) == 0);
    SetFlag(CpuFlag::N, temp & 0x80);

    Write(m_AddressAbsolute, temp & 0x00FF);
}

void Cpu::InstrASL_AcummAddr() {
    uint16_t temp = static_cast<uint16_t>(m_RegisterA) << 1;
    SetFlag(CpuFlag::C, (temp & 0xFF00) > 0);
    SetFlag(CpuFlag::Z, (temp & 0x00FF) == 0);
    SetFlag(CpuFlag::N, temp & 0x80);

    m_RegisterA = temp & 0x00FF;
}

void Cpu::InstrExecuteBranch() {
    m_Cycles++;
    m_AddressAbsolute = m_ProgramCounter + m_AddressRelative;

    if ((m_AddressAbsolute & 0xFF00) != (m_ProgramCounter & 0xFF00)) {
        m_Cycles++;
    }
    m_ProgramCounter = m_AddressAbsolute;
}

void Cpu::InstrBCC() {
    if (GetFlag(CpuFlag::C) == 0) {
        InstrExecuteBranch();
    }
}

void Cpu::InstrBCS() {
    if (GetFlag(CpuFlag::C) == 1) {
        InstrExecuteBranch();
    }
}

void Cpu::InstrBEQ() {
    if (GetFlag(CpuFlag::Z) == 1) {
        InstrExecuteBranch();
    }
}

void Cpu::InstrBIT() {
    uint8_t valueFetched = Read(m_AddressAbsolute);

    uint16_t temp = m_RegisterA & valueFetched;
    SetFlag(Z, (temp & 0x00FF) == 0x00);
    SetFlag(N, valueFetched & (1 << 7));
    SetFlag(V, valueFetched & (1 << 6));
}

void Cpu::InstrBMI() {
    if (GetFlag(CpuFlag::N) == 1) {
        InstrExecuteBranch();
    }
}

void Cpu::InstrBNE() {
    if (GetFlag(CpuFlag::Z) == 0) {
        InstrExecuteBranch();
    }
}

void Cpu::InstrBPL() {
    if (GetFlag(CpuFlag::N) == 0) {
        InstrExecuteBranch();
    }
}

void Cpu::InstrBRK() {
    m_ProgramCounter++;

    SetFlag(I, 1);
    Write(0x0100 + m_StackPointer, (m_ProgramCounter >> 8) & 0x00FF);
    m_StackPointer--;
    Write(0x0100 + m_StackPointer, m_ProgramCounter & 0x00FF);
    m_StackPointer--;

    SetFlag(CpuFlag::B, 1);
    Write(0x0100 + m_StackPointer, m_StatusRegister);
    m_StackPointer--;
    SetFlag(B, 0);

    m_ProgramCounter = static_cast<uint16_t>(Read(0xFFFE)) |
                       (static_cast<uint16_t>(Read(0xFFFF)) << 8);
}

void Cpu::InstrBVC() {
    if (GetFlag(CpuFlag::V) == 0) {
        InstrExecuteBranch();
    }
}

void Cpu::InstrBVS() {
    if (GetFlag(CpuFlag::V) == 1) {
        InstrExecuteBranch();
    }
}

void Cpu::InstrCLC() { SetFlag(CpuFlag::C, false); }

void Cpu::InstrCLD() { SetFlag(CpuFlag::D, false); }

void Cpu::InstrCLI() { SetFlag(CpuFlag::I, false); }

void Cpu::InstrCLV() { SetFlag(CpuFlag::V, false); }

void Cpu::InstrCMP() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    uint16_t temp = static_cast<uint16_t>(m_RegisterA) -
                    static_cast<uint16_t>(valueFetched);
    SetFlag(C, m_RegisterA >= valueFetched);
    SetFlag(Z, (temp & 0x00FF) == 0x0000);
    SetFlag(N, temp & 0x0080);
    m_InstructionNeedsAdditionalCycle = true;
}

void Cpu::InstrCPX() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    uint16_t temp = static_cast<uint16_t>(m_RegisterX) -
                    static_cast<uint16_t>(valueFetched);
    SetFlag(C, m_RegisterX >= valueFetched);
    SetFlag(Z, (temp & 0x00FF) == 0x0000);
    SetFlag(N, temp & 0x0080);
}

void Cpu::InstrCPY() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    uint16_t temp = static_cast<uint16_t>(m_RegisterY) -
                    static_cast<uint16_t>(valueFetched);
    SetFlag(C, m_RegisterY >= valueFetched);
    SetFlag(Z, (temp & 0x00FF) == 0x0000);
    SetFlag(N, temp & 0x0080);
}

void Cpu::InstrDEC() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    uint16_t temp = valueFetched - 1;
    Write(m_AddressAbsolute, temp & 0x00FF);
    SetFlag(Z, (temp & 0x00FF) == 0x0000);
    SetFlag(N, temp & 0x0080);
}

void Cpu::InstrDEX() {
    m_RegisterX--;
    SetFlag(Z, m_RegisterX == 0x00);
    SetFlag(N, m_RegisterX & 0x80);
}

void Cpu::InstrDEY() {
    m_RegisterY--;
    SetFlag(Z, m_RegisterY == 0x00);
    SetFlag(N, m_RegisterY & 0x80);
}

void Cpu::InstrEOR() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    m_RegisterA = m_RegisterA ^ valueFetched;
    SetFlag(Z, m_RegisterA == 0x00);
    SetFlag(N, m_RegisterA & 0x80);
    m_InstructionNeedsAdditionalCycle = true;
}

void Cpu::InstrINC() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    uint16_t temp = valueFetched + 1;
    Write(m_AddressAbsolute, temp & 0x00FF);
    SetFlag(Z, (temp & 0x00FF) == 0x0000);
    SetFlag(N, temp & 0x0080);
}

void Cpu::InstrINX() {
    m_RegisterX++;
    SetFlag(Z, m_RegisterX == 0x00);
    SetFlag(N, m_RegisterX & 0x80);
}

void Cpu::InstrINY() {
    m_RegisterY++;
    SetFlag(Z, m_RegisterY == 0x00);
    SetFlag(N, m_RegisterY & 0x80);
}

void Cpu::InstrJMP() { m_ProgramCounter = m_AddressAbsolute; }

void Cpu::InstrJSR() {
    m_ProgramCounter--;

    Write(0x0100 + m_StackPointer, (m_ProgramCounter >> 8) & 0x00FF);
    m_StackPointer--;
    Write(0x0100 + m_StackPointer, m_ProgramCounter & 0x00FF);
    m_StackPointer--;

    m_ProgramCounter = m_AddressAbsolute;
}

void Cpu::InstrLDA() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    m_RegisterA = valueFetched;
    SetFlag(Z, m_RegisterA == 0x00);
    SetFlag(N, m_RegisterA & 0x80);
    m_InstructionNeedsAdditionalCycle = true;
}

void Cpu::InstrLDX() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    m_RegisterX = valueFetched;
    SetFlag(Z, m_RegisterX == 0x00);
    SetFlag(N, m_RegisterX & 0x80);
    m_InstructionNeedsAdditionalCycle = true;
}

void Cpu::InstrLDY() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    m_RegisterY = valueFetched;
    SetFlag(Z, m_RegisterY == 0x00);
    SetFlag(N, m_RegisterY & 0x80);
    m_InstructionNeedsAdditionalCycle = true;
}

void Cpu::InstrLSR() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    SetFlag(C, valueFetched & 0x0001);
    uint16_t temp = valueFetched >> 1;
    SetFlag(Z, (temp & 0x00FF) == 0x0000);
    SetFlag(N, temp & 0x0080);

    Write(m_AddressAbsolute, temp & 0x00FF);
}

void Cpu::InstrLSR_AcummAddr() {
    SetFlag(C, m_RegisterA & 0x0001);
    uint16_t temp = m_RegisterA >> 1;
    SetFlag(Z, (temp & 0x00FF) == 0x0000);
    SetFlag(N, temp & 0x0080);

    m_RegisterA = temp & 0x00FF;
}

void Cpu::InstrNOP() {}

void Cpu::InstrORA() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    m_RegisterA = m_RegisterA | valueFetched;
    SetFlag(Z, m_RegisterA == 0x00);
    SetFlag(N, m_RegisterA & 0x80);
    m_InstructionNeedsAdditionalCycle = true;
}

void Cpu::InstrPHA() {
    Write(0x0100 + m_StackPointer, m_RegisterA);
    m_StackPointer--;
}

void Cpu::InstrPHP() {
    Write(0x0100 + m_StackPointer, m_StatusRegister | B | U);
    SetFlag(B, 0);
    SetFlag(U, 0);
    m_StackPointer--;
}

void Cpu::InstrPLA() {
    m_StackPointer++;
    m_RegisterA = Read(0x0100 + m_StackPointer);
    SetFlag(Z, m_RegisterA == 0x00);
    SetFlag(N, m_RegisterA & 0x80);
}

void Cpu::InstrPLP() {
    m_StackPointer++;
    m_StatusRegister = Read(0x0100 + m_StackPointer);
    SetFlag(U, 1);
}

void Cpu::InstrROL() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    uint16_t temp = static_cast<uint16_t>(valueFetched << 1) | GetFlag(C);
    SetFlag(C, temp & 0xFF00);
    SetFlag(Z, (temp & 0x00FF) == 0x0000);
    SetFlag(N, temp & 0x0080);

    Write(m_AddressAbsolute, temp & 0x00FF);
}

void Cpu::InstrROL_AcummAddr() {
    uint16_t temp = static_cast<uint16_t>(m_RegisterA << 1) | GetFlag(C);
    SetFlag(C, temp & 0xFF00);
    SetFlag(Z, (temp & 0x00FF) == 0x0000);
    SetFlag(N, temp & 0x0080);

    m_RegisterA = temp & 0x00FF;
}

void Cpu::InstrROR() {
    uint8_t valueFetched = Read(m_AddressAbsolute);
    uint16_t temp =
        static_cast<uint16_t>(GetFlag(C) << 7) | (valueFetched >> 1);
    SetFlag(C, valueFetched & 0x01);
    SetFlag(Z, (temp & 0x00FF) == 0x00);
    SetFlag(N, temp & 0x0080);

    Write(m_AddressAbsolute, temp & 0x00FF);
}
void Cpu::InstrROR_AcummAddr() {
    uint16_t temp = static_cast<uint16_t>(GetFlag(C) << 7) | (m_RegisterA >> 1);
    SetFlag(C, m_RegisterA & 0x01);
    SetFlag(Z, (temp & 0x00FF) == 0x00);
    SetFlag(N, temp & 0x0080);

    m_RegisterA = temp & 0x00FF;
}

void Cpu::InstrRTI() {
    m_StackPointer++;
    m_StatusRegister = Read(0x0100 + m_StackPointer);
    m_StatusRegister &= ~B;
    m_StatusRegister &= ~U;

    m_StackPointer++;
    m_ProgramCounter = static_cast<uint16_t>(Read(0x0100 + m_StackPointer));
    m_StackPointer++;
    m_ProgramCounter |= static_cast<uint16_t>(Read(0x0100 + m_StackPointer))
                        << 8;
}

void Cpu::InstrRTS() {
    m_StackPointer++;
    m_ProgramCounter = static_cast<uint16_t>(Read(0x0100 + m_StackPointer));
    m_StackPointer++;
    m_ProgramCounter |= static_cast<uint16_t>(Read(0x0100 + m_StackPointer))
                        << 8;

    m_ProgramCounter++;
}

void Cpu::InstrSBC() {
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
    m_InstructionNeedsAdditionalCycle = true;
}

void Cpu::InstrSEC() { SetFlag(C, true); }

void Cpu::InstrSED() { SetFlag(D, true); }

void Cpu::InstrSEI() { SetFlag(I, true); }

void Cpu::InstrSTA() { Write(m_AddressAbsolute, m_RegisterA); }

void Cpu::InstrSTX() { Write(m_AddressAbsolute, m_RegisterX); }

void Cpu::InstrSTY() { Write(m_AddressAbsolute, m_RegisterY); }

void Cpu::InstrTAX() {
    m_RegisterX = m_RegisterA;
    SetFlag(Z, m_RegisterX == 0x00);
    SetFlag(N, m_RegisterX & 0x80);
}

void Cpu::InstrTAY() {
    m_RegisterY = m_RegisterA;
    SetFlag(Z, m_RegisterY == 0x00);
    SetFlag(N, m_RegisterY & 0x80);
}

void Cpu::InstrTSX() {
    m_RegisterX = m_StackPointer;
    SetFlag(Z, m_RegisterX == 0x00);
    SetFlag(N, m_RegisterX & 0x80);
}

void Cpu::InstrTXA() {
    m_RegisterA = m_RegisterX;
    SetFlag(Z, m_RegisterA == 0x00);
    SetFlag(N, m_RegisterA & 0x80);
}

void Cpu::InstrTXS() { m_StackPointer = m_RegisterX; }

void Cpu::InstrTYA() {
    m_RegisterA = m_RegisterY;
    SetFlag(Z, m_RegisterA == 0x00);
    SetFlag(N, m_RegisterA & 0x80);
}

void Cpu::InstrNoImpl() { return; }

Cpu::Instruction Cpu::m_InstructionTable[0x100] = {
    Instruction{&Cpu::InstrBRK, nullptr, 7},                     // 0x00
    Instruction{&Cpu::InstrORA, &Cpu::AddrIndexedIndirectX, 6},  // 0x01
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x02
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x03
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x04
    Instruction{&Cpu::InstrORA, &Cpu::AddrZeroPage, 3},          // 0x05
    Instruction{&Cpu::InstrASL, &Cpu::AddrZeroPage, 5},          // 0x06
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x07
    Instruction{&Cpu::InstrPHP, nullptr, 3},                     // 0x08
    Instruction{&Cpu::InstrORA, &Cpu::AddrImmediate, 2},         // 0x09
    Instruction{&Cpu::InstrASL_AcummAddr, nullptr, 2},           // 0x0A
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x0B
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x0C
    Instruction{&Cpu::InstrORA, &Cpu::AddrAbsolute, 4},          // 0x0D
    Instruction{&Cpu::InstrASL, &Cpu::AddrAbsolute, 6},          // 0x0E
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x0F
    Instruction{&Cpu::InstrBPL, &Cpu::AddrRelative, 2},          // 0x10
    Instruction{&Cpu::InstrORA, &Cpu::AddrIndirectIndexedY, 5},  // 0x11
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x12
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x13
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x14
    Instruction{&Cpu::InstrORA, &Cpu::AddrIndexedZeroPageX, 4},  // 0x15
    Instruction{&Cpu::InstrASL, &Cpu::AddrIndexedZeroPageX, 6},  // 0x16
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x17
    Instruction{&Cpu::InstrCLC, nullptr, 2},                     // 0x18
    Instruction{&Cpu::InstrORA, &Cpu::AddrIndexedAbsoluteY, 4},  // 0x19
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x1A
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x1B
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x1C
    Instruction{&Cpu::InstrORA, &Cpu::AddrIndexedAbsoluteX, 4},  // 0x1D
    Instruction{&Cpu::InstrASL, &Cpu::AddrIndexedAbsoluteX, 7},  // 0x1E
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x1F
    Instruction{&Cpu::InstrJSR, &Cpu::AddrAbsolute, 6},          // 0x20
    Instruction{&Cpu::InstrAND, &Cpu::AddrIndexedIndirectX, 6},  // 0x21
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x22
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x23
    Instruction{&Cpu::InstrBIT, &Cpu::AddrZeroPage, 3},          // 0x24
    Instruction{&Cpu::InstrAND, &Cpu::AddrZeroPage, 3},          // 0x25
    Instruction{&Cpu::InstrROL, &Cpu::AddrZeroPage, 5},          // 0x26
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x27
    Instruction{&Cpu::InstrPLP, nullptr, 4},                     // 0x28
    Instruction{&Cpu::InstrAND, &Cpu::AddrImmediate, 2},         // 0x29
    Instruction{&Cpu::InstrROL_AcummAddr, nullptr, 2},           // 0x2A
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x2B
    Instruction{&Cpu::InstrBIT, &Cpu::AddrAbsolute, 4},          // 0x2C
    Instruction{&Cpu::InstrAND, &Cpu::AddrAbsolute, 4},          // 0x2D
    Instruction{&Cpu::InstrROL, &Cpu::AddrAbsolute, 6},          // 0x2E
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x2F
    Instruction{&Cpu::InstrBMI, &Cpu::AddrRelative, 2},          // 0x30
    Instruction{&Cpu::InstrAND, &Cpu::AddrIndirectIndexedY, 5},  // 0x31
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x32
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x33
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x34
    Instruction{&Cpu::InstrAND, &Cpu::AddrIndexedZeroPageX, 4},  // 0x35
    Instruction{&Cpu::InstrROL, &Cpu::AddrIndexedZeroPageX, 6},  // 0x36
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x37
    Instruction{&Cpu::InstrSEC, nullptr, 2},                     // 0x38
    Instruction{&Cpu::InstrAND, &Cpu::AddrIndexedAbsoluteY, 4},  // 0x39
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x3A
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x3B
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x3C
    Instruction{&Cpu::InstrAND, &Cpu::AddrIndexedAbsoluteX, 4},  // 0x3D
    Instruction{&Cpu::InstrROL, &Cpu::AddrIndexedAbsoluteX, 7},  // 0x3E
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x3F
    Instruction{&Cpu::InstrRTI, nullptr, 6},                     // 0x40
    Instruction{&Cpu::InstrEOR, &Cpu::AddrIndexedIndirectX, 6},  // 0x41
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x42
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x43
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x44
    Instruction{&Cpu::InstrEOR, &Cpu::AddrZeroPage, 3},          // 0x45
    Instruction{&Cpu::InstrLSR, &Cpu::AddrZeroPage, 5},          // 0x46
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x47
    Instruction{&Cpu::InstrPHA, nullptr, 3},                     // 0x48
    Instruction{&Cpu::InstrEOR, &Cpu::AddrImmediate, 2},         // 0x49
    Instruction{&Cpu::InstrLSR_AcummAddr, nullptr, 2},           // 0x4A
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x4B
    Instruction{&Cpu::InstrJMP, &Cpu::AddrAbsolute, 3},          // 0x4C
    Instruction{&Cpu::InstrEOR, &Cpu::AddrAbsolute, 4},          // 0x4D
    Instruction{&Cpu::InstrLSR, &Cpu::AddrAbsolute, 6},          // 0x4E
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x4F
    Instruction{&Cpu::InstrBVC, &Cpu::AddrRelative, 2},          // 0x50
    Instruction{&Cpu::InstrEOR, &Cpu::AddrIndirectIndexedY, 5},  // 0x51
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x52
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x53
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x54
    Instruction{&Cpu::InstrEOR, &Cpu::AddrIndexedZeroPageX, 4},  // 0x55
    Instruction{&Cpu::InstrLSR, &Cpu::AddrIndexedZeroPageX, 6},  // 0x56
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x57
    Instruction{&Cpu::InstrCLI, nullptr, 2},                     // 0x58
    Instruction{&Cpu::InstrEOR, &Cpu::AddrIndexedAbsoluteY, 4},  // 0x59
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x5A
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x5B
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x5C
    Instruction{&Cpu::InstrEOR, &Cpu::AddrIndexedAbsoluteX, 4},  // 0x5D
    Instruction{&Cpu::InstrLSR, &Cpu::AddrIndexedAbsoluteX, 7},  // 0x5E
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x5F
    Instruction{&Cpu::InstrRTS, nullptr, 6},                     // 0x60
    Instruction{&Cpu::InstrADC, &Cpu::AddrIndexedIndirectX, 6},  // 0x61
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x62
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x63
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x64
    Instruction{&Cpu::InstrADC, &Cpu::AddrZeroPage, 3},          // 0x65
    Instruction{&Cpu::InstrROR, &Cpu::AddrZeroPage, 5},          // 0x66
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x67
    Instruction{&Cpu::InstrPLA, nullptr, 4},                     // 0x68
    Instruction{&Cpu::InstrADC, &Cpu::AddrImmediate, 2},         // 0x69
    Instruction{&Cpu::InstrROR_AcummAddr, nullptr, 2},           // 0x6A
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x6B
    Instruction{&Cpu::InstrJMP, &Cpu::AddrAbsoluteIndirect, 5},  // 0x6C
    Instruction{&Cpu::InstrADC, &Cpu::AddrAbsolute, 4},          // 0x6D
    Instruction{&Cpu::InstrROR, &Cpu::AddrAbsolute, 6},          // 0x6E
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x6F
    Instruction{&Cpu::InstrBVS, &Cpu::AddrRelative, 2},          // 0x70
    Instruction{&Cpu::InstrADC, &Cpu::AddrIndirectIndexedY, 5},  // 0x71
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x72
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x73
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x74
    Instruction{&Cpu::InstrADC, &Cpu::AddrIndexedZeroPageX, 4},  // 0x75
    Instruction{&Cpu::InstrROR, &Cpu::AddrIndexedZeroPageX, 6},  // 0x76
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x77
    Instruction{&Cpu::InstrSEI, nullptr, 2},                     // 0x78
    Instruction{&Cpu::InstrADC, &Cpu::AddrIndexedAbsoluteY, 4},  // 0x79
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x7A
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x7B
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x7C
    Instruction{&Cpu::InstrADC, &Cpu::AddrIndexedAbsoluteX, 4},  // 0x7D
    Instruction{&Cpu::InstrROR, &Cpu::AddrIndexedAbsoluteX, 7},  // 0x7E
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x7F
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x80
    Instruction{&Cpu::InstrSTA, &Cpu::AddrIndexedIndirectX, 6},  // 0x81
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x82
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x83
    Instruction{&Cpu::InstrSTY, &Cpu::AddrZeroPage, 3},          // 0x84
    Instruction{&Cpu::InstrSTA, &Cpu::AddrZeroPage, 3},          // 0x85
    Instruction{&Cpu::InstrSTX, &Cpu::AddrZeroPage, 3},          // 0x86
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x87
    Instruction{&Cpu::InstrDEY, nullptr, 2},                     // 0x88
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x89
    Instruction{&Cpu::InstrTXA, nullptr, 2},                     // 0x8A
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x8B
    Instruction{&Cpu::InstrSTY, &Cpu::AddrAbsolute, 4},          // 0x8C
    Instruction{&Cpu::InstrSTA, &Cpu::AddrAbsolute, 4},          // 0x8D
    Instruction{&Cpu::InstrSTX, &Cpu::AddrAbsolute, 4},          // 0x8E
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x8F
    Instruction{&Cpu::InstrBCC, &Cpu::AddrRelative, 2},          // 0x90
    Instruction{&Cpu::InstrSTA, &Cpu::AddrIndirectIndexedY, 6},  // 0x91
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x92
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x93
    Instruction{&Cpu::InstrSTY, &Cpu::AddrIndexedZeroPageX, 4},  // 0x94
    Instruction{&Cpu::InstrSTA, &Cpu::AddrIndexedZeroPageX, 4},  // 0x95
    Instruction{&Cpu::InstrSTX, &Cpu::AddrIndexedZeroPageY, 4},  // 0x96
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x97
    Instruction{&Cpu::InstrTYA, nullptr, 2},                     // 0x98
    Instruction{&Cpu::InstrSTA, &Cpu::AddrIndexedAbsoluteY, 5},  // 0x99
    Instruction{&Cpu::InstrTXS, nullptr, 2},                     // 0x9A
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x9B
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x9C
    Instruction{&Cpu::InstrSTA, &Cpu::AddrIndexedAbsoluteX, 5},  // 0x9D
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x9E
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0x9F
    Instruction{&Cpu::InstrLDY, &Cpu::AddrImmediate, 2},         // 0xA0
    Instruction{&Cpu::InstrLDA, &Cpu::AddrIndexedIndirectX, 6},  // 0xA1
    Instruction{&Cpu::InstrLDX, &Cpu::AddrImmediate, 2},         // 0xA2
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xA3
    Instruction{&Cpu::InstrLDY, &Cpu::AddrZeroPage, 3},          // 0xA4
    Instruction{&Cpu::InstrLDA, &Cpu::AddrZeroPage, 3},          // 0xA5
    Instruction{&Cpu::InstrLDX, &Cpu::AddrZeroPage, 3},          // 0xA6
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xA7
    Instruction{&Cpu::InstrTAY, nullptr, 2},                     // 0xA8
    Instruction{&Cpu::InstrLDA, &Cpu::AddrImmediate, 2},         // 0xA9
    Instruction{&Cpu::InstrTAX, nullptr, 2},                     // 0xAA
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xAB
    Instruction{&Cpu::InstrLDY, &Cpu::AddrAbsolute, 4},          // 0xAC
    Instruction{&Cpu::InstrLDA, &Cpu::AddrAbsolute, 4},          // 0xAD
    Instruction{&Cpu::InstrLDX, &Cpu::AddrAbsolute, 4},          // 0xAE
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xAF
    Instruction{&Cpu::InstrBCS, &Cpu::AddrRelative, 2},          // 0xB0
    Instruction{&Cpu::InstrLDA, &Cpu::AddrIndirectIndexedY, 5},  // 0xB1
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xB2
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xB3
    Instruction{&Cpu::InstrLDY, &Cpu::AddrIndexedZeroPageX, 4},  // 0xB4
    Instruction{&Cpu::InstrLDA, &Cpu::AddrIndexedZeroPageX, 4},  // 0xB5
    Instruction{&Cpu::InstrLDX, &Cpu::AddrIndexedZeroPageY, 4},  // 0xB6
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xB7
    Instruction{&Cpu::InstrCLV, nullptr, 2},                     // 0xB8
    Instruction{&Cpu::InstrLDA, &Cpu::AddrIndexedAbsoluteY, 4},  // 0xB9
    Instruction{&Cpu::InstrTSX, nullptr, 2},                     // 0xBA
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xBB
    Instruction{&Cpu::InstrLDY, &Cpu::AddrIndexedAbsoluteX, 4},  // 0xBC
    Instruction{&Cpu::InstrLDA, &Cpu::AddrIndexedAbsoluteX, 4},  // 0xBD
    Instruction{&Cpu::InstrLDX, &Cpu::AddrIndexedAbsoluteY, 4},  // 0xBE
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xBF
    Instruction{&Cpu::InstrCPY, &Cpu::AddrImmediate, 2},         // 0xC0
    Instruction{&Cpu::InstrCMP, &Cpu::AddrIndexedIndirectX, 6},  // 0xC1
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xC2
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xC3
    Instruction{&Cpu::InstrCPY, &Cpu::AddrZeroPage, 3},          // 0xC4
    Instruction{&Cpu::InstrCMP, &Cpu::AddrZeroPage, 3},          // 0xC5
    Instruction{&Cpu::InstrDEC, &Cpu::AddrZeroPage, 5},          // 0xC6
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xC7
    Instruction{&Cpu::InstrINY, nullptr, 2},                     // 0xC8
    Instruction{&Cpu::InstrCMP, &Cpu::AddrImmediate, 2},         // 0xC9
    Instruction{&Cpu::InstrDEX, nullptr, 2},                     // 0xCA
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xCB
    Instruction{&Cpu::InstrCPY, &Cpu::AddrAbsolute, 4},          // 0xCC
    Instruction{&Cpu::InstrCMP, &Cpu::AddrAbsolute, 4},          // 0xCD
    Instruction{&Cpu::InstrDEC, &Cpu::AddrAbsolute, 6},          // 0xCE
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xCF
    Instruction{&Cpu::InstrBNE, &Cpu::AddrRelative, 2},          // 0xD0
    Instruction{&Cpu::InstrCMP, &Cpu::AddrIndirectIndexedY, 5},  // 0xD1
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xD2
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xD3
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xD4
    Instruction{&Cpu::InstrCMP, &Cpu::AddrIndexedZeroPageX, 4},  // 0xD5
    Instruction{&Cpu::InstrDEC, &Cpu::AddrIndexedZeroPageX, 6},  // 0xD6
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xD7
    Instruction{&Cpu::InstrCLD, nullptr, 2},                     // 0xD8
    Instruction{&Cpu::InstrCMP, &Cpu::AddrIndexedAbsoluteY, 4},  // 0xD9
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xDA
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xDB
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xDC
    Instruction{&Cpu::InstrCMP, &Cpu::AddrIndexedAbsoluteX, 4},  // 0xDD
    Instruction{&Cpu::InstrDEC, &Cpu::AddrIndexedAbsoluteX, 7},  // 0xDE
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xDF
    Instruction{&Cpu::InstrCPX, &Cpu::AddrImmediate, 2},         // 0xE0
    Instruction{&Cpu::InstrSBC, &Cpu::AddrIndexedIndirectX, 6},  // 0xE1
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xE2
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xE3
    Instruction{&Cpu::InstrCPX, &Cpu::AddrZeroPage, 3},          // 0xE4
    Instruction{&Cpu::InstrSBC, &Cpu::AddrZeroPage, 3},          // 0xE5
    Instruction{&Cpu::InstrINC, &Cpu::AddrZeroPage, 5},          // 0xE6
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xE7
    Instruction{&Cpu::InstrINX, nullptr, 2},                     // 0xE8
    Instruction{&Cpu::InstrSBC, &Cpu::AddrImmediate, 2},         // 0xE9
    Instruction{&Cpu::InstrNOP, nullptr, 2},                     // 0xEA
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xEB
    Instruction{&Cpu::InstrCPX, &Cpu::AddrAbsolute, 4},          // 0xEC
    Instruction{&Cpu::InstrSBC, &Cpu::AddrAbsolute, 4},          // 0xED
    Instruction{&Cpu::InstrINC, &Cpu::AddrAbsolute, 6},          // 0xEE
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xEF
    Instruction{&Cpu::InstrBEQ, &Cpu::AddrRelative, 2},          // 0xF0
    Instruction{&Cpu::InstrSBC, &Cpu::AddrIndirectIndexedY, 5},  // 0xF1
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xF2
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xF3
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xF4
    Instruction{&Cpu::InstrSBC, &Cpu::AddrIndexedZeroPageX, 4},  // 0xF5
    Instruction{&Cpu::InstrINC, &Cpu::AddrIndexedZeroPageX, 6},  // 0xF6
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xF7
    Instruction{&Cpu::InstrSED, nullptr, 2},                     // 0xF8
    Instruction{&Cpu::InstrSBC, &Cpu::AddrIndexedAbsoluteY, 4},  // 0xF9
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xFA
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xFB
    Instruction{&Cpu::InstrNoImpl, nullptr, 0},                  // 0xFC
    Instruction{&Cpu::InstrSBC, &Cpu::AddrIndexedAbsoluteX, 4},  // 0xFD
    Instruction{&Cpu::InstrINC, &Cpu::AddrIndexedAbsoluteX, 7},  // 0xFE
    Instruction{&Cpu::InstrNoImpl, nullptr, 0}                   // 0xFF
};

constexpr Cpu::Instruction::Instruction(const FuncPtr executeInstruction,
                                        const FuncPtr execureAddressingMode,
                                        const uint8_t cycles)
    : m_ExecuteInstruction{executeInstruction},
      m_ExecureAddressingMode{execureAddressingMode},
      m_Cycles{cycles} {}

constexpr const Cpu::Instruction& Cpu::FindInstruction(const uint8_t opCode) {
    using Instruction = Cpu::Instruction;

    return m_InstructionTable[opCode];
}

}  // namespace dearnes