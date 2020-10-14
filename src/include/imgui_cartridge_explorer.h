// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <string>
// TODO: precompile guard
#include <windows.h>

namespace cpuemulator {

class NesEmulator;

class ImguiCartridgeExplorer {
   public:
    ImguiCartridgeExplorer() = default;
    explicit ImguiCartridgeExplorer(const std::string& cartridgePath);

    void Update();
    void RenderWidgets();

   private:
    static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam,
                                           LPARAM lpData);
    std::string BrowseFolder(std::string savedPath);

    std::wstring GetFileFromUser();

    bool m_CartridgeLoaded = false;
};

}  // namespace cpuemulator
