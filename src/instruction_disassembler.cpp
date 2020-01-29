#include "include/instruction_disassembler.h"
#include <fmt/core.h>
#include "include/bus.h"

namespace cpuemulator {

InstructionDisassembler::InstructionDisassembler(Bus& bus) : m_Bus{bus} {}

void InstructionDisassembler::Render() {}

void InstructionDisassembler::DisassembleMemory(uint16_t beginAddress,
                                             uint16_t endAddress) {
    m_DisassembledMemory.clear();
    std::unordered_map<uint8_t, std::pair<std::string, uint8_t>>& instrSet =
        GetInstructionSet();
    for (uint16_t currAddr = beginAddress; currAddr < endAddress; ++currAddr) {
        uint8_t opCode = m_Bus.Read(currAddr);
        auto it = instrSet.find(opCode);
        if (it != instrSet.end()) {
            std::pair<std::string, uint8_t>& instr = (*it).second;
            switch (instr.second) {
                case 0:
                    m_DisassembledMemory.emplace(
                        currAddr, fmt::format(instr.first, currAddr));
                    break;
                case 1: {
                    uint8_t param1 = m_Bus.Read(++currAddr);
                    m_DisassembledMemory.emplace(
                        currAddr, fmt::format(instr.first, currAddr, param1));
                } break;
                case 2: {
                    uint8_t param1 = m_Bus.Read(++currAddr);
                    uint8_t param2 = m_Bus.Read(++currAddr);
                    m_DisassembledMemory.emplace(
                        currAddr,
                        fmt::format(instr.first, currAddr, param1, param2));
                } break;
            }
        }
    }
}

std::unordered_map<uint8_t, std::pair<std::string, uint8_t>>&
InstructionDisassembler::GetInstructionSet() {
    static std::unordered_map<uint8_t, std::pair<std::string, uint8_t>>
        instrSet;
    instrSet.emplace(0x00, std::make_pair("{:#06X}: BRK", 0));
    instrSet.emplace(0x01, std::make_pair("{:#06X}: ORA (${:#04X},X)", 1));
    instrSet.emplace(0x05, std::make_pair("{:#06X}: ORA ${:#04X}", 1));
    instrSet.emplace(0x06, std::make_pair("{:#06X}: ASL ${:#04X}", 1));
    instrSet.emplace(0x08, std::make_pair("{:#06X}: PHP", 0));
    instrSet.emplace(0x09, std::make_pair("{:#06X}: ORA #${:#04X}", 1));
    instrSet.emplace(0x0A, std::make_pair("{:#06X}: ASL A", 0));
    instrSet.emplace(0x0D, std::make_pair("{:#06X}: ORA ${:#04X}{:#04X}", 2));
    instrSet.emplace(0x0E, std::make_pair("{:#06X}: ASL ${:#04X}{:#04X}", 2));
    instrSet.emplace(0x10, std::make_pair("{:#06X}: BPL ${:#04X}", 1));
    instrSet.emplace(0x11, std::make_pair("{:#06X}: ORA (${:#04X}),Y", 1));
    instrSet.emplace(0x15, std::make_pair("{:#06X}: ORA ${:#04X},X", 1));
    instrSet.emplace(0x16, std::make_pair("{:#06X}: ASL ${:#04X},X", 1));
    instrSet.emplace(0x18, std::make_pair("{:#06X}: CLC", 0));
    instrSet.emplace(0x19, std::make_pair("{:#06X}: ORA ${:#04X}{:#04X},Y", 2));
    instrSet.emplace(0x1D, std::make_pair("{:#06X}: ORA ${:#04X}{:#04X},X", 2));
    instrSet.emplace(0x1E, std::make_pair("{:#06X}: ASL ${:#04X}{:#04X},X", 2));
    instrSet.emplace(0x20, std::make_pair("{:#06X}: JSR ${:#04X}{:#04X}", 2));
    instrSet.emplace(0x21, std::make_pair("{:#06X}: AND (${:#04X},X)", 1));
    instrSet.emplace(0x24, std::make_pair("{:#06X}: BIT ${:#04X}", 1));
    instrSet.emplace(0x25, std::make_pair("{:#06X}: AND ${:#04X}", 1));
    instrSet.emplace(0x26, std::make_pair("{:#06X}: ROL ${:#04X}", 1));
    instrSet.emplace(0x28, std::make_pair("{:#06X}: PLP", 0));
    instrSet.emplace(0x29, std::make_pair("{:#06X}: AND #${:#04X}", 1));
    instrSet.emplace(0x2A, std::make_pair("{:#06X}: ROL A", 0));
    instrSet.emplace(0x2C, std::make_pair("{:#06X}: BIT ${:#04X}{:#04X}", 2));
    instrSet.emplace(0x2D, std::make_pair("{:#06X}: AND ${:#04X}{:#04X}", 2));
    instrSet.emplace(0x2E, std::make_pair("{:#06X}: ROL ${:#04X}{:#04X}", 2));
    instrSet.emplace(0x30, std::make_pair("{:#06X}: BMI ${:#04X}", 1));
    instrSet.emplace(0x31, std::make_pair("{:#06X}: AND (${:#04X}),Y", 1));
    instrSet.emplace(0x35, std::make_pair("{:#06X}: AND ${:#04X},X", 1));
    instrSet.emplace(0x36, std::make_pair("{:#06X}: ROL ${:#04X},X", 1));
    instrSet.emplace(0x38, std::make_pair("{:#06X}: SEC", 0));
    instrSet.emplace(0x39, std::make_pair("{:#06X}: AND ${:#04X}{:#04X},Y", 2));
    instrSet.emplace(0x3D, std::make_pair("{:#06X}: AND ${:#04X}{:#04X},X", 2));
    instrSet.emplace(0x3E, std::make_pair("{:#06X}: ROL ${:#04X}{:#04X},X", 2));
    instrSet.emplace(0x40, std::make_pair("{:#06X}: RTI", 0));
    instrSet.emplace(0x41, std::make_pair("{:#06X}: EOR (${:#04X},X)", 1));
    instrSet.emplace(0x45, std::make_pair("{:#06X}: EOR ${:#04X}", 1));
    instrSet.emplace(0x46, std::make_pair("{:#06X}: LSR ${:#04X}", 1));
    instrSet.emplace(0x48, std::make_pair("{:#06X}: PHA", 0));
    instrSet.emplace(0x49, std::make_pair("{:#06X}: EOR #${:#04X}", 1));
    instrSet.emplace(0x4A, std::make_pair("{:#06X}: LSR A", 0));
    instrSet.emplace(0x4C, std::make_pair("{:#06X}: JMP ${:#04X}{:#04X}", 2));
    instrSet.emplace(0x4D, std::make_pair("{:#06X}: EOR ${:#04X}{:#04X}", 2));
    instrSet.emplace(0x4E, std::make_pair("{:#06X}: LSR ${:#04X}{:#04X}", 2));
    instrSet.emplace(0x50, std::make_pair("{:#06X}: BVC ${:#04X}", 1));
    instrSet.emplace(0x51, std::make_pair("{:#06X}: EOR (${:#04X}),Y", 1));
    instrSet.emplace(0x55, std::make_pair("{:#06X}: EOR ${:#04X},X", 1));
    instrSet.emplace(0x56, std::make_pair("{:#06X}: LSR ${:#04X},X", 1));
    instrSet.emplace(0x58, std::make_pair("{:#06X}: CLI", 0));
    instrSet.emplace(0x59, std::make_pair("{:#06X}: EOR ${:#04X}{:#04X},Y", 2));
    instrSet.emplace(0x5D, std::make_pair("{:#06X}: EOR ${:#04X}{:#04X},X", 2));
    instrSet.emplace(0x5E, std::make_pair("{:#06X}: LSR ${:#04X}{:#04X},X", 2));
    instrSet.emplace(0x60, std::make_pair("{:#06X}: RTS", 0));
    instrSet.emplace(0x61, std::make_pair("{:#06X}: ADC (${:#04X},X)", 1));
    instrSet.emplace(0x65, std::make_pair("{:#06X}: ADC ${:#04X}", 1));
    instrSet.emplace(0x66, std::make_pair("{:#06X}: ROR ${:#04X}", 1));
    instrSet.emplace(0x68, std::make_pair("{:#06X}: PLA", 0));
    instrSet.emplace(0x69, std::make_pair("{:#06X}: ADC #${:#04X}", 1));
    instrSet.emplace(0x6A, std::make_pair("{:#06X}: ROR A", 0));
    instrSet.emplace(0x6C, std::make_pair("{:#06X}: JMP (${:#04X}{:#04X})", 2));
    instrSet.emplace(0x6D, std::make_pair("{:#06X}: ADC ${:#04X}{:#04X}", 2));
    instrSet.emplace(0x6E, std::make_pair("{:#06X}: ROR ${:#04X}{:#04X}", 2));
    instrSet.emplace(0x70, std::make_pair("{:#06X}: BVS ${:#04X}", 1));
    instrSet.emplace(0x71, std::make_pair("{:#06X}: ADC (${:#04X}),Y", 1));
    instrSet.emplace(0x75, std::make_pair("{:#06X}: ADC ${:#04X},X", 1));
    instrSet.emplace(0x76, std::make_pair("{:#06X}: ROR ${:#04X},X", 1));
    instrSet.emplace(0x78, std::make_pair("{:#06X}: SEI", 0));
    instrSet.emplace(0x79, std::make_pair("{:#06X}: ADC ${:#04X}{:#04X},Y", 2));
    instrSet.emplace(0x7D, std::make_pair("{:#06X}: ADC ${:#04X}{:#04X},X", 2));
    instrSet.emplace(0x7E, std::make_pair("{:#06X}: ROR ${:#04X}{:#04X},X", 2));
    instrSet.emplace(0x81, std::make_pair("{:#06X}: STA (${:#04X},X)", 1));
    instrSet.emplace(0x84, std::make_pair("{:#06X}: STY ${:#04X}", 1));
    instrSet.emplace(0x85, std::make_pair("{:#06X}: STA ${:#04X}", 1));
    instrSet.emplace(0x86, std::make_pair("{:#06X}: STX ${:#04X}", 1));
    instrSet.emplace(0x88, std::make_pair("{:#06X}: DEY", 0));
    instrSet.emplace(0x8A, std::make_pair("{:#06X}: TXA", 0));
    instrSet.emplace(0x8C, std::make_pair("{:#06X}: STY ${:#04X}{:#04X}", 2));
    instrSet.emplace(0x8D, std::make_pair("{:#06X}: STA ${:#04X}{:#04X}", 2));
    instrSet.emplace(0x8E, std::make_pair("{:#06X}: STX ${:#04X}{:#04X}", 2));
    instrSet.emplace(0x90, std::make_pair("{:#06X}: BCC ${:#04X}", 1));
    instrSet.emplace(0x91, std::make_pair("{:#06X}: STA (${:#04X}),Y", 1));
    instrSet.emplace(0x94, std::make_pair("{:#06X}: STY ${:#04X},X", 1));
    instrSet.emplace(0x95, std::make_pair("{:#06X}: STA ${:#04X},X", 1));
    instrSet.emplace(0x96, std::make_pair("{:#06X}: STX ${:#04X},Y", 1));
    instrSet.emplace(0x98, std::make_pair("{:#06X}: TYA", 0));
    instrSet.emplace(0x99, std::make_pair("{:#06X}: STA ${:#04X}{:#04X},Y", 2));
    instrSet.emplace(0x9A, std::make_pair("{:#06X}: TXS", 0));
    instrSet.emplace(0x9D, std::make_pair("{:#06X}: STA ${:#04X}{:#04X},X", 2));
    instrSet.emplace(0xA0, std::make_pair("{:#06X}: LDY #${:#04X}", 1));
    instrSet.emplace(0xA1, std::make_pair("{:#06X}: LDA (${:#04X},X)", 1));
    instrSet.emplace(0xA2, std::make_pair("{:#06X}: LDX #${:#04X}", 1));
    instrSet.emplace(0xA4, std::make_pair("{:#06X}: LDY ${:#04X}", 1));
    instrSet.emplace(0xA5, std::make_pair("{:#06X}: LDA ${:#04X}", 1));
    instrSet.emplace(0xA6, std::make_pair("{:#06X}: LDX ${:#04X}", 1));
    instrSet.emplace(0xA8, std::make_pair("{:#06X}: TAY", 0));
    instrSet.emplace(0xA9, std::make_pair("{:#06X}: LDA #${:#04X}", 1));
    instrSet.emplace(0xAA, std::make_pair("{:#06X}: TAX", 0));
    instrSet.emplace(0xAC, std::make_pair("{:#06X}: LDY ${:#04X}{:#04X}", 2));
    instrSet.emplace(0xAD, std::make_pair("{:#06X}: LDA ${:#04X}{:#04X}", 2));
    instrSet.emplace(0xAE, std::make_pair("{:#06X}: LDX ${:#04X}{:#04X}", 2));
    instrSet.emplace(0xB0, std::make_pair("{:#06X}: BCS ${:#04X}", 1));
    instrSet.emplace(0xB1, std::make_pair("{:#06X}: LDA (${:#04X}),Y", 1));
    instrSet.emplace(0xB4, std::make_pair("{:#06X}: LDY ${:#04X},X", 1));
    instrSet.emplace(0xB5, std::make_pair("{:#06X}: LDA ${:#04X},X", 1));
    instrSet.emplace(0xB6, std::make_pair("{:#06X}: LDX ${:#04X},Y", 1));
    instrSet.emplace(0xB8, std::make_pair("{:#06X}: CLV", 0));
    instrSet.emplace(0xB9, std::make_pair("{:#06X}: LDA ${:#04X}{:#04X},Y", 2));
    instrSet.emplace(0xBA, std::make_pair("{:#06X}: TSX", 0));
    instrSet.emplace(0xBC, std::make_pair("{:#06X}: LDY ${:#04X}{:#04X},X", 2));
    instrSet.emplace(0xBD, std::make_pair("{:#06X}: LDA ${:#04X}{:#04X},X", 2));
    instrSet.emplace(0xBE, std::make_pair("{:#06X}: LDX ${:#04X}{:#04X},Y", 2));
    instrSet.emplace(0xC0, std::make_pair("{:#06X}: CPY #${:#04X}", 1));
    instrSet.emplace(0xC1, std::make_pair("{:#06X}: CMP (${:#04X},X)", 1));
    instrSet.emplace(0xC4, std::make_pair("{:#06X}: CPY ${:#04X}", 1));
    instrSet.emplace(0xC5, std::make_pair("{:#06X}: CMP ${:#04X}", 1));
    instrSet.emplace(0xC6, std::make_pair("{:#06X}: DEC ${:#04X}", 1));
    instrSet.emplace(0xC8, std::make_pair("{:#06X}: INY", 0));
    instrSet.emplace(0xC9, std::make_pair("{:#06X}: CMP #${:#04X}", 1));
    instrSet.emplace(0xCA, std::make_pair("{:#06X}: DEX", 0));
    instrSet.emplace(0xCC, std::make_pair("{:#06X}: CPY ${:#04X}{:#04X}", 2));
    instrSet.emplace(0xCD, std::make_pair("{:#06X}: CMP ${:#04X}{:#04X}", 2));
    instrSet.emplace(0xCE, std::make_pair("{:#06X}: DEC ${:#04X}{:#04X}", 2));
    instrSet.emplace(0xD0, std::make_pair("{:#06X}: BNE ${:#04X}", 1));
    instrSet.emplace(0xD1, std::make_pair("{:#06X}: CMP (${:#04X}),Y", 1));
    instrSet.emplace(0xD5, std::make_pair("{:#06X}: CMP ${:#04X},X", 1));
    instrSet.emplace(0xD6, std::make_pair("{:#06X}: DEC ${:#04X},X", 1));
    instrSet.emplace(0xD8, std::make_pair("{:#06X}: CLD", 0));
    instrSet.emplace(0xD9, std::make_pair("{:#06X}: CMP ${:#04X}{:#04X},Y", 2));
    instrSet.emplace(0xDD, std::make_pair("{:#06X}: CMP ${:#04X}{:#04X},X", 2));
    instrSet.emplace(0xDF, std::make_pair("{:#06X}: DEC ${:#04X}{:#04X},X", 2));
    instrSet.emplace(0xE0, std::make_pair("{:#06X}: CPX #${:#04X}", 1));
    instrSet.emplace(0xE1, std::make_pair("{:#06X}: SBC (${:#04X},X)", 1));
    instrSet.emplace(0xE4, std::make_pair("{:#06X}: CPX ${:#04X}", 1));
    instrSet.emplace(0xE5, std::make_pair("{:#06X}: SBC ${:#04X}", 1));
    instrSet.emplace(0xE6, std::make_pair("{:#06X}: INC ${:#04X}", 1));
    instrSet.emplace(0xE8, std::make_pair("{:#06X}: INX", 0));
    instrSet.emplace(0xE9, std::make_pair("{:#06X}: SBC #${:#04X}", 1));
    instrSet.emplace(0xEA, std::make_pair("{:#06X}: NOP", 0));
    instrSet.emplace(0xEC, std::make_pair("{:#06X}: CPX ${:#04X}{:#04X}", 2));
    instrSet.emplace(0xED, std::make_pair("{:#06X}: SBC ${:#04X}{:#04X}", 2));
    instrSet.emplace(0xEE, std::make_pair("{:#06X}: INC ${:#04X}{:#04X}", 2));
    instrSet.emplace(0xF0, std::make_pair("{:#06X}: BEQ ${:#04X}", 1));
    instrSet.emplace(0xF1, std::make_pair("{:#06X}: SBC (${:#04X}),Y", 1));
    instrSet.emplace(0xF5, std::make_pair("{:#06X}: SBC ${:#04X},X", 1));
    instrSet.emplace(0xF6, std::make_pair("{:#06X}: INC ${:#04X},X", 1));
    instrSet.emplace(0xF8, std::make_pair("{:#06X}: SED", 0));
    instrSet.emplace(0xF9, std::make_pair("{:#06X}: SBC ${:#04X}{:#04X},Y", 2));
    instrSet.emplace(0xFD, std::make_pair("{:#06X}: SBC ${:#04X}{:#04X},X", 2));
    instrSet.emplace(0xFE, std::make_pair("{:#06X}: INC ${:#04X}{:#04X},X", 2));
    return instrSet;
}

}  // namespace cpuemulator
