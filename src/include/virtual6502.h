// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <cinttypes>
#include <functional>
#include <map>

template <class MemoryInterfaceT>
class Virtual6502 {
   public:
    explicit Virtual6502(MemoryInterfaceT *memoryInterface)
        : m_MemoryInterface{memoryInterface} {
        RegisterAllInstructionSet();
    }
    ~Virtual6502() = default;

    // The RESET routine takes 8 cycles, and for our purposes
    // it will simulate this process:
    // * Zero registers A, X, Y and status flags
    // * Load the program counter register by reading the address from
    //   0xFFFC (low byte) and 0xFFFD (high byte)
    // * Leave the stack pointer register in 0xFD
    // For more info refer to: https://www.c64-wiki.com/wiki/Reset_(Process)
    void Reset() {
        m_RegisterA = 0;
        m_RegisterX = 0;
        m_RegisterY = 0;
        m_StatusRegister = 0x00 | Flag::U;

        static constexpr uint16_t addressToReadPC = 0xFFFC;
        uint16_t lo = m_MemoryInterface->CpuRead(addressToReadPC);
        uint16_t hi = m_MemoryInterface->CpuRead(addressToReadPC + 1);
        m_ProgramCounter = (hi << 8) | lo;

        m_StackPointer = 0xFD;

        m_CyclesToIdle = 8;
    }

    void Clock() {
        if (m_Cycles == 0) {
            m_OpCode = ReadWordFromProgramCounter();

            SetFlag(Flag::U, 1);

            // todo handle illegal ops
            auto instrIt = m_InstrTable.find(m_OpCode);
            if (instrIt == m_InstrTable.end()) {
                return;
            }

            Instruction &instr = instrIt->second;

            m_Cycles = instr.m_Cycles;

            if (instr.m_AddressingMode) {
                instr.m_AddressingMode(this);
            }

            instr.m_FuncOperate(this);

            if (m_AddressingModeNeedsAdditionalCycle &&
                m_InstructionNeedsAdditionalCycle) {
                ++m_Cycles;
            }

            SetFlag(U, true);
        }

        m_Cycles--;
    }

    void NonMaskableInterrupt() {
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

    bool IsCurrentInstructionComplete() const { return m_Cycles == 0; }

    enum Flag : uint8_t {
        C = (0b1 << 0),  // Carry Bit
        Z = (0b1 << 1),  // Zero
        I = (0b1 << 2),  // Disable Interrupts
        D = (0b1 << 3),  // Decimal Mode
        B = (0b1 << 4),  // Break
        U = (0b1 << 5),  // Unused
        V = (0b1 << 6),  // Overflow
        N = (0b1 << 7),  // Negative
    };

    uint8_t GetFlag(Flag flag) const {
        if ((m_StatusRegister & flag) == 0x00) {
            return 0;
        }
        return 1;
    }

    void SetFlag(Flag flag, bool value) {
        if (value) {
            m_StatusRegister |= flag;
        } else {
            m_StatusRegister &= ~flag;
        }
    }

    uint8_t GetRegisterA() const { return m_RegisterA; }
    uint8_t GetRegisterX() const { return m_RegisterX; }
    uint8_t GetRegisterY() const { return m_RegisterY; }
    uint8_t GetStackPointer() const { return m_StackPointer; }
    uint16_t GetProgramCounter() const { return m_ProgramCounter; }

   private:
    uint8_t m_RegisterA = 0x00;
    uint8_t m_RegisterX = 0x00;
    uint8_t m_RegisterY = 0x00;

    uint8_t m_StackPointer = 0x00;

    uint8_t m_StatusRegister = 0x00;

    uint16_t m_ProgramCounter = 0x00;

    uint8_t m_CyclesToIdle = 0;
    uint64_t m_GlobalCycles = 0;

    uint16_t m_AddressAbsolute = 0x0000;
    uint16_t m_AddressRelative = 0x0000;

    uint8_t m_OpCode = 0x00;
    uint8_t m_Cycles = 0x00;

    struct Instruction {
        Instruction(const std::string &name,
                    std::function<void(Virtual6502 *)> funcOperate,
                    std::function<void(Virtual6502 *)> addressingMode,
                    uint8_t cycles)
            : m_Name{name},
              m_FuncOperate{funcOperate},
              m_AddressingMode{addressingMode},
              m_Cycles{cycles} {}

        std::string m_Name;
        std::function<void(Virtual6502 *)> m_FuncOperate;
        std::function<void(Virtual6502 *)> m_AddressingMode;
        uint8_t m_Cycles = 0;
    };
    std::map<uint8_t, Instruction> m_InstrTable;

    MemoryInterfaceT *m_MemoryInterface = nullptr;

    bool m_AddressingModeNeedsAdditionalCycle = false;
    bool m_InstructionNeedsAdditionalCycle = false;

   private:
    uint8_t Read(uint16_t address) {
        assert(m_MemoryInterface != nullptr);
        return m_MemoryInterface->CpuRead(address);
    }

    void Write(uint16_t address, uint8_t data) {
        assert(m_MemoryInterface != nullptr);
        m_MemoryInterface->CpuWrite(address, data);
    }

    uint8_t ReadWordFromProgramCounter() {
        return m_MemoryInterface->CpuRead(m_ProgramCounter++);
    }

    uint16_t ReadDoubleWordFromProgramCounter() {
        uint16_t lowNibble = ReadWordFromProgramCounter();
        uint16_t highNibble = ReadWordFromProgramCounter();

        return (highNibble << 8) | lowNibble;
    }

    void ImmediateAddressing() { m_AddressAbsolute = m_ProgramCounter++; }

    void ZeroPageAddressing() {
        m_AddressAbsolute = ReadWordFromProgramCounter();
        m_AddressAbsolute &= 0x00FF;
    }

    void IndexedZeroPageAddressingX() {
        m_AddressAbsolute = ReadWordFromProgramCounter();
        m_AddressAbsolute += m_RegisterX;
        m_AddressAbsolute &= 0x00FF;
    }

    void IndexedZeroPageAddressingY() {
        m_AddressAbsolute = ReadWordFromProgramCounter();
        m_AddressAbsolute += m_RegisterY;
        m_AddressAbsolute &= 0x00FF;
    }

    void AbsoluteAddressing() {
        m_AddressAbsolute = ReadDoubleWordFromProgramCounter();
    }

    void IndexedAbsoluteAddressingX() {
        m_AddressAbsolute = ReadDoubleWordFromProgramCounter();
        uint16_t highNibble = m_AddressAbsolute & 0xFF00;

        m_AddressAbsolute += m_RegisterX;

        m_AddressingModeNeedsAdditionalCycle =
            (m_AddressAbsolute & 0xFF00) != highNibble;
    }

    void IndexedAbsoluteAddressingY() {
        m_AddressAbsolute = ReadDoubleWordFromProgramCounter();
        uint16_t highNibble = m_AddressAbsolute & 0xFF00;

        m_AddressAbsolute += m_RegisterY;

        m_AddressingModeNeedsAdditionalCycle =
            (m_AddressAbsolute & 0xFF00) != highNibble;
    }

    void AbsoluteIndirectAddressing() {
        uint16_t pointer = ReadDoubleWordFromProgramCounter();

        // Simulate page boundary hardware bug
        if ((pointer & 0x00FF) == 0x00FF) {
            m_AddressAbsolute = (Read(pointer & 0xFF00) << 8) | Read(pointer);
        } else {
            m_AddressAbsolute = (Read(pointer + 1) << 8) | Read(pointer);
        }
    }

    void IndexedIndirectAddressingX() {
        uint16_t t = ReadWordFromProgramCounter();

        uint16_t registerXValue = static_cast<uint16_t>(m_RegisterX);

        uint16_t lowNibbleAddress = (t + registerXValue) & 0x00FF;
        uint16_t lowNibble = Read(lowNibbleAddress);

        uint16_t highNibbleAddress = (t + registerXValue + 1) & 0x00FF;
        uint16_t highNibble = Read(highNibbleAddress);

        m_AddressAbsolute = (highNibble << 8) | lowNibble;
    }

    void IndirectIndexedAddressingY() {
        uint16_t t = ReadWordFromProgramCounter();

        uint16_t lowNibble = Read(t & 0x00FF);
        uint16_t highNibble = Read((t + 1) & 0x00FF);

        m_AddressAbsolute = (highNibble << 8) | lowNibble;
        m_AddressAbsolute += m_RegisterY;

        m_AddressingModeNeedsAdditionalCycle =
            (m_AddressAbsolute & 0xFF00) != (highNibble << 8);
    }

    void RelativeAddressing() {
        m_AddressRelative = ReadWordFromProgramCounter();

        if (m_AddressRelative & 0x80)  // if unsigned
        {
            m_AddressRelative |= 0xFF00;
        }
    }

    void Instruction_ADC() {
        uint8_t valueFetched = Read(m_AddressAbsolute);

        uint16_t castedFetched = static_cast<uint16_t>(valueFetched);
        uint16_t castedCarry = static_cast<uint16_t>(GetFlag(Flag::C));
        uint16_t castedAccum = static_cast<uint16_t>(m_RegisterA);

        uint16_t temp = castedAccum + castedFetched + castedCarry;
        SetFlag(Flag::C, temp > 255);
        SetFlag(Flag::Z, (temp & 0x00FF) == 0);
        SetFlag(Flag::N, temp & 0x80);
        SetFlag(V, (~(castedAccum ^ castedFetched) & (castedAccum ^ temp)) &
                       0x0080);

        m_RegisterA = temp & 0x00FF;

        m_InstructionNeedsAdditionalCycle = true;
    }

    void Instruction_AND() {
        uint8_t valueFetched = Read(m_AddressAbsolute);

        m_RegisterA &= valueFetched;
        SetFlag(Flag::Z, m_RegisterA == 0x00);
        SetFlag(Flag::N, m_RegisterA & 0x80);

        m_InstructionNeedsAdditionalCycle = true;
    }

    void Instruction_ASL() {
        uint8_t valueFetched = Read(m_AddressAbsolute);

        uint16_t temp = static_cast<uint16_t>(valueFetched) << 1;
        SetFlag(Flag::C, (temp & 0xFF00) > 0);
        SetFlag(Flag::Z, (temp & 0x00FF) == 0);
        SetFlag(Flag::N, temp & 0x80);

        Write(m_AddressAbsolute, temp & 0x00FF);
    }

    void Instruction_ASL_AcummAddr() {
        uint16_t temp = static_cast<uint16_t>(m_RegisterA) << 1;
        SetFlag(Flag::C, (temp & 0xFF00) > 0);
        SetFlag(Flag::Z, (temp & 0x00FF) == 0);
        SetFlag(Flag::N, temp & 0x80);

        m_RegisterA = temp & 0x00FF;
    }

    void Instruction_ExecuteBranch() {
        m_Cycles++;
        m_AddressAbsolute = m_ProgramCounter + m_AddressRelative;

        if ((m_AddressAbsolute & 0xFF00) != (m_ProgramCounter & 0xFF00)) {
            m_Cycles++;
        }
        m_ProgramCounter = m_AddressAbsolute;
    }

    void Instruction_BCC() {
        if (GetFlag(Flag::C) == 0) {
            Instruction_ExecuteBranch();
        }
    }

    void Instruction_BCS() {
        if (GetFlag(Flag::C) == 1) {
            Instruction_ExecuteBranch();
        }
    }

    void Instruction_BEQ() {
        if (GetFlag(Flag::Z) == 1) {
            Instruction_ExecuteBranch();
        }
    }

    void Instruction_BIT() {
        uint8_t valueFetched = Read(m_AddressAbsolute);

        uint16_t temp = m_RegisterA & valueFetched;
        SetFlag(Z, (temp & 0x00FF) == 0x00);
        SetFlag(N, valueFetched & (1 << 7));
        SetFlag(V, valueFetched & (1 << 6));
    }

    void Instruction_BMI() {
        if (GetFlag(Flag::N) == 1) {
            Instruction_ExecuteBranch();
        }
    }

    void Instruction_BNE() {
        if (GetFlag(Flag::Z) == 0) {
            Instruction_ExecuteBranch();
        }
    }

    void Instruction_BPL() {
        if (GetFlag(Flag::N) == 0) {
            Instruction_ExecuteBranch();
        }
    }

    void Instruction_BRK() {
        m_ProgramCounter++;

        SetFlag(I, 1);
        Write(0x0100 + m_StackPointer, (m_ProgramCounter >> 8) & 0x00FF);
        m_StackPointer--;
        Write(0x0100 + m_StackPointer, m_ProgramCounter & 0x00FF);
        m_StackPointer--;

        SetFlag(Flag::B, 1);
        Write(0x0100 + m_StackPointer, m_StatusRegister);
        m_StackPointer--;
        SetFlag(B, 0);

        m_ProgramCounter = static_cast<uint16_t>(Read(0xFFFE)) |
                           (static_cast<uint16_t>(Read(0xFFFF)) << 8);
    }

    void Instruction_BVC() {
        if (GetFlag(Flag::V) == 0) {
            Instruction_ExecuteBranch();
        }
    }

    void Instruction_BVS() {
        if (GetFlag(Flag::V) == 1) {
            Instruction_ExecuteBranch();
        }
    }

    void Instruction_CLC() { SetFlag(Flag::C, false); }

    void Instruction_CLD() { SetFlag(Flag::D, false); }

    void Instruction_CLI() { SetFlag(Flag::I, false); }

    void Instruction_CLV() { SetFlag(Flag::V, false); }

    void Instruction_CMP() {
        uint8_t valueFetched = Read(m_AddressAbsolute);
        uint16_t temp = static_cast<uint16_t>(m_RegisterA) -
                        static_cast<uint16_t>(valueFetched);
        SetFlag(C, m_RegisterA >= valueFetched);
        SetFlag(Z, (temp & 0x00FF) == 0x0000);
        SetFlag(N, temp & 0x0080);
        m_InstructionNeedsAdditionalCycle = true;
    }

    void Instruction_CPX() {
        uint8_t valueFetched = Read(m_AddressAbsolute);
        uint16_t temp = static_cast<uint16_t>(m_RegisterX) -
                        static_cast<uint16_t>(valueFetched);
        SetFlag(C, m_RegisterX >= valueFetched);
        SetFlag(Z, (temp & 0x00FF) == 0x0000);
        SetFlag(N, temp & 0x0080);
    }

    void Instruction_CPY() {
        uint8_t valueFetched = Read(m_AddressAbsolute);
        uint16_t temp = static_cast<uint16_t>(m_RegisterY) -
                        static_cast<uint16_t>(valueFetched);
        SetFlag(C, m_RegisterY >= valueFetched);
        SetFlag(Z, (temp & 0x00FF) == 0x0000);
        SetFlag(N, temp & 0x0080);
    }

    void Instruction_DEC() {
        uint8_t valueFetched = Read(m_AddressAbsolute);
        uint16_t temp = valueFetched - 1;
        Write(m_AddressAbsolute, temp & 0x00FF);
        SetFlag(Z, (temp & 0x00FF) == 0x0000);
        SetFlag(N, temp & 0x0080);
    }

    void Instruction_DEX() {
        m_RegisterX--;
        SetFlag(Z, m_RegisterX == 0x00);
        SetFlag(N, m_RegisterX & 0x80);
    }

    void Instruction_DEY() {
        m_RegisterY--;
        SetFlag(Z, m_RegisterY == 0x00);
        SetFlag(N, m_RegisterY & 0x80);
    }

    void Instruction_EOR() {
        uint8_t valueFetched = Read(m_AddressAbsolute);
        m_RegisterA = m_RegisterA ^ valueFetched;
        SetFlag(Z, m_RegisterA == 0x00);
        SetFlag(N, m_RegisterA & 0x80);
        m_InstructionNeedsAdditionalCycle = true;
    }

    void Instruction_INC() {
        uint8_t valueFetched = Read(m_AddressAbsolute);
        uint16_t temp = valueFetched + 1;
        Write(m_AddressAbsolute, temp & 0x00FF);
        SetFlag(Z, (temp & 0x00FF) == 0x0000);
        SetFlag(N, temp & 0x0080);
    }

    void Instruction_INX() {
        m_RegisterX++;
        SetFlag(Z, m_RegisterX == 0x00);
        SetFlag(N, m_RegisterX & 0x80);
    }

    void Instruction_INY() {
        m_RegisterY++;
        SetFlag(Z, m_RegisterY == 0x00);
        SetFlag(N, m_RegisterY & 0x80);
    }

    void Instruction_JMP() { m_ProgramCounter = m_AddressAbsolute; }

    void Instruction_JSR() {
        m_ProgramCounter--;

        Write(0x0100 + m_StackPointer, (m_ProgramCounter >> 8) & 0x00FF);
        m_StackPointer--;
        Write(0x0100 + m_StackPointer, m_ProgramCounter & 0x00FF);
        m_StackPointer--;

        m_ProgramCounter = m_AddressAbsolute;
    }

    void Instruction_LDA() {
        uint8_t valueFetched = Read(m_AddressAbsolute);
        m_RegisterA = valueFetched;
        SetFlag(Z, m_RegisterA == 0x00);
        SetFlag(N, m_RegisterA & 0x80);
        m_InstructionNeedsAdditionalCycle = true;
    }

    void Instruction_LDX() {
        uint8_t valueFetched = Read(m_AddressAbsolute);
        m_RegisterX = valueFetched;
        SetFlag(Z, m_RegisterX == 0x00);
        SetFlag(N, m_RegisterX & 0x80);
        m_InstructionNeedsAdditionalCycle = true;
    }

    void Instruction_LDY() {
        uint8_t valueFetched = Read(m_AddressAbsolute);
        m_RegisterY = valueFetched;
        SetFlag(Z, m_RegisterY == 0x00);
        SetFlag(N, m_RegisterY & 0x80);
        m_InstructionNeedsAdditionalCycle = true;
    }

    void Instruction_LSR() {
        uint8_t valueFetched = Read(m_AddressAbsolute);
        SetFlag(C, valueFetched & 0x0001);
        uint16_t temp = valueFetched >> 1;
        SetFlag(Z, (temp & 0x00FF) == 0x0000);
        SetFlag(N, temp & 0x0080);

        Write(m_AddressAbsolute, temp & 0x00FF);
    }

    void Instruction_LSR_AcummAddr() {
        SetFlag(C, m_RegisterA & 0x0001);
        uint16_t temp = m_RegisterA >> 1;
        SetFlag(Z, (temp & 0x00FF) == 0x0000);
        SetFlag(N, temp & 0x0080);

        m_RegisterA = temp & 0x00FF;
    }

    void Instruction_NOP() {}

    void Instruction_ORA() {
        uint8_t valueFetched = Read(m_AddressAbsolute);
        m_RegisterA = m_RegisterA | valueFetched;
        SetFlag(Z, m_RegisterA == 0x00);
        SetFlag(N, m_RegisterA & 0x80);
        m_InstructionNeedsAdditionalCycle = true;
    }

    void Instruction_PHA() {
        Write(0x0100 + m_StackPointer, m_RegisterA);
        m_StackPointer--;
    }

    void Instruction_PHP() {
        Write(0x0100 + m_StackPointer, m_StatusRegister | B | U);
        SetFlag(B, 0);
        SetFlag(U, 0);
        m_StackPointer--;
    }

    void Instruction_PLA() {
        m_StackPointer++;
        m_RegisterA = Read(0x0100 + m_StackPointer);
        SetFlag(Z, m_RegisterA == 0x00);
        SetFlag(N, m_RegisterA & 0x80);
    }

    void Instruction_PLP() {
        m_StackPointer++;
        m_StatusRegister = Read(0x0100 + m_StackPointer);
        SetFlag(U, 1);
    }

    void Instruction_ROL() {
        uint8_t valueFetched = Read(m_AddressAbsolute);
        uint16_t temp = static_cast<uint16_t>(valueFetched << 1) | GetFlag(C);
        SetFlag(C, temp & 0xFF00);
        SetFlag(Z, (temp & 0x00FF) == 0x0000);
        SetFlag(N, temp & 0x0080);

        Write(m_AddressAbsolute, temp & 0x00FF);
    }

    void Instruction_ROL_AcummAddr() {
        uint16_t temp = static_cast<uint16_t>(m_RegisterA << 1) | GetFlag(C);
        SetFlag(C, temp & 0xFF00);
        SetFlag(Z, (temp & 0x00FF) == 0x0000);
        SetFlag(N, temp & 0x0080);

        m_RegisterA = temp & 0x00FF;
    }

    void Instruction_ROR() {
        uint8_t valueFetched = Read(m_AddressAbsolute);
        uint16_t temp =
            static_cast<uint16_t>(GetFlag(C) << 7) | (valueFetched >> 1);
        SetFlag(C, valueFetched & 0x01);
        SetFlag(Z, (temp & 0x00FF) == 0x00);
        SetFlag(N, temp & 0x0080);

        Write(m_AddressAbsolute, temp & 0x00FF);
    }
    void Instruction_ROR_AcummAddr() {
        uint16_t temp =
            static_cast<uint16_t>(GetFlag(C) << 7) | (m_RegisterA >> 1);
        SetFlag(C, m_RegisterA & 0x01);
        SetFlag(Z, (temp & 0x00FF) == 0x00);
        SetFlag(N, temp & 0x0080);

        m_RegisterA = temp & 0x00FF;
    }

    void Instruction_RTI() {
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

    void Instruction_RTS() {
        m_StackPointer++;
        m_ProgramCounter = static_cast<uint16_t>(Read(0x0100 + m_StackPointer));
        m_StackPointer++;
        m_ProgramCounter |= static_cast<uint16_t>(Read(0x0100 + m_StackPointer))
                            << 8;

        m_ProgramCounter++;
    }

    void Instruction_SBC() {
        uint8_t valueFetched = Read(m_AddressAbsolute);

        // Operating in 16-bit domain to capture carry out

        // We can invert the bottom 8 bits with bitwise xor
        uint16_t value = static_cast<uint16_t>(valueFetched) ^ 0x00FF;

        // Notice this is exactly the same as addition from here!
        uint16_t temp = static_cast<uint16_t>(m_RegisterA) + value +
                        static_cast<uint16_t>(GetFlag(C));
        SetFlag(C, temp & 0xFF00);
        SetFlag(Z, ((temp & 0x00FF) == 0));
        SetFlag(V, (temp ^ static_cast<uint16_t>(m_RegisterA)) &
                       (temp ^ value) & 0x0080);
        SetFlag(N, temp & 0x0080);
        m_RegisterA = temp & 0x00FF;
        m_InstructionNeedsAdditionalCycle = true;
    }

    void Instruction_SEC() { SetFlag(C, true); }

    void Instruction_SED() { SetFlag(D, true); }

    void Instruction_SEI() { SetFlag(I, true); }

    void Instruction_STA() { Write(m_AddressAbsolute, m_RegisterA); }

    void Instruction_STX() { Write(m_AddressAbsolute, m_RegisterX); }

    void Instruction_STY() { Write(m_AddressAbsolute, m_RegisterY); }

    void Instruction_TAX() {
        m_RegisterX = m_RegisterA;
        SetFlag(Z, m_RegisterX == 0x00);
        SetFlag(N, m_RegisterX & 0x80);
    }

    void Instruction_TAY() {
        m_RegisterY = m_RegisterA;
        SetFlag(Z, m_RegisterY == 0x00);
        SetFlag(N, m_RegisterY & 0x80);
    }

    void Instruction_TSX() {
        m_RegisterX = m_StackPointer;
        SetFlag(Z, m_RegisterX == 0x00);
        SetFlag(N, m_RegisterX & 0x80);
    }

    void Instruction_TXA() {
        m_RegisterA = m_RegisterX;
        SetFlag(Z, m_RegisterA == 0x00);
        SetFlag(N, m_RegisterA & 0x80);
    }

    void Instruction_TXS() { m_StackPointer = m_RegisterX; }

    void Instruction_TYA() {
        m_RegisterA = m_RegisterY;
        SetFlag(Z, m_RegisterA == 0x00);
        SetFlag(N, m_RegisterA & 0x80);
    }

    void RegisterAllInstructionSet() {
        m_InstrTable.try_emplace(0x00, "BRK", &Virtual6502::Instruction_BRK,
                                 nullptr, 7);
        m_InstrTable.try_emplace(0x01, "ORA", &Virtual6502::Instruction_ORA,
                                 &Virtual6502::IndexedIndirectAddressingX, 6);
        m_InstrTable.try_emplace(0x05, "ORA", &Virtual6502::Instruction_ORA,
                                 &Virtual6502::ZeroPageAddressing, 3);
        m_InstrTable.try_emplace(0x06, "ASL", &Virtual6502::Instruction_ASL,
                                 &Virtual6502::ZeroPageAddressing, 5);
        m_InstrTable.try_emplace(0x08, "PHP", &Virtual6502::Instruction_PHP,
                                 nullptr, 3);
        m_InstrTable.try_emplace(0x09, "ORA", &Virtual6502::Instruction_ORA,
                                 &Virtual6502::ImmediateAddressing, 2);
        m_InstrTable.try_emplace(
            0x0A, "ASL", &Virtual6502::Instruction_ASL_AcummAddr, nullptr, 2);
        m_InstrTable.try_emplace(0x0D, "ORA", &Virtual6502::Instruction_ORA,
                                 &Virtual6502::AbsoluteAddressing, 4);
        m_InstrTable.try_emplace(0x0E, "ASL", &Virtual6502::Instruction_ASL,
                                 &Virtual6502::AbsoluteAddressing, 6);
        m_InstrTable.try_emplace(0x10, "BPL", &Virtual6502::Instruction_BPL,
                                 &Virtual6502::RelativeAddressing, 2);
        m_InstrTable.try_emplace(0x11, "ORA", &Virtual6502::Instruction_ORA,
                                 &Virtual6502::IndirectIndexedAddressingY, 5);
        m_InstrTable.try_emplace(0x15, "ORA", &Virtual6502::Instruction_ORA,
                                 &Virtual6502::IndexedZeroPageAddressingX, 4);
        m_InstrTable.try_emplace(0x16, "ASL", &Virtual6502::Instruction_ASL,
                                 &Virtual6502::IndexedZeroPageAddressingX, 6);
        m_InstrTable.try_emplace(0x18, "CLC", &Virtual6502::Instruction_CLC,
                                 nullptr, 2);
        m_InstrTable.try_emplace(0x19, "ORA", &Virtual6502::Instruction_ORA,
                                 &Virtual6502::IndexedAbsoluteAddressingY, 4);
        m_InstrTable.try_emplace(0x1D, "ORA", &Virtual6502::Instruction_ORA,
                                 &Virtual6502::IndexedAbsoluteAddressingX, 4);
        m_InstrTable.try_emplace(0x1E, "ASL", &Virtual6502::Instruction_ASL,
                                 &Virtual6502::IndexedAbsoluteAddressingX, 7);
        m_InstrTable.try_emplace(0x20, "JSR", &Virtual6502::Instruction_JSR,
                                 &Virtual6502::AbsoluteAddressing, 6);
        m_InstrTable.try_emplace(0x21, "AND", &Virtual6502::Instruction_AND,
                                 &Virtual6502::IndexedIndirectAddressingX, 6);
        m_InstrTable.try_emplace(0x24, "BIT", &Virtual6502::Instruction_BIT,
                                 &Virtual6502::ZeroPageAddressing, 3);
        m_InstrTable.try_emplace(0x25, "AND", &Virtual6502::Instruction_AND,
                                 &Virtual6502::ZeroPageAddressing, 3);
        m_InstrTable.try_emplace(0x26, "ROL", &Virtual6502::Instruction_ROL,
                                 &Virtual6502::ZeroPageAddressing, 5);
        m_InstrTable.try_emplace(0x28, "PLP", &Virtual6502::Instruction_PLP,
                                 nullptr, 4);
        m_InstrTable.try_emplace(0x29, "AND", &Virtual6502::Instruction_AND,
                                 &Virtual6502::ImmediateAddressing, 2);
        m_InstrTable.try_emplace(
            0x2A, "ROL", &Virtual6502::Instruction_ROL_AcummAddr, nullptr, 2);
        m_InstrTable.try_emplace(0x2C, "BIT", &Virtual6502::Instruction_BIT,
                                 &Virtual6502::AbsoluteAddressing, 4);
        m_InstrTable.try_emplace(0x2D, "AND", &Virtual6502::Instruction_AND,
                                 &Virtual6502::AbsoluteAddressing, 4);
        m_InstrTable.try_emplace(0x2E, "ROL", &Virtual6502::Instruction_ROL,
                                 &Virtual6502::AbsoluteAddressing, 6);
        m_InstrTable.try_emplace(0x30, "BMI", &Virtual6502::Instruction_BMI,
                                 &Virtual6502::RelativeAddressing, 2);
        m_InstrTable.try_emplace(0x31, "AND", &Virtual6502::Instruction_AND,
                                 &Virtual6502::IndirectIndexedAddressingY, 5);
        m_InstrTable.try_emplace(0x35, "AND", &Virtual6502::Instruction_AND,
                                 &Virtual6502::IndexedZeroPageAddressingX, 4);
        m_InstrTable.try_emplace(0x36, "ROL", &Virtual6502::Instruction_ROL,
                                 &Virtual6502::IndexedZeroPageAddressingX, 6);
        m_InstrTable.try_emplace(0x38, "SEC", &Virtual6502::Instruction_SEC,
                                 nullptr, 2);
        m_InstrTable.try_emplace(0x39, "AND", &Virtual6502::Instruction_AND,
                                 &Virtual6502::IndexedAbsoluteAddressingY, 4);
        m_InstrTable.try_emplace(0x3D, "AND", &Virtual6502::Instruction_AND,
                                 &Virtual6502::IndexedAbsoluteAddressingX, 4);
        m_InstrTable.try_emplace(0x3E, "ROL", &Virtual6502::Instruction_ROL,
                                 &Virtual6502::IndexedAbsoluteAddressingX, 7);
        m_InstrTable.try_emplace(0x40, "RTI", &Virtual6502::Instruction_RTI,
                                 nullptr, 6);
        m_InstrTable.try_emplace(0x41, "EOR", &Virtual6502::Instruction_EOR,
                                 &Virtual6502::IndexedIndirectAddressingX, 6);
        m_InstrTable.try_emplace(0x45, "EOR", &Virtual6502::Instruction_EOR,
                                 &Virtual6502::ZeroPageAddressing, 3);
        m_InstrTable.try_emplace(0x46, "LSR", &Virtual6502::Instruction_LSR,
                                 &Virtual6502::ZeroPageAddressing, 5);
        m_InstrTable.try_emplace(0x48, "PHA", &Virtual6502::Instruction_PHA,
                                 nullptr, 3);
        m_InstrTable.try_emplace(0x49, "EOR", &Virtual6502::Instruction_EOR,
                                 &Virtual6502::ImmediateAddressing, 2);
        m_InstrTable.try_emplace(
            0x4A, "LSR", &Virtual6502::Instruction_LSR_AcummAddr, nullptr, 2);
        m_InstrTable.try_emplace(0x4C, "JMP", &Virtual6502::Instruction_JMP,
                                 &Virtual6502::AbsoluteAddressing, 3);
        m_InstrTable.try_emplace(0x4D, "EOR", &Virtual6502::Instruction_EOR,
                                 &Virtual6502::AbsoluteAddressing, 4);
        m_InstrTable.try_emplace(0x4E, "LSR", &Virtual6502::Instruction_LSR,
                                 &Virtual6502::AbsoluteAddressing, 6);
        m_InstrTable.try_emplace(0x50, "BVC", &Virtual6502::Instruction_BVC,
                                 &Virtual6502::RelativeAddressing, 2);
        m_InstrTable.try_emplace(0x51, "EOR", &Virtual6502::Instruction_EOR,
                                 &Virtual6502::IndirectIndexedAddressingY, 5);
        m_InstrTable.try_emplace(0x55, "EOR", &Virtual6502::Instruction_EOR,
                                 &Virtual6502::IndexedZeroPageAddressingX, 4);
        m_InstrTable.try_emplace(0x56, "LSR", &Virtual6502::Instruction_LSR,
                                 &Virtual6502::IndexedZeroPageAddressingX, 6);
        m_InstrTable.try_emplace(0x58, "CLI", &Virtual6502::Instruction_CLI,
                                 nullptr, 2);
        m_InstrTable.try_emplace(0x59, "EOR", &Virtual6502::Instruction_EOR,
                                 &Virtual6502::IndexedAbsoluteAddressingY, 4);
        m_InstrTable.try_emplace(0x5D, "EOR", &Virtual6502::Instruction_EOR,
                                 &Virtual6502::IndexedAbsoluteAddressingX, 4);
        m_InstrTable.try_emplace(0x5E, "LSR", &Virtual6502::Instruction_LSR,
                                 &Virtual6502::IndexedAbsoluteAddressingX, 7);
        m_InstrTable.try_emplace(0x60, "RTS", &Virtual6502::Instruction_RTS,
                                 nullptr, 6);
        m_InstrTable.try_emplace(0x61, "ADC", &Virtual6502::Instruction_ADC,
                                 &Virtual6502::IndexedIndirectAddressingX, 6);
        m_InstrTable.try_emplace(0x65, "ADC", &Virtual6502::Instruction_ADC,
                                 &Virtual6502::ZeroPageAddressing, 3);
        m_InstrTable.try_emplace(0x66, "ROR", &Virtual6502::Instruction_ROR,
                                 &Virtual6502::ZeroPageAddressing, 5);
        m_InstrTable.try_emplace(0x68, "PLA", &Virtual6502::Instruction_PLA,
                                 nullptr, 4);
        m_InstrTable.try_emplace(0x69, "ADC", &Virtual6502::Instruction_ADC,
                                 &Virtual6502::ImmediateAddressing, 2);
        m_InstrTable.try_emplace(
            0x6A, "ROR", &Virtual6502::Instruction_ROR_AcummAddr, nullptr, 2);
        m_InstrTable.try_emplace(0x6C, "JMP", &Virtual6502::Instruction_JMP,
                                 &Virtual6502::AbsoluteIndirectAddressing, 5);
        m_InstrTable.try_emplace(0x6D, "ADC", &Virtual6502::Instruction_ADC,
                                 &Virtual6502::AbsoluteAddressing, 4);
        m_InstrTable.try_emplace(0x6E, "ROR", &Virtual6502::Instruction_ROR,
                                 &Virtual6502::AbsoluteAddressing, 6);
        m_InstrTable.try_emplace(0x70, "BVS", &Virtual6502::Instruction_BVS,
                                 &Virtual6502::RelativeAddressing, 2);
        m_InstrTable.try_emplace(0x71, "ADC", &Virtual6502::Instruction_ADC,
                                 &Virtual6502::IndirectIndexedAddressingY, 5);
        m_InstrTable.try_emplace(0x75, "ADC", &Virtual6502::Instruction_ADC,
                                 &Virtual6502::IndexedZeroPageAddressingX, 4);
        m_InstrTable.try_emplace(0x76, "ROR", &Virtual6502::Instruction_ROR,
                                 &Virtual6502::IndexedZeroPageAddressingX, 6);
        m_InstrTable.try_emplace(0x78, "SEI", &Virtual6502::Instruction_SEI,
                                 nullptr, 2);
        m_InstrTable.try_emplace(0x79, "ADC", &Virtual6502::Instruction_ADC,
                                 &Virtual6502::IndexedAbsoluteAddressingY, 4);
        m_InstrTable.try_emplace(0x7D, "ADC", &Virtual6502::Instruction_ADC,
                                 &Virtual6502::IndexedAbsoluteAddressingX, 4);
        m_InstrTable.try_emplace(0x7E, "ROR", &Virtual6502::Instruction_ROR,
                                 &Virtual6502::IndexedAbsoluteAddressingX, 7);
        m_InstrTable.try_emplace(0x81, "STA", &Virtual6502::Instruction_STA,
                                 &Virtual6502::IndexedIndirectAddressingX, 6);
        m_InstrTable.try_emplace(0x84, "STY", &Virtual6502::Instruction_STY,
                                 &Virtual6502::ZeroPageAddressing, 3);
        m_InstrTable.try_emplace(0x85, "STA", &Virtual6502::Instruction_STA,
                                 &Virtual6502::ZeroPageAddressing, 3);
        m_InstrTable.try_emplace(0x86, "STX", &Virtual6502::Instruction_STX,
                                 &Virtual6502::ZeroPageAddressing, 3);
        m_InstrTable.try_emplace(0x88, "DEY", &Virtual6502::Instruction_DEY,
                                 nullptr, 2);
        m_InstrTable.try_emplace(0x8A, "TXA", &Virtual6502::Instruction_TXA,
                                 nullptr, 2);
        m_InstrTable.try_emplace(0x8C, "STY", &Virtual6502::Instruction_STY,
                                 &Virtual6502::AbsoluteAddressing, 4);
        m_InstrTable.try_emplace(0x8D, "STA", &Virtual6502::Instruction_STA,
                                 &Virtual6502::AbsoluteAddressing, 4);
        m_InstrTable.try_emplace(0x8E, "STX", &Virtual6502::Instruction_STX,
                                 &Virtual6502::AbsoluteAddressing, 4);
        m_InstrTable.try_emplace(0x90, "BCC", &Virtual6502::Instruction_BCC,
                                 &Virtual6502::RelativeAddressing, 2);
        m_InstrTable.try_emplace(0x91, "STA", &Virtual6502::Instruction_STA,
                                 &Virtual6502::IndirectIndexedAddressingY, 6);
        m_InstrTable.try_emplace(0x94, "STY", &Virtual6502::Instruction_STY,
                                 &Virtual6502::IndexedZeroPageAddressingX, 4);
        m_InstrTable.try_emplace(0x95, "STA", &Virtual6502::Instruction_STA,
                                 &Virtual6502::IndexedZeroPageAddressingX, 4);
        m_InstrTable.try_emplace(0x96, "STX", &Virtual6502::Instruction_STX,
                                 &Virtual6502::IndexedZeroPageAddressingY, 4);
        m_InstrTable.try_emplace(0x98, "TYA", &Virtual6502::Instruction_TYA,
                                 nullptr, 2);
        m_InstrTable.try_emplace(0x99, "STA", &Virtual6502::Instruction_STA,
                                 &Virtual6502::IndexedAbsoluteAddressingY, 5);
        m_InstrTable.try_emplace(0x9A, "TXS", &Virtual6502::Instruction_TXS,
                                 nullptr, 2);
        m_InstrTable.try_emplace(0x9D, "STA", &Virtual6502::Instruction_STA,
                                 &Virtual6502::IndexedAbsoluteAddressingX, 5);
        m_InstrTable.try_emplace(0xA0, "LDY", &Virtual6502::Instruction_LDY,
                                 &Virtual6502::ImmediateAddressing, 2);
        m_InstrTable.try_emplace(0xA1, "LDA", &Virtual6502::Instruction_LDA,
                                 &Virtual6502::IndexedIndirectAddressingX, 6);
        m_InstrTable.try_emplace(0xA2, "LDX", &Virtual6502::Instruction_LDX,
                                 &Virtual6502::ImmediateAddressing, 2);
        m_InstrTable.try_emplace(0xA4, "LDY", &Virtual6502::Instruction_LDY,
                                 &Virtual6502::ZeroPageAddressing, 3);
        m_InstrTable.try_emplace(0xA5, "LDA", &Virtual6502::Instruction_LDA,
                                 &Virtual6502::ZeroPageAddressing, 3);
        m_InstrTable.try_emplace(0xA6, "LDX", &Virtual6502::Instruction_LDX,
                                 &Virtual6502::ZeroPageAddressing, 3);
        m_InstrTable.try_emplace(0xA8, "TAY", &Virtual6502::Instruction_TAY,
                                 nullptr, 2);
        m_InstrTable.try_emplace(0xA9, "LDA", &Virtual6502::Instruction_LDA,
                                 &Virtual6502::ImmediateAddressing, 2);
        m_InstrTable.try_emplace(0xAA, "TAX", &Virtual6502::Instruction_TAX,
                                 nullptr, 2);
        m_InstrTable.try_emplace(0xAC, "LDY", &Virtual6502::Instruction_LDY,
                                 &Virtual6502::AbsoluteAddressing, 4);
        m_InstrTable.try_emplace(0xAD, "LDA", &Virtual6502::Instruction_LDA,
                                 &Virtual6502::AbsoluteAddressing, 4);
        m_InstrTable.try_emplace(0xAE, "LDX", &Virtual6502::Instruction_LDX,
                                 &Virtual6502::AbsoluteAddressing, 4);
        m_InstrTable.try_emplace(0xB0, "BCS", &Virtual6502::Instruction_BCS,
                                 &Virtual6502::RelativeAddressing, 2);
        m_InstrTable.try_emplace(0xB1, "LDA", &Virtual6502::Instruction_LDA,
                                 &Virtual6502::IndirectIndexedAddressingY, 5);
        m_InstrTable.try_emplace(0xB4, "LDY", &Virtual6502::Instruction_LDY,
                                 &Virtual6502::IndexedZeroPageAddressingX, 4);
        m_InstrTable.try_emplace(0xB5, "LDA", &Virtual6502::Instruction_LDA,
                                 &Virtual6502::IndexedZeroPageAddressingX, 4);
        m_InstrTable.try_emplace(0xB6, "LDX", &Virtual6502::Instruction_LDX,
                                 &Virtual6502::IndexedZeroPageAddressingY, 4);
        m_InstrTable.try_emplace(0xB8, "CLV", &Virtual6502::Instruction_CLV,
                                 nullptr, 2);
        m_InstrTable.try_emplace(0xB9, "LDA", &Virtual6502::Instruction_LDA,
                                 &Virtual6502::IndexedAbsoluteAddressingY, 4);
        m_InstrTable.try_emplace(0xBA, "TSX", &Virtual6502::Instruction_TSX,
                                 nullptr, 2);
        m_InstrTable.try_emplace(0xBC, "LDY", &Virtual6502::Instruction_LDY,
                                 &Virtual6502::IndexedAbsoluteAddressingX, 4);
        m_InstrTable.try_emplace(0xBD, "LDA", &Virtual6502::Instruction_LDA,
                                 &Virtual6502::IndexedAbsoluteAddressingX, 4);
        m_InstrTable.try_emplace(0xBE, "LDX", &Virtual6502::Instruction_LDX,
                                 &Virtual6502::IndexedAbsoluteAddressingY, 4);
        m_InstrTable.try_emplace(0xC0, "CPY", &Virtual6502::Instruction_CPY,
                                 &Virtual6502::ImmediateAddressing, 2);
        m_InstrTable.try_emplace(0xC1, "CMP", &Virtual6502::Instruction_CMP,
                                 &Virtual6502::IndexedIndirectAddressingX, 6);
        m_InstrTable.try_emplace(0xC4, "CPY", &Virtual6502::Instruction_CPY,
                                 &Virtual6502::ZeroPageAddressing, 3);
        m_InstrTable.try_emplace(0xC5, "CMP", &Virtual6502::Instruction_CMP,
                                 &Virtual6502::ZeroPageAddressing, 3);
        m_InstrTable.try_emplace(0xC6, "DEC", &Virtual6502::Instruction_DEC,
                                 &Virtual6502::ZeroPageAddressing, 5);
        m_InstrTable.try_emplace(0xC8, "INY", &Virtual6502::Instruction_INY,
                                 nullptr, 2);
        m_InstrTable.try_emplace(0xC9, "CMP", &Virtual6502::Instruction_CMP,
                                 &Virtual6502::ImmediateAddressing, 2);
        m_InstrTable.try_emplace(0xCA, "DEX", &Virtual6502::Instruction_DEX,
                                 nullptr, 2);
        m_InstrTable.try_emplace(0xCC, "CPY", &Virtual6502::Instruction_CPY,
                                 &Virtual6502::AbsoluteAddressing, 4);
        m_InstrTable.try_emplace(0xCD, "CMP", &Virtual6502::Instruction_CMP,
                                 &Virtual6502::AbsoluteAddressing, 4);
        m_InstrTable.try_emplace(0xCE, "DEC", &Virtual6502::Instruction_DEC,
                                 &Virtual6502::AbsoluteAddressing, 6);
        m_InstrTable.try_emplace(0xD0, "BNE", &Virtual6502::Instruction_BNE,
                                 &Virtual6502::RelativeAddressing, 2);
        m_InstrTable.try_emplace(0xD1, "CMP", &Virtual6502::Instruction_CMP,
                                 &Virtual6502::IndirectIndexedAddressingY, 5);
        m_InstrTable.try_emplace(0xD5, "CMP", &Virtual6502::Instruction_CMP,
                                 &Virtual6502::IndexedZeroPageAddressingX, 4);
        m_InstrTable.try_emplace(0xD6, "DEC", &Virtual6502::Instruction_DEC,
                                 &Virtual6502::IndexedZeroPageAddressingX, 6);
        m_InstrTable.try_emplace(0xD8, "CLD", &Virtual6502::Instruction_CLD,
                                 nullptr, 2);
        m_InstrTable.try_emplace(0xD9, "CMP", &Virtual6502::Instruction_CMP,
                                 &Virtual6502::IndexedAbsoluteAddressingY, 4);
        m_InstrTable.try_emplace(0xDD, "CMP", &Virtual6502::Instruction_CMP,
                                 &Virtual6502::IndexedAbsoluteAddressingX, 4);
        m_InstrTable.try_emplace(0xDE, "DEC", &Virtual6502::Instruction_DEC,
                                 &Virtual6502::IndexedAbsoluteAddressingX, 7);
        m_InstrTable.try_emplace(0xE0, "CPX", &Virtual6502::Instruction_CPX,
                                 &Virtual6502::ImmediateAddressing, 2);
        m_InstrTable.try_emplace(0xE1, "SBC", &Virtual6502::Instruction_SBC,
                                 &Virtual6502::IndexedIndirectAddressingX, 6);
        m_InstrTable.try_emplace(0xE4, "CPX", &Virtual6502::Instruction_CPX,
                                 &Virtual6502::ZeroPageAddressing, 3);
        m_InstrTable.try_emplace(0xE5, "SBC", &Virtual6502::Instruction_SBC,
                                 &Virtual6502::ZeroPageAddressing, 3);
        m_InstrTable.try_emplace(0xE6, "INC", &Virtual6502::Instruction_INC,
                                 &Virtual6502::ZeroPageAddressing, 5);
        m_InstrTable.try_emplace(0xE8, "INX", &Virtual6502::Instruction_INX,
                                 nullptr, 2);
        m_InstrTable.try_emplace(0xE9, "SBC", &Virtual6502::Instruction_SBC,
                                 &Virtual6502::ImmediateAddressing, 2);
        m_InstrTable.try_emplace(0xEA, "NOP", &Virtual6502::Instruction_NOP,
                                 nullptr, 2);
        m_InstrTable.try_emplace(0xEC, "CPX", &Virtual6502::Instruction_CPX,
                                 &Virtual6502::AbsoluteAddressing, 4);
        m_InstrTable.try_emplace(0xED, "SBC", &Virtual6502::Instruction_SBC,
                                 &Virtual6502::AbsoluteAddressing, 4);
        m_InstrTable.try_emplace(0xEE, "INC", &Virtual6502::Instruction_INC,
                                 &Virtual6502::AbsoluteAddressing, 6);
        m_InstrTable.try_emplace(0xF0, "BEQ", &Virtual6502::Instruction_BEQ,
                                 &Virtual6502::RelativeAddressing, 2);
        m_InstrTable.try_emplace(0xF1, "SBC", &Virtual6502::Instruction_SBC,
                                 &Virtual6502::IndirectIndexedAddressingY, 5);
        m_InstrTable.try_emplace(0xF5, "SBC", &Virtual6502::Instruction_SBC,
                                 &Virtual6502::IndexedZeroPageAddressingX, 4);
        m_InstrTable.try_emplace(0xF6, "INC", &Virtual6502::Instruction_INC,
                                 &Virtual6502::IndexedZeroPageAddressingX, 6);
        m_InstrTable.try_emplace(0xF8, "SED", &Virtual6502::Instruction_SED,
                                 nullptr, 2);
        m_InstrTable.try_emplace(0xF9, "SBC", &Virtual6502::Instruction_SBC,
                                 &Virtual6502::IndexedAbsoluteAddressingY, 4);
        m_InstrTable.try_emplace(0xFD, "SBC", &Virtual6502::Instruction_SBC,
                                 &Virtual6502::IndexedAbsoluteAddressingX, 4);
        m_InstrTable.try_emplace(0xFE, "INC", &Virtual6502::Instruction_INC,
                                 &Virtual6502::IndexedAbsoluteAddressingX, 7);
    }
};