#include "include/ram_widget.h"
#include <imgui/imgui.h>

namespace cpuemulator {

RamWidget::RamWidget(std::string name, void* memoryPtr, size_t memSize,
                     size_t baseAddr)
    : m_Name{name},
      m_MemoryPtr{memoryPtr},
      m_MemSize{memSize},
      m_BaseAddr{baseAddr} {}

void RamWidget::Render() {
    ImGui::Begin(m_Name.c_str());
    ImGui::SetWindowSize({530, 280});

    m_MemoryEditor.DrawContents(m_MemoryPtr, m_MemSize, m_BaseAddr);
    ImGui::End();
}

}  // namespace cpuemulator
