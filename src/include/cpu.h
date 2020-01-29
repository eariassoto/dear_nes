#pragma once
#include <functional>
#include <string>
#include <unordered_map>

namespace cpuemulator {
class Bus;

class Cpu {
   public:
    void ConnectBus(Bus* bus);

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

    uint8_t GetFlag(FLAGS flag) const;

    inline uint16_t GetProgramCounter() const { return m_ProgramCounter; }
    inline uint16_t GetStackPointer() const { return m_StackPointer; }
    inline uint8_t GetRegisterA() const { return m_RegisterA; }
    inline uint8_t GetRegisterX() const { return m_RegisterX; }
    inline uint8_t GetRegisterY() const { return m_RegisterY; }

    void Clock();
    void Reset();
    void InterruptRequest();
    void NonMaskableInterrupt();

    bool InstructionComplete() const;

   private:
    AddressingMode m_AddressingMode = AddressingMode::ACCUMMULATOR_ADDRESSING;

    struct Instruction;
    std::unordered_map<uint8_t, Instruction> m_InstructionSet;

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

    Bus* m_Bus = nullptr;

   private:
    void SetFlag(FLAGS flag, bool value);
    uint8_t Read(uint16_t address);
    void Write(uint16_t address, uint8_t data);

    void RegisterAllInstructionSet();

    // Addressing modes

    uint8_t ExecuteAddressing();
    uint8_t AccumulatorAddressing();
    uint8_t ImpliedAddressing();
    uint8_t ImmediateAddressing();
    uint8_t ZeroPageAddressing();
    uint8_t IndexedZeroPageAddressingX();
    uint8_t IndexedZeroPageAddressingY();
    uint8_t AbsoluteAddressing();
    uint8_t IndexedAbsoluteAddressingX();
    uint8_t IndexedAbsoluteAddressingY();
    uint8_t AbsoluteIndirectAddressing();
    uint8_t IndexedIndirectAddressingX();
    uint8_t IndirectIndexedAddressingY();
    uint8_t RelativeAddressing();

    // Instructions
    uint8_t Fetch();

    uint8_t Instruction_ADC();
    uint8_t Instruction_AND();
    uint8_t Instruction_ASL();
    void Instruction_ExecuteBranch();
    uint8_t Instruction_BCC();
    uint8_t Instruction_BCS();
    uint8_t Instruction_BEQ();
    uint8_t Instruction_BIT();
    uint8_t Instruction_BMI();
    uint8_t Instruction_BNE();
    uint8_t Instruction_BPL();
    uint8_t Instruction_BRK();
    uint8_t Instruction_BVC();
    uint8_t Instruction_BVS();
    uint8_t Instruction_CLC();
    uint8_t Instruction_CLD();
    uint8_t Instruction_CLI();
    uint8_t Instruction_CLV();
    uint8_t Instruction_CMP();
    uint8_t Instruction_CPX();
    uint8_t Instruction_CPY();
    uint8_t Instruction_DEC();
    uint8_t Instruction_DEX();
    uint8_t Instruction_DEY();
    uint8_t Instruction_EOR();
    uint8_t Instruction_INC();
    uint8_t Instruction_INX();
    uint8_t Instruction_INY();
    uint8_t Instruction_JMP();
    uint8_t Instruction_JSR();
    uint8_t Instruction_LDA();
    uint8_t Instruction_LDX();
    uint8_t Instruction_LDY();
    uint8_t Instruction_LSR();
    uint8_t Instruction_NOP();
    uint8_t Instruction_ORA();
    uint8_t Instruction_PHA();
    uint8_t Instruction_PHP();
    uint8_t Instruction_PLA();
    uint8_t Instruction_PLP();
    uint8_t Instruction_ROL();
    uint8_t Instruction_ROR();
    uint8_t Instruction_RTI();
    uint8_t Instruction_RTS();
    uint8_t Instruction_SBC();
    uint8_t Instruction_SEC();
    uint8_t Instruction_SED();
    uint8_t Instruction_SEI();
    uint8_t Instruction_STA();
    uint8_t Instruction_STX();
    uint8_t Instruction_STY();
    uint8_t Instruction_TAX();
    uint8_t Instruction_TAY();
    uint8_t Instruction_TSX();
    uint8_t Instruction_TXA();
    uint8_t Instruction_TXS();
    uint8_t Instruction_TYA();
};

struct Cpu::Instruction {
    std::string m_Name;
    std::function<uint8_t()> m_FuncOperate;
    AddressingMode m_AddressingMode;
    uint8_t m_Cycles = 0;
};
}  // namespace cpuemulator