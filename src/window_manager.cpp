#include "window_manager.h"
#include <algorithm>
#include <cstring>
#include <cwctype>
#include <cstdint>

static std::wstring ToLowerCopy(const std::wstring& value) {
    std::wstring lower(value);
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](wchar_t ch) { return static_cast<wchar_t>(towlower(ch)); });
    return lower;
}

static int CompareTextEmptyLast(const std::wstring& a, const std::wstring& b) {
    if (a.empty() && !b.empty()) return 1;
    if (!a.empty() && b.empty()) return -1;

    std::wstring lowerA = ToLowerCopy(a);
    std::wstring lowerB = ToLowerCopy(b);
    if (lowerA < lowerB) return -1;
    if (lowerB < lowerA) return 1;
    return 0;
}

std::vector<WindowInfo> WindowManager::EnumerateWindows() {
    m_windows.clear();
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(this));
    SortWindows(m_windows);
    return m_windows;
}

void WindowManager::Refresh() {
    EnumerateWindows();
}

BOOL CALLBACK WindowManager::EnumWindowsProc(HWND hWnd, LPARAM lParam) {
    auto* pThis = reinterpret_cast<WindowManager*>(lParam);
    if (!IsCandidateWindow(hWnd))
        return TRUE;

    WindowInfo info;
    info.hWnd = hWnd;

    // 标题
    wchar_t buf[256] = {};
    GetWindowTextW(hWnd, buf, 255);
    info.title = buf;

    // 类名
    wchar_t classBuf[256] = {};
    GetClassNameW(hWnd, classBuf, 255);
    info.className = classBuf;

    // 进程 ID
    GetWindowThreadProcessId(hWnd, &info.processId);
    info.processName = GetProcessName(info.processId);

    // 屏幕坐标
    GetWindowRect(hWnd, &info.rect);

    // 客户区屏幕坐标
    GetClientRect(hWnd, &info.clientRect);
    POINT pt = { 0, 0 };
    ClientToScreen(hWnd, &pt);
    info.clientRect.left = pt.x;
    info.clientRect.top = pt.y;
    info.clientRect.right = pt.x + info.clientRect.right;
    info.clientRect.bottom = pt.y + info.clientRect.bottom;

    info.visible = IsWindowVisible(hWnd) && !IsIconic(hWnd);

    pThis->m_windows.push_back(std::move(info));
    return TRUE;
}

bool WindowManager::IsCandidateWindow(HWND hWnd) {
    if (!IsWindowVisible(hWnd))
        return false;

    LONG style = GetWindowLongW(hWnd, GWL_STYLE);
    LONG exStyle = GetWindowLongW(hWnd, GWL_EXSTYLE);

    // 跳过工具窗口
    if (exStyle & WS_EX_TOOLWINDOW)
        return false;

    // 跳过子窗口（只枚举顶层窗口）
    if (style & WS_CHILD)
        return false;

    // 必须有标题文字（过滤空标题系统窗口）
    wchar_t title[256] = {};
    GetWindowTextW(hWnd, title, 255);
    if (title[0] == L'\0')
        return false;

    // 过滤开始菜单、任务栏、桌面等系统 UI
    wchar_t className[128] = {};
    GetClassNameW(hWnd, className, 127);

    // 系统 UI 黑名单
    static const wchar_t* s_systemClasses[] = {
        L"Windows.UI.Core.CoreWindow",
        L"Progman",
        L"WorkerW",
        L"Shell_TrayWnd",
        L"Shell_SecondaryTrayWnd",
        L"TaskSwitcherWnd",
        L"Button",
        L"Static",
        L"ToolbarWindow32",
        L"ApplicationManager_DesktopShellWindow",
    };
    for (const auto* sysClass : s_systemClasses) {
        if (_wcsicmp(className, sysClass) == 0)
            return false;
    }

    // 尺寸过滤：太小的窗口不是游戏窗口（最小 200x150）
    RECT r;
    GetWindowRect(hWnd, &r);
    int w = r.right - r.left;
    int h = r.bottom - r.top;
    if (w < 200 || h < 150)
        return false;

    // 不再强制要求 WS_CAPTION —— 无边框/全屏窗口模式的游戏使用 WS_POPUP
    // 只要是有标题、有合理尺寸、非系统窗口的可见顶层窗口即可
    return true;
}

HWND WindowManager::GetForeground() const {
    return GetForegroundWindow();
}

RECT WindowManager::GetWindowScreenRect(HWND hWnd) {
    RECT r = {};
    GetWindowRect(hWnd, &r);
    return r;
}

std::wstring WindowManager::GetProcessName(DWORD processId) {
    std::wstring result;
    HANDLE hProcess = OpenProcess(
        PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (hProcess) {
        wchar_t path[MAX_PATH] = {};
        DWORD size = MAX_PATH;
        if (QueryFullProcessImageNameW(hProcess, 0, path, &size)) {
            // 从完整路径中提取文件名
            wchar_t* fileName = wcsrchr(path, L'\\');
            if (fileName)
                result = fileName + 1;  // 跳过反斜杠
            else
                result = path;
        }
        CloseHandle(hProcess);
    }
    return result;
}

void WindowManager::SortWindows(std::vector<WindowInfo>& windows) {
    std::sort(windows.begin(), windows.end(), WindowInfoLess);
}

bool WindowManager::WindowInfoLess(const WindowInfo& a, const WindowInfo& b) {
    int processCmp = CompareTextEmptyLast(a.processName, b.processName);
    if (processCmp != 0) return processCmp < 0;

    int titleCmp = CompareTextEmptyLast(a.title, b.title);
    if (titleCmp != 0) return titleCmp < 0;

    if (a.processId != b.processId)
        return a.processId < b.processId;

    return reinterpret_cast<uintptr_t>(a.hWnd) < reinterpret_cast<uintptr_t>(b.hWnd);
}
