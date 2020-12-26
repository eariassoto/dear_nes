// Copyright (c) 2020 Emmanuel Arias
#include "include/status_widget.h"

#include <imgui.h>

#ifdef _WIN32
#include <shlobj.h>

#include <algorithm>
#include <fstream>

#include "helpers/RootDir.h"
#endif

#include "include/global_nes.h"
#include "dear_nes_lib/cartridge.h"

bool StatusWidget::IsNesPoweredUp() const { return m_IsPowerUp; }

void StatusWidget::Render() {
    if (!m_Show) {
        return;
    }
    if (!Begin(m_WindowName)) {
        End();
        return;
    }
    bool buttonPressed = false;
    if (!m_IsPowerUp) {
        buttonPressed = ImGui::Button(m_PowerUpStr.c_str());
    } else {
        buttonPressed = ImGui::Button(m_ShutdownStr.c_str());
    }
    if (buttonPressed) {
        m_IsPowerUp = !m_IsPowerUp;
    }

    ImGui::SameLine();

    bool resetPressed = ImGui::Button("Reset");
    if (resetPressed) {
        dearnes::Nes* nesPtr = g_GetGlobalNes();
        nesPtr->Reset();
    }

#ifdef _WIN32
    ImGui::SameLine();

    bool loadCartridgePressed = ImGui::Button("Load Cartridge");
    if (loadCartridgePressed) {
        dearnes::Nes* nesPtr = g_GetGlobalNes();
        std::wstring newPath = GetFileFromUser();
        if (!newPath.empty()) {
            std::ifstream ifs;
            ifs.open(newPath, std::ifstream::binary);

            auto ret = m_CartridgeLoader.LoadNewCartridge(ifs);
            if (auto pval = std::get_if<dearnes::Cartridge*>(&ret)) {
                nesPtr->InsertCatridge(*pval);
            }
        }
    }
#endif
    End();
}

#ifdef _WIN32
std::wstring StatusWidget::GetFileFromUser() {
    std::wstring filePath;
    HRESULT hr =
        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr)) {
        IFileOpenDialog* pFileOpen;

        // Create the FileOpenDialog object.
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                              IID_IFileOpenDialog,
                              reinterpret_cast<void**>(&pFileOpen));

        if (SUCCEEDED(hr)) {
            IShellItem* shellItem;

            std::wstring basePath = WSTRING(ROOT_DIR) L"res/roms";
            std::replace(basePath.begin(), basePath.end(), '/', '\\');

            HRESULT hr = SHCreateItemFromParsingName(
                basePath.c_str(), 0, IID_IShellItem,
                reinterpret_cast<void**>(&shellItem));
            if (SUCCEEDED(hr)) {
                pFileOpen->SetFolder(shellItem);
                shellItem->Release();

                // Show the Open dialog box.
                hr = pFileOpen->Show(NULL);

                // Get the file name from the dialog box.
                if (SUCCEEDED(hr)) {
                    IShellItem* pItem;
                    hr = pFileOpen->GetResult(&pItem);
                    if (SUCCEEDED(hr)) {
                        PWSTR pszFilePath;
                        hr = pItem->GetDisplayName(SIGDN_FILESYSPATH,
                                                   &pszFilePath);

                        // Display the file name to the user.
                        if (SUCCEEDED(hr)) {
                            filePath = pszFilePath;
                            CoTaskMemFree(pszFilePath);
                        }
                        pItem->Release();
                    }
                }
                pFileOpen->Release();
            }
        }
        CoUninitialize();
    }
    return filePath;
}
#endif
