#include "../src/window_manager.h"

#include <cassert>
#include <cstdint>
#include <vector>

static WindowInfo MakeWindow(const wchar_t* process, const wchar_t* title,
                             DWORD pid, uintptr_t hwndValue) {
    WindowInfo info = {};
    info.hWnd = reinterpret_cast<HWND>(hwndValue);
    info.processName = process;
    info.title = title;
    info.processId = pid;
    return info;
}

int main() {
    std::vector<WindowInfo> windows;
    windows.push_back(MakeWindow(L"Game.exe", L"B Window", 20, 200));
    windows.push_back(MakeWindow(L"alpha.exe", L"Z Window", 30, 300));
    windows.push_back(MakeWindow(L"game.exe", L"A Window", 10, 100));
    windows.push_back(MakeWindow(L"game.exe", L"A Window", 10, 50));

    WindowManager::SortWindows(windows);

    assert(windows[0].processName == L"alpha.exe");
    assert(windows[1].title == L"A Window" &&
           reinterpret_cast<uintptr_t>(windows[1].hWnd) == 50);
    assert(windows[2].title == L"A Window" &&
           reinterpret_cast<uintptr_t>(windows[2].hWnd) == 100);
    assert(windows[3].title == L"B Window");
    return 0;
}
