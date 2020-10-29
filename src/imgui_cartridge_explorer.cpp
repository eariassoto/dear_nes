// Copyright (c) 2020 Emmanuel Arias
#include "include/imgui_cartridge_explorer.h"

#include <imgui.h>
#include <shlobj.h>

#include <algorithm>
#include <iostream>
#include <string>

#include "helpers/RootDir.h"
#include  "virtual-nes/cartridge.h"

namespace virtualnes {

int CALLBACK ImguiCartridgeExplorer::BrowseCallbackProc(HWND hwnd, UINT uMsg,
                                                        LPARAM lParam,
                                                        LPARAM lpData) {
    if (uMsg == BFFM_INITIALIZED) {
        std::cout << (const char*)lpData << std::endl;
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
    }

    return 0;
}

std::string ImguiCartridgeExplorer::BrowseFolder(std::string savedPath) {
    TCHAR path[MAX_PATH];

    BROWSEINFO bi = {0};
    bi.lpszTitle = ("Browse for folder...");
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.lpfn = BrowseCallbackProc;
    bi.lParam = (LPARAM)savedPath.c_str();

    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

    if (pidl != 0) {
        // get the name of the folder and put it in path
        SHGetPathFromIDList(pidl, path);

        // free memory used
        IMalloc* imalloc = 0;
        if (SUCCEEDED(SHGetMalloc(&imalloc))) {
            imalloc->Free(pidl);
            imalloc->Release();
        }

        return path;
    }

    return "";
}

std::wstring ImguiCartridgeExplorer::GetFileFromUser() {
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

ImguiCartridgeExplorer::ImguiCartridgeExplorer(
    const std::string& cartridgePath) {
    // TODO
    m_CartridgeLoaded = true;
}

void ImguiCartridgeExplorer::Update() {}

void ImguiCartridgeExplorer::RenderWidgets() {
    ImGui::Begin("Cartridge Explorer");
    //if (ImGui::Button("Load new cartridge")) {
    //    std::wstring newPath = GetFileFromUser();
    //    if (!newPath.empty()) {
    //    
    //        Cartridge* cartridge = new Cartridge(newPath);
    //        if (cartridge->IsLoaded()) {
    //            // TODO
    //        }

    //    }
    //}
    ImGui::End();
}
}  // namespace virtualnes