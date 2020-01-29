#include "include\ram_widget.h"
#include "include\bus.h"

namespace cpuemulator {

RamWidget::RamWidget(std::string name, Bus& bus, size_t memSize, size_t baseAddr)
    : m_Name{name}, m_Bus{bus}, m_MemSize{memSize}, m_BaseAddr{baseAddr} {
}

void RamWidget::Render() {
    ImGui::Begin(m_Name.c_str());
    ImGui::SetWindowSize({530, 280});
    
	m_MemoryEditor.DrawContents((void*)(m_Bus.GetMemoryPtr()), m_MemSize,
                                m_BaseAddr);
    ImGui::End();
}

}  // namespace cpuemulator
