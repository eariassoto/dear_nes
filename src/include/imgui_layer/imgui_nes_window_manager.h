// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <string>
#include "include/imgui_layer/imgui_window_manager.h"

class ImGuiNesStatusWindow;

struct ImGuiNesWindowManager : public ImGuiWindowManager {

    ImGuiNesWindowManager();

    ImGuiNesStatusWindow* m_NesStatusWindow = nullptr;
};
