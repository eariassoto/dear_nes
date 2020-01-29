#pragma once
#include <unordered_map>
#include <map>
#include <string>

namespace cpuemulator {
class Bus;

class InstructionDisassembler {
   public:
    InstructionDisassembler(Bus& bus);
    void Render();

   private:
    Bus& m_Bus;
    std::map<uint16_t, std::string> m_DisassembledMemory;

	void DisassembleMemory(uint16_t beginAddress, uint16_t endAddress);

    static std::unordered_map<uint8_t, std::pair<std::string, uint8_t>>&
    GetInstructionSet();
};
}  // namespace cpuemulator