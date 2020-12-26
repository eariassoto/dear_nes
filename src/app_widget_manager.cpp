// Copyright (c) 2020 Emmanuel Arias
#include "include/app_widget_manager.h"

#include <imgui.h>

#include "include/controller_widget.h"
#include "include/cpu_widget.h"
#include "include/ppu_nametable_widget.h"
#include "include/ppu_pattern_table_widget.h"
#include "include/screen_widget.h"
#include "include/status_widget.h"

AppWidgetManager::AppWidgetManager() {
    AddWidget(new ScreenWidget());
    m_NesStatusWindow = dynamic_cast<StatusWidget*>(
        AddWidget(new StatusWidget()));
    AddWidget(new CpuWidget());
    AddWidget(new PpuPatternTableWidget(0));
    AddWidget(new PpuPatternTableWidget(1));
    AddWidget(new ControllerWidget(0));
    // AddWindow(new ImGuiNesPpuNametableWindow(0));
    // AddWindow(new ImGuiNesPpuNametableWindow(1));
}
