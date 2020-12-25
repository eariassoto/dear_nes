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
    AddWindow(new ScreenWidget());
    m_NesStatusWindow = dynamic_cast<StatusWidget*>(
        AddWindow(new StatusWidget()));
    AddWindow(new CpuWidget());
    AddWindow(new PpuPatternTableWidget(0));
    AddWindow(new PpuPatternTableWidget(1));
    AddWindow(new ControllerWidget(0));
    // AddWindow(new ImGuiNesPpuNametableWindow(0));
    // AddWindow(new ImGuiNesPpuNametableWindow(1));
}
