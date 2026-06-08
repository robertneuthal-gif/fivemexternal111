#pragma once
#include <Gui/Overlay/Overlay.hpp>
#include <Includes/Includes.hpp>
#include <core/sdk/Memory.hpp>
#include <Includes/Utils.hpp>
#include <core/sdk/sdk.hpp>
#include <core/core.hpp>
#include <Gui/gui.hpp>

#include <winternl.h>
#include <windows.h>
#include <dwmapi.h>
#include <tchar.h>
#include <vector>
#include <regex>

#include <Security/AntiCrack.hpp>
#include <csignal>
#include <utilities/auth/auth.hpp>

using namespace core;


std::string WStringToString(const std::wstring& wstr) {
    if (wstr.empty()) return {};

    int size_needed = WideCharToMultiByte(
        CP_UTF8,
        0,
        wstr.data(),
        (int)wstr.size(),
        nullptr,
        0,
        nullptr,
        nullptr
    );
    std::string result(size_needed, 0);
    WideCharToMultiByte(
        CP_UTF8,
        0,
        wstr.data(),
        (int)wstr.size(),
        result.data(),
        size_needed,
        nullptr,
        nullptr
    );
    return result;
}


int main(int argc, char* argv[]) {

    HANDLE hMutex = CreateMutexA(nullptr, TRUE, xorstr("secure_mutex"));
    if (!hMutex) {
        return 0;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(hMutex);
        return 0;
    }

    if (!driver.GetMaxPrivileges(GetCurrentProcess())) {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 0;
    }


    constexpr int maxWaitMs = 30000;
    int waitedMs = 0;
    while (!g_Variables.g_hGameWindow && waitedMs < maxWaitMs) {
        g_Variables.g_hGameWindow = FindWindowA(xorstr("grcWindow"), nullptr);
        if (g_Variables.g_hGameWindow) {
            auto WindowInfo = Utils::GetWindowPosAndSize(g_Variables.g_hGameWindow);
            g_Variables.g_vGameWindowPos = WindowInfo.first;
            g_Variables.g_vGameWindowSize = WindowInfo.second;
            g_Variables.g_vGameWindowCenter = {
                g_Variables.g_vGameWindowSize.x / 2,
                g_Variables.g_vGameWindowSize.y / 2
            };
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        waitedMs += 500;
    }

    if (!g_Variables.g_hGameWindow) {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 0;
    }

    DWORD ProcIdFiveM = 0;
    GetWindowThreadProcessId(g_Variables.g_hGameWindow, &ProcIdFiveM);
    if (!ProcIdFiveM) {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 0;
    }

    std::string processName = driver.GetNameByPid(ProcIdFiveM);
    if (processName.empty()) {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 0;
    }

    if (!driver.OpenProc(processName.c_str())) {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 0;
    }

    if (!driver.ModBase) {
        MessageBoxA(nullptr, xorstr("Looks like there was an error Please restart your computer."), xorstr("Warning"), MB_OK | MB_ICONWARNING);
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 0;
    }

    uintptr_t modSize = 0;
    driver.name_module_base = driver.GetModuleBaseAddr(ProcIdFiveM, xorstr("citizen-playernames-five.dll"), &modSize);
    driver.glue_dll = driver.GetModuleBaseAddr(ProcIdFiveM, xorstr("glue.dll"), &modSize);

    offsets.CurrentBuild = 3258;

    GetOffsets();

    Gui::cOverlay.Render();

    ReleaseMutex(hMutex);
    CloseHandle(hMutex);

    return 0;
}