// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <optional>

#include "include/cpu_widget.h"

namespace cpuemulator {
class Nes;

class Cpu {
   public:
    Cpu();
    void RegisterNesPointer(Nes* bus);

    void RenderWidgets();
    void Clock();
    void Reset();
    void InterruptRequest();
    void NonMaskableInterrupt();

    bool IsCurrentInstructionComplete() const;

   private:
    enum FLAGS {
        C = (1 << 0),  // Carry Bit
        Z = (1 << 1),  // Zero
        I = (1 << 2),  // Disable Interrupts
        D = (1 << 3),  // Decimal Mode
        B = (1 << 4),  // Break
        U = (1 << 5),  // Unused
        V = (1 << 6),  // Overflow
        N = (1 << 7),  // Negative
    };

    enum AddressingMode {
        ACCUMMULATOR_ADDRESSING = 0,
        IMPLIED_ADDRESSING,
        IMMEDIATE_ADDRESSING,
        ZERO_PAGE_ADDRESSING,
        INDEXED_ZERO_PAGE_ADDRESSING_X,
        INDEXED_ZERO_PAGE_ADDRESSING_Y,
        ABSOLUTE_ADDRESSING,
        INDEXED_ABSOLUTE_ADDRESSING_X,
        INDEXED_ABSOLUTE_ADDRESSING_Y,
        ABSOLUTE_INDIRECT_ADDRESSING,
        INDEXED_INDIRECT_ADDRESSING_X,
        INDIRECT_INDEXED_ADDRESSING_Y,
        RELATIVE_ADDRESSING
    };

    AddressingMode m_AddressingMode = AddressingMode::ACCUMMULATOR_ADDRESSING;

    struct Instruction;
    std::vector<std::optional<Instruction>> m_InstrTable;

    uint8_t m_Cycles = 0x00;
    uint8_t m_OpCode = 0x00;

    uint8_t m_RegisterA = 0x00;
    uint8_t m_RegisterX = 0x00;
    uint8_t m_RegisterY = 0x00;

    uint8_t m_StackPointer = 0x00;

    uint8_t m_StatusRegister = 0x00;

    uint16_t m_ProgramCounter = 0x00;

    uint8_t m_ValueFetched = 0x00;

    uint16_t m_AddressAbsolute = 0x0000;
    uint16_t m_AddressRelative = 0x0000;

    Nes* m_NesPtr = nullptr;

    CpuWidget m_CpuWidget;

   private:
    uint8_t GetFlag(FLAGS flag) const;
    void SetFlag(FLAGS flag, bool value);
    uint8_t Read(uint16_t address);
    void Write(uint16_t address, uint8_t data);

    void RegisterAllInstructionSet();

    // Addressing modes

    bool ExecuteAddressing();
    bool AccumulatorAddressing();
    bool ImmediateAddressing();
    bool ZeroPageAddressing();
    bool IndexedZeroPageAddressingX();
    bool IndexedZeroPageAddressingY();
    bool AbsoluteAddressing();
    bool IndexedAbsoluteAddressingX();
    bool IndexedAbsoluteAddressingY();
    bool AbsoluteIndirectAddressing();
    bool IndexedIndirectAddressingX();
    bool IndirectIndexedAddressingY();
    bool RelativeAddressing();

    // Instructions
    uint8_t Fetch();

    bool Instruction_ADC();
    bool Instruction_AND();
    bool Instruction_ASL();
    void Instruction_ExecuteBranch();
    bool Instruction_BCC();
    bool Instruction_BCS();
    bool Instruction_BEQ();
    bool Instruction_BIT();
    bool Instruction_BMI();
    bool Instruction_BNE();
    bool Instruction_BPL();
    bool Instruction_BRK();
    bool Instruction_BVC();
    bool Instruction_BVS();
    bool Instruction_CLC();
    bool Instruction_CLD();
    bool Instruction_CLI();
    bool Instruction_CLV();
    bool Instruction_CMP();
    bool Instruction_CPX();
    bool Instruction_CPY();
    bool Instruction_DEC();
    bool Instruction_DEX();
    bool Instruction_DEY();
    bool Instruction_EOR();
    bool Instruction_INC();
    bool Instruction_INX();
    bool Instruction_INY();
    bool Instruction_JMP();
    bool Instruction_JSR();
    bool Instruction_LDA();
    bool Instruction_LDX();
    bool Instruction_LDY();
    bool Instruction_LSR();
    bool Instruction_NOP();
    bool Instruction_ORA();
    bool Instruction_PHA();
    bool Instruction_PHP();
    bool Instruction_PLA();
    bool Instruction_PLP();
    bool Instruction_ROL();
    bool Instruction_ROR();
    bool Instruction_RTI();
    bool Instruction_RTS();
    bool Instruction_SBC();
    bool Instruction_SEC();
    bool Instruction_SED();
    bool Instruction_SEI();
    bool Instruction_STA();
    bool Instruction_STX();
    bool Instruction_STY();
    bool Instruction_TAX();
    bool Instruction_TAY();
    bool Instruction_TSX();
    bool Instruction_TXA();
    bool Instruction_TXS();
    bool Instruction_TYA();

    void AppendAddressingModeString(uint16_t address,
                                    AddressingMode addressingMode,
                                    std::string& outStr);
    std::string GetInstructionString(uint16_t opCodeAddress);

    friend class CpuWidget;
};

struct Cpu::Instruction {
    Instruction(const char name[4], std::function<uint8_t(Cpu*)> funcOperate,
                AddressingMode addressingMode, uint8_t cycles)
        : m_Name{name},
          m_FuncOperate{funcOperate},
          m_AddressingMode{addressingMode},
	m_Cycles{cycles} {}

    std::string m_Name;
    std::function<bool(Cpu*)> m_FuncOperate;
    AddressingMode m_AddressingMode;
    uint8_t m_Cycles = 0;
};
}  // namespace cpuemulator