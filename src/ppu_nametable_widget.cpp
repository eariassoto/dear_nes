// Copyright (c) 2020-2021 Emmanuel Arias
#include "include/ppu_nametable_widget.h"

#include <fmt/core.h>
#include <imgui.h>

#include "dear_nes_lib/nes.h"
#include "dear_nes_lib/ppu.h"
#include "include/dearnes_base_widget.h"

PpuNametableWidget::PpuNametableWidget(dearnes::Nes* nesPtr, unsigned int nametableIdx)
    : DearNESBaseWidget(nesPtr)
    , m_NametableIdx{nametableIdx}
    , m_WindowName{fmt::format("Ppu Nametable {}", m_NametableIdx)} {
    for (int i = 0; i < 30 * 32; ++i) {
        m_RefreshValues[i] = 0;
        m_NametableValues[i] = 0;
    }
}

void PpuNametableWidget::Update(float delta) {
    auto m_PpuPtr = m_NesPtr->GetPpu();

    for (int i = 0; i < 30 * 32; ++i) {
        if (m_RefreshValues[i] > 0) {
            m_RefreshValues[i] -= delta;
        }

        uint8_t newValue = m_PpuPtr->m_Nametables[m_NametableIdx][i];
        if (newValue != m_NametableValues[i]) {
            m_RefreshValues[i] = m_InitialRefreshValue;
        }
        m_NametableValues[i] = newValue;
    }
}

void PpuNametableWidget::Render() {
    if (!m_Show) {
        return;
    }
    if (!Begin(m_WindowName)) {
        End();
        return;
    }

    size_t aux = 0;
    for (size_t y = 0; y < 30; ++y) {
        for (size_t x = 0; x < 32; ++x) {
            ImGui::SameLine();

            auto refresh = m_RefreshValues[32 * y + x];
            auto text = fmt::format("{:02x}", m_NametableValues[32 * y + x]);
            if (refresh > 0) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
                                   text.c_str());
            } else {
                ImGui::Text(text.c_str());
            }

            ++aux;
        }
        ImGui::NewLine();
    }

    End();
}
