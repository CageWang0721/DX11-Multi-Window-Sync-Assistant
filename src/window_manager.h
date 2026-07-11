#pragma once

#include <windows.h>
#include <vector>
#include <string>

struct WindowInfo {
    HWND hWnd;
    std::wstring title;
    std::wstring className;
    std::wstring processName;  // exe 文件名
    DWORD processId;
    RECT rect;
    RECT clientRect;
    bool visible;
};

class WindowManager {
public:
    // 枚举所有可见顶层窗口
    std::vector<WindowInfo> EnumerateWindows();

    // 刷新缓存
    void Refresh();

    // 获取当前缓存的窗口列表
    const std::vector<WindowInfo>& GetWindows() const { return m_windows; }

    // 获取前台窗口
    HWND GetForeground() const;

    // 获取窗口屏幕矩形
    static RECT GetWindowScreenRect(HWND hWnd);

    // 判断窗口是否候选
    static bool IsCandidateWindow(HWND hWnd);

    // 获取进程名
    static std::wstring GetProcessName(DWORD processId);

    // 稳定排序窗口列表，避免依赖 EnumWindows 的系统枚举顺序
    static void SortWindows(std::vector<WindowInfo>& windows);
    static bool WindowInfoLess(const WindowInfo& a, const WindowInfo& b);

private:
    static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam);
    std::vector<WindowInfo> m_windows;
};
