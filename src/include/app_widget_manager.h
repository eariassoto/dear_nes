// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <string>
#include "include/imgui_widget_manager.h"

class StatusWidget;

struct AppWidgetManager : public ImGuiWidgetManager {

    AppWidgetManager();

    StatusWidget* m_NesStatusWindow = nullptr;
};
