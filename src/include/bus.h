#pragma once
#include <array>
#include <cinttypes>

namespace cpuemulator {

class Bus {
   public:
    Bus();
    ~Bus();

	inline const uint8_t* GetMemoryPtr() const { return m_RAM; }

   private:
    // fake ram
    uint8_t *m_RAM = nullptr;

   public:
    void Write(uint16_t address, uint8_t data);
    uint8_t Read(uint16_t address, bool isReadOnly = false);
};
}  // namespace cpuemulator