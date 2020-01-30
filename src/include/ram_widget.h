#pragma once
#include <string>
#include <imgui/imgui.h>
#include <imgui_memory_editor.h>

namespace cpuemulator {
class Bus;

class RamWidget {
   public:
    RamWidget(std::string name, void* memoryPtr, size_t memSize, size_t baseAddr);
    void Render();

   private:
    std::string m_Name;
    void* m_MemoryPtr;
    size_t m_MemSize;
    size_t m_BaseAddr;
    MemoryEditor m_MemoryEditor;
};

}  // namespace cpuemulator
