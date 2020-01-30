#pragma once
#include <unordered_map>
#include <map>
#include <string>
#include <imgui.h>

namespace cpuemulator {
class Bus;

class InstructionDisassembler {
   public:
    InstructionDisassembler(Bus& bus);
    void Render(uint16_t currentPC);
    void DisassembleMemory(uint16_t beginAddress, uint16_t endAddress);

   private:
    Bus& m_Bus;
    std::map<uint16_t, std::string> m_DisassembledMemory;
    static const ImVec4 m_ColorCurrInstr;

    static std::unordered_map<uint8_t, std::pair<std::string, uint8_t>>&
    GetInstructionSet();
};
}  // namespace cpuemulator