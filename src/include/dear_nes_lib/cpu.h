// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <cinttypes>

#include "dear_nes_lib/enums.h"

namespace dearnes {

// Forward declaration
class Bus;

/// <summary>
/// Virtual implementation of the 6502 CPU version for the NES. The instruction
/// set for this implementation is based on https://www.masswerk.at/6502/6502_instruction_set.html
/// The CPU can be reset using the Reset() function, and this will simulate the real
/// life version of the reset routine.
/// In this current implementation illegal instruction are not handled. There are some instruction
/// that do not crash the system, but they do originally undocumented operations. Some games
/// take advantage of this and will need to support said instructions.
/// </summary>
class Cpu {
   public:

    /// <summary>
    /// Set the reference to the memory bus
    /// </summary>
    /// <param name="bus"></param>
    void SetBus(Bus *bus);

    /// <summary>
    /// The RESET routine takes 8 cycles, and for our purposes
    /// it will simulate this process:
    /// * Zero registers A, X, Y and status flags
    /// * Load the program counter register by reading the address from
    ///   0xFFFC (low byte) and 0xFFFD (high byte)
    /// * Leave the stack pointer register in 0xFD
    /// For more info refer to: https://www.c64-wiki.com/wiki/Reset_(Process)
    /// </summary>
    void Reset();

    /// <summary>
    /// This function will read the opcode from the memory location pointed by the
    /// program counter register and execute the function. It will add the proper amount
    /// of cycles to fully simulate the cycle count.
    /// </summary>
    void Clock();

    /// <summary>
    /// Simulate the NMI (non maskable interruption) process. The IRL process is
    /// described in this wiki entry:
    /// https://wiki.nesdev.com/w/index.php/CPU_interrupts#IRQ_and_NMI_tick-by-tick_execution
    /// </summary>
    void NonMaskableInterrupt();

    /// <summary>
    /// Returns true if the current instruction has finished to wait for the cycles
    /// it takes to finish in the real hardware version
    /// </summary>
    /// <returns></returns>
    inline bool IsCurrentInstructionComplete() const { return m_Cycles == 0; }

    /// <summary>
    /// Return 0x01 or 0x01 for a given register flag
    /// </summary>
    /// <param name="flag"></param>
    /// <returns></returns>
    inline uint8_t GetFlag(CpuFlag flag) const {
        if ((m_StatusRegister & flag) == 0x00) {
            return 0;
        }
        return 1;
    }

    /// <summary>
    /// Set a register flag
    /// </summary>
    /// <param name="flag"></param>
    /// <param name="value"></param>
    inline void SetFlag(CpuFlag flag, bool value) {
        if (value) {
            m_StatusRegister |= flag;
        } else {
            m_StatusRegister &= ~flag;
        }
    }

    /// <summary>
    /// Get the value for register A
    /// </summary>
    /// <returns></returns>
    inline uint8_t GetRegisterA() const { return m_RegisterA; }
    
    /// <summary>
    /// Get the value for register X
    /// </summary>
    /// <returns></returns>
    inline uint8_t GetRegisterX() const { return m_RegisterX; }
    
    /// <summary>
    /// Get the value for register Y
    /// </summary>
    /// <returns></returns>
    inline uint8_t GetRegisterY() const { return m_RegisterY; }
    
    /// <summary>
    /// Get the value for the stack pointer register
    /// </summary>
    /// <returns></returns>
    inline uint8_t GetStackPointer() const { return m_StackPointer; }
    
    /// <summary>
    /// Get the value for the program counter register
    /// </summary>
    /// <returns></returns>
    inline uint16_t GetProgramCounter() const { return m_ProgramCounter; }

    /// <summary>
    /// Typedef for a void() function pointer. Used for the instruction table callbacks.
    /// </summary>
    using FuncPtr = void (Cpu::*)();

    /// <summary>
    /// Wrapper object for a item in the look-up table. It is meant to be used in
    /// a constexpr container.
    /// </summary>
    struct Instruction {
        /// <summary>
        /// Construct a constexpr item for the instruction set lookup table.
        /// The table will reference the operation code to the proper callbacks.
        /// </summary>
        /// <param name="executeInstruction"></param>
        /// <param name="execureAddressingMode"></param>
        /// <param name="cycles"></param>
        constexpr Instruction(const FuncPtr executeInstruction,
                              const FuncPtr execureAddressingMode,
                               const uint8_t cycles);        
        /// <summary>
        /// Instruction callback. This is the virtual implementation of the instruction.
        /// </summary>
        const FuncPtr m_ExecuteInstruction;

        /// <summary>
        /// Virtual implementation for the memory addressing associated to the instruction.
        /// A instruction can have more than one op code, each associated to a specific
        /// addressing mode.
        /// </summary>
        const FuncPtr m_ExecureAddressingMode;

        /// <summary>
        /// Base cycle duration for the instruction.
        /// </summary>
        uint8_t m_Cycles;
    };

   private:
    Bus *m_Bus;

    uint8_t m_RegisterA = 0x00;
    uint8_t m_RegisterX = 0x00;
    uint8_t m_RegisterY = 0x00;

    uint8_t m_StackPointer = 0x00;

    uint8_t m_StatusRegister = 0x00;

    uint16_t m_ProgramCounter = 0x00;

    uint16_t m_AddressAbsolute = 0x0000;
    uint16_t m_AddressRelative = 0x0000;

    uint8_t m_OpCode = 0x00;
    uint8_t m_Cycles = 0x00;

    bool m_AddressingModeNeedsAdditionalCycle = false;
    bool m_InstructionNeedsAdditionalCycle = false;

    static Instruction m_InstructionTable[0x100];

   private:
    uint8_t Read(uint16_t address);

    void Write(uint16_t address, uint8_t data);

    uint8_t ReadWordFromProgramCounter();

    uint16_t ReadDoubleWordFromProgramCounter();

    constexpr const Instruction& FindInstruction(const uint8_t opCode);

   private:
    void AddrImmediate();

    void AddrZeroPage();

    void AddrIndexedZeroPageX();

    void AddrIndexedZeroPageY();

    void AddrAbsolute();

    void AddrIndexedAbsoluteX();

    void AddrIndexedAbsoluteY();

    void AddrAbsoluteIndirect();

    void AddrIndexedIndirectX();

    void AddrIndirectIndexedY();

    void AddrRelative();

    void InstrNoImpl();

    void InstrADC();

    void InstrAND();

    void InstrASL();

    void InstrASL_AcummAddr();

    void InstrExecuteBranch();

    void InstrBCC();

    void InstrBCS();

    void InstrBEQ();

    void InstrBIT();

    void InstrBMI();

    void InstrBNE();

    void InstrBPL();

    void InstrBRK();

    void InstrBVC();

    void InstrBVS();

    void InstrCLC();

    void InstrCLD();

    void InstrCLI();

    void InstrCLV();

    void InstrCMP();

    void InstrCPX();

    void InstrCPY();

    void InstrDEC();

    void InstrDEX();

    void InstrDEY();

    void InstrEOR();

    void InstrINC();

    void InstrINX();

    void InstrINY();

    void InstrJMP();

    void InstrJSR();

    void InstrLDA();

    void InstrLDX();

    void InstrLDY();

    void InstrLSR();

    void InstrLSR_AcummAddr();

    void InstrNOP();

    void InstrORA();

    void InstrPHA();

    void InstrPHP();

    void InstrPLA();

    void InstrPLP();

    void InstrROL();

    void InstrROL_AcummAddr();

    void InstrROR();

    void InstrROR_AcummAddr();

    void InstrRTI();

    void InstrRTS();

    void InstrSBC();

    void InstrSEC();

    void InstrSED();

    void InstrSEI();

    void InstrSTA();

    void InstrSTX();

    void InstrSTY();

    void InstrTAX();

    void InstrTAY();

    void InstrTSX();

    void InstrTXA();

    void InstrTXS();

    void InstrTYA();
};

}  // namespace dearnes
