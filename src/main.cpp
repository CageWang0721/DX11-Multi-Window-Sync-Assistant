// DX11 游戏多窗口同步器 — Win32 GUI

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

// MinGW 兼容：EM_SETCUEBANNER 可能未定义
#ifndef EM_SETCUEBANNER
#define EM_SETCUEBANNER 0x1501
#endif

#include <string>
#include <vector>
#include <algorithm>

#include "window_manager.h"
#include "sync_engine.h"

#pragma comment(lib, "comctl32.lib")

// ─── 资源常量 ────────────────────────────────────────────
#define IDC_PARENT_LIST     1001
#define IDC_CHILD_LIST      1002
#define IDC_BTN_REFRESH     1003
#define IDC_BTN_START       1004
#define IDC_BTN_STOP        1005
#define IDC_LBL_PARENT      1006
#define IDC_LBL_CHILD       1007
#define IDC_STATUS          1008
#define IDC_EDIT_FILTER     1009
#define IDC_PARENT_INFO     1011
#define IDC_CHILD_INFO      1012
#define IDC_EDIT_PROCFILTER 1013

// ─── 全局对象 ────────────────────────────────────────────
static WindowManager  g_windowMgr;
static SyncEngine     g_syncEngine;
static HWND           g_hWndMain       = nullptr;
static HWND           g_hParentList    = nullptr;
static HWND           g_hChildList     = nullptr;
static HWND           g_hBtnStart      = nullptr;
static HWND           g_hBtnStop       = nullptr;
static HWND           g_hStatus        = nullptr;
static HWND           g_hParentInfo    = nullptr;
static HWND           g_hChildInfo     = nullptr;
static HWND           g_hEditFilter    = nullptr;
static HWND           g_hEditProcFilter = nullptr;
static HFONT          g_hFont          = nullptr;
static HFONT          g_hFontBold      = nullptr;
static HBRUSH         g_hBrushGreen    = nullptr;
static HBRUSH         g_hBrushGray     = nullptr;

// ─── 控件尺寸常量 ────────────────────────────────────────
constexpr int MARGIN         = 12;
constexpr int PADDING        = 6;
constexpr int LIST_WIDTH     = 280;
constexpr int BTN_HEIGHT     = 36;
constexpr int LABEL_HEIGHT   = 20;
constexpr int INFO_HEIGHT    = 48;
constexpr int FILTER_HEIGHT  = 22;
constexpr int STATUS_HEIGHT  = 24;

// ─── 前向声明 ────────────────────────────────────────────
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
void     LayoutControls(HWND hWnd, int cx, int cy);
void     RefreshWindowLists();
void     UpdateSyncButtons();
void     UpdateInfoLabels();
void     StartSync();
void     StopSync();

// ─── WinMain ─────────────────────────────────────────────

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE,
                    _In_ LPWSTR, _In_ int nCmdShow)
{
    // 初始化公共控件
    INITCOMMONCONTROLSEX icc = { sizeof(INITCOMMONCONTROLSEX), ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&icc);

    // 注册主窗口类
    WNDCLASSEXW wc = {};
    wc.cbSize        = sizeof(WNDCLASSEXW);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = MainWndProc;
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursorW(nullptr, (LPCWSTR)IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"DX11SyncMainWnd";
    wc.hIcon         = LoadIconW(nullptr, (LPCWSTR)IDI_APPLICATION);
    RegisterClassExW(&wc);

    // 创建主窗口
    g_hWndMain = CreateWindowExW(
        0, L"DX11SyncMainWnd", L"DX11 多窗口同步器 v1.0",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 820, 600,
        nullptr, nullptr, hInstance, nullptr
    );
    if (!g_hWndMain) return 1;

    // 字体
    g_hFont = CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        L"Microsoft YaHei UI");
    g_hFontBold = CreateFontW(-15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        L"Microsoft YaHei UI");

    // 画刷（避免 WM_CTLCOLORSTATIC 泄漏）
    g_hBrushGreen = CreateSolidBrush(RGB(0, 160, 0));
    g_hBrushGray  = CreateSolidBrush(RGB(80, 80, 80));

    HWND hWnd = g_hWndMain;
    HINSTANCE hInst = hInstance;

    // ── 创建子控件 ──

    CreateWindowExW(0, L"STATIC", L"👤 父窗口 (单选)",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        0,0,0,0, hWnd, (HMENU)(UINT_PTR)IDC_LBL_PARENT, hInst, nullptr);

    g_hParentList = CreateWindowExW(0, L"LISTBOX", L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER |
        LBS_NOTIFY | LBS_HASSTRINGS,
        0,0,0,0, hWnd, (HMENU)(UINT_PTR)IDC_PARENT_LIST, hInst, nullptr);

    g_hParentInfo = CreateWindowExW(0, L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_SUNKEN,
        0,0,0,0, hWnd, (HMENU)(UINT_PTR)IDC_PARENT_INFO, hInst, nullptr);

    CreateWindowExW(0, L"STATIC", L"🎮 子窗口 (点击切换多选)",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        0,0,0,0, hWnd, (HMENU)(UINT_PTR)IDC_LBL_CHILD, hInst, nullptr);

    g_hChildList = CreateWindowExW(0, L"LISTBOX", L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER |
        LBS_NOTIFY | LBS_MULTIPLESEL | LBS_HASSTRINGS,
        0,0,0,0, hWnd, (HMENU)(UINT_PTR)IDC_CHILD_LIST, hInst, nullptr);

    g_hChildInfo = CreateWindowExW(0, L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_SUNKEN,
        0,0,0,0, hWnd, (HMENU)(UINT_PTR)IDC_CHILD_INFO, hInst, nullptr);

    g_hEditFilter = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        0,0,0,0, hWnd, (HMENU)(UINT_PTR)IDC_EDIT_FILTER, hInst, nullptr);
    // 过滤框提示文字
    SendMessageW(g_hEditFilter, EM_SETCUEBANNER, TRUE,
        (LPARAM)L"输入关键字过滤窗口...");

    // 进程名过滤
    g_hEditProcFilter = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        0,0,0,0, hWnd, (HMENU)(UINT_PTR)IDC_EDIT_PROCFILTER, hInst, nullptr);
    SendMessageW(g_hEditProcFilter, EM_SETCUEBANNER, TRUE,
        (LPARAM)L"进程名过滤 如: game.exe");

    CreateWindowExW(0, L"BUTTON", L"🔄 刷新窗口列表",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0,0,0,0, hWnd, (HMENU)(UINT_PTR)IDC_BTN_REFRESH, hInst, nullptr);

    g_hBtnStart = CreateWindowExW(0, L"BUTTON", L"▶ 开始同步",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0,0,0,0, hWnd, (HMENU)(UINT_PTR)IDC_BTN_START, hInst, nullptr);

    g_hBtnStop = CreateWindowExW(0, L"BUTTON", L"⏹ 终止同步",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0,0,0,0, hWnd, (HMENU)(UINT_PTR)IDC_BTN_STOP, hInst, nullptr);

    g_hStatus = CreateWindowExW(0, L"STATIC", L"就绪 — 请刷新窗口列表并选择父/子窗口",
        WS_CHILD | WS_VISIBLE | SS_CENTER | SS_SUNKEN,
        0,0,0,0, hWnd, (HMENU)(UINT_PTR)IDC_STATUS, hInst, nullptr);

    // ── 应用字体 ──
    SendMessageW(g_hParentList, WM_SETFONT, (WPARAM)g_hFont, TRUE);
    SendMessageW(g_hChildList,  WM_SETFONT, (WPARAM)g_hFont, TRUE);
    SendMessageW(g_hBtnStart,   WM_SETFONT, (WPARAM)g_hFont, TRUE);
    SendMessageW(g_hBtnStop,    WM_SETFONT, (WPARAM)g_hFont, TRUE);
    SendMessageW(g_hStatus,     WM_SETFONT, (WPARAM)g_hFont, TRUE);
    SendMessageW(g_hParentInfo, WM_SETFONT, (WPARAM)g_hFont, TRUE);
    SendMessageW(g_hChildInfo,  WM_SETFONT, (WPARAM)g_hFont, TRUE);
    SendMessageW(g_hEditFilter,     WM_SETFONT, (WPARAM)g_hFont, TRUE);
    SendMessageW(g_hEditProcFilter, WM_SETFONT, (WPARAM)g_hFont, TRUE);
    SendMessageW(GetDlgItem(hWnd, IDC_LBL_PARENT), WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
    SendMessageW(GetDlgItem(hWnd, IDC_LBL_CHILD),  WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
    SendMessageW(GetDlgItem(hWnd, IDC_BTN_REFRESH), WM_SETFONT, (WPARAM)g_hFont, TRUE);

    // ── 状态回调 ──
    g_syncEngine.SetStatusCallback([](const wchar_t* status) {
        SetWindowTextW(g_hStatus, status);
        InvalidateRect(g_hStatus, nullptr, TRUE);
    });

    // ── 显示 ──
    ShowWindow(g_hWndMain, nCmdShow);
    UpdateWindow(g_hWndMain);
    RefreshWindowLists();

    // ── 消息循环 ──
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    // ── 清理 ──
    g_syncEngine.Stop();
    DeleteObject(g_hFont);
    DeleteObject(g_hFontBold);
    DeleteObject(g_hBrushGreen);
    DeleteObject(g_hBrushGray);

    return (int)msg.wParam;
}

// ─── 窗口过程 ────────────────────────────────────────────

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_SIZE: {
        LayoutControls(hWnd, LOWORD(lParam), HIWORD(lParam));
        return 0;
    }

    case WM_GETMINMAXINFO: {
        auto* mmi = (MINMAXINFO*)lParam;
        mmi->ptMinTrackSize = { 720, 460 };
        return 0;
    }

    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        HWND hCtrl = (HWND)lParam;
        if (hCtrl == g_hStatus) {
            if (g_syncEngine.IsRunning()) {
                SetBkColor(hdc, RGB(0, 160, 0));
                SetTextColor(hdc, RGB(255, 255, 255));
                return (LRESULT)g_hBrushGreen;
            } else {
                SetBkColor(hdc, RGB(80, 80, 80));
                SetTextColor(hdc, RGB(240, 240, 240));
                return (LRESULT)g_hBrushGray;
            }
        }
        return 0;  // 其他控件使用默认颜色
    }

    case WM_COMMAND: {
        WORD id = LOWORD(wParam);
        WORD code = HIWORD(wParam);

        switch (id) {

        case IDC_BTN_REFRESH:
            if (code == BN_CLICKED) {
                g_syncEngine.ClearChildren();
                RefreshWindowLists();
            }
            break;

        case IDC_BTN_START:
            if (code == BN_CLICKED) StartSync();
            break;

        case IDC_BTN_STOP:
            if (code == BN_CLICKED) StopSync();
            break;

        case IDC_PARENT_LIST:
            if (code == LBN_SELCHANGE) {
                int sel = (int)SendMessageW(g_hParentList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    LRESULT data = SendMessageW(g_hParentList, LB_GETITEMDATA, sel, 0);
                    if (data != LB_ERR) {
                        const auto& windows = g_windowMgr.GetWindows();
                        size_t idx = (size_t)data;
                        if (idx < windows.size()) {
                            g_syncEngine.SetParent(windows[idx].hWnd);
                            UpdateInfoLabels();
                            UpdateSyncButtons();
                        }
                    }
                }
            }
            break;

        case IDC_CHILD_LIST:
            if (code == LBN_SELCHANGE) {
                g_syncEngine.ClearChildren();
                int count = (int)SendMessageW(g_hChildList, LB_GETCOUNT, 0, 0);
                const auto& windows = g_windowMgr.GetWindows();
                for (int i = 0; i < count; i++) {
                    if (SendMessageW(g_hChildList, LB_GETSEL, i, 0) > 0) {
                        LRESULT data = SendMessageW(g_hChildList, LB_GETITEMDATA, i, 0);
                        if (data != LB_ERR) {
                            size_t idx = (size_t)data;
                            if (idx < windows.size()) {
                                g_syncEngine.AddChild(windows[idx].hWnd);
                            }
                        }
                    }
                }
                UpdateInfoLabels();
                UpdateSyncButtons();
            }
            break;

        case IDC_EDIT_FILTER:
        case IDC_EDIT_PROCFILTER:
            if (code == EN_CHANGE) {
                RefreshWindowLists();
            }
            break;
        }
        return 0;
    }

    case WM_DESTROY:
        g_syncEngine.Stop();
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

// ─── 布局计算 ────────────────────────────────────────────

void LayoutControls(HWND hWnd, int cx, int cy) {
    int leftX  = MARGIN;
    int rightX = cx - MARGIN - LIST_WIDTH;

    // 标题过滤 + 进程过滤 (两行)
    int y = MARGIN;
    SetWindowPos(g_hEditFilter, nullptr,
        leftX, y, LIST_WIDTH, FILTER_HEIGHT, SWP_NOZORDER);
    SetWindowPos(g_hEditProcFilter, nullptr,
        leftX + LIST_WIDTH + 4, y, LIST_WIDTH, FILTER_HEIGHT, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hWnd, IDC_BTN_REFRESH), nullptr,
        rightX, y, LIST_WIDTH, FILTER_HEIGHT, SWP_NOZORDER);

    // 标签
    y += FILTER_HEIGHT + PADDING;
    SetWindowPos(GetDlgItem(hWnd, IDC_LBL_PARENT), nullptr,
        leftX, y, LIST_WIDTH, LABEL_HEIGHT, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hWnd, IDC_LBL_CHILD), nullptr,
        rightX, y, LIST_WIDTH, LABEL_HEIGHT, SWP_NOZORDER);

    // 列表
    y += LABEL_HEIGHT + PADDING;
    int listH = cy - y - INFO_HEIGHT - PADDING - BTN_HEIGHT - PADDING
                - STATUS_HEIGHT - MARGIN - PADDING;
    SetWindowPos(g_hParentList, nullptr,
        leftX, y, LIST_WIDTH, listH, SWP_NOZORDER);
    SetWindowPos(g_hChildList, nullptr,
        rightX, y, LIST_WIDTH, listH, SWP_NOZORDER);

    // 信息区
    y += listH + PADDING;
    SetWindowPos(g_hParentInfo, nullptr,
        leftX, y, LIST_WIDTH, INFO_HEIGHT, SWP_NOZORDER);
    SetWindowPos(g_hChildInfo, nullptr,
        rightX, y, LIST_WIDTH, INFO_HEIGHT, SWP_NOZORDER);

    // 按钮
    y += INFO_HEIGHT + PADDING;
    int btnW = (LIST_WIDTH - PADDING) / 2;
    SetWindowPos(g_hBtnStart, nullptr,
        leftX, y, btnW, BTN_HEIGHT, SWP_NOZORDER);
    SetWindowPos(g_hBtnStop, nullptr,
        leftX + btnW + PADDING, y, btnW, BTN_HEIGHT, SWP_NOZORDER);

    // 状态栏
    SetWindowPos(g_hStatus, nullptr,
        MARGIN, cy - STATUS_HEIGHT - MARGIN,
        cx - MARGIN * 2, STATUS_HEIGHT, SWP_NOZORDER);
}

// ─── 刷新窗口列表 ────────────────────────────────────────

void RefreshWindowLists() {
    g_windowMgr.Refresh();
    const auto& windows = g_windowMgr.GetWindows();

    // 获取过滤文本
    wchar_t filter[256] = {};
    GetWindowTextW(g_hEditFilter, filter, 255);
    std::wstring titleFilter(filter);
    std::transform(titleFilter.begin(), titleFilter.end(), titleFilter.begin(), ::towlower);

    // 进程名过滤
    wchar_t procFilter[256] = {};
    GetWindowTextW(g_hEditProcFilter, procFilter, 255);
    std::wstring procFilterStr(procFilter);
    std::transform(procFilterStr.begin(), procFilterStr.end(), procFilterStr.begin(), ::towlower);

    // 保存当前选择
    HWND curParent = g_syncEngine.GetParent();
    std::vector<HWND> curChildren = g_syncEngine.GetChildren();

    // 清空
    SendMessageW(g_hParentList, LB_RESETCONTENT, 0, 0);
    SendMessageW(g_hChildList,  LB_RESETCONTENT, 0, 0);

    int parentSel = -1;
    int shownCount = 0;

    for (size_t i = 0; i < windows.size(); i++) {
        const auto& w = windows[i];

        // 标题过滤
        if (!titleFilter.empty()) {
            std::wstring title(w.title);
            std::transform(title.begin(), title.end(), title.begin(), ::towlower);
            if (title.find(titleFilter) == std::wstring::npos)
                continue;
        }

        // 进程名过滤
        if (!procFilterStr.empty()) {
            std::wstring pn(w.processName);
            std::transform(pn.begin(), pn.end(), pn.begin(), ::towlower);
            if (pn.find(procFilterStr) == std::wstring::npos)
                continue;
        }

        // 显示格式: "[exe] 标题  [PID:xxx]"
        wchar_t display[512];
        if (!w.processName.empty())
            swprintf_s(display, L"[%s] %s  [PID:%lu]",
                       w.processName.c_str(), w.title.c_str(), w.processId);
        else
            swprintf_s(display, L"%s  [PID:%lu]",
                       w.title.c_str(), w.processId);

        // 父窗口列表
        int idx = (int)SendMessageW(g_hParentList, LB_ADDSTRING, 0, (LPARAM)display);
        SendMessageW(g_hParentList, LB_SETITEMDATA, idx, (LPARAM)i);
        if (w.hWnd == curParent) parentSel = idx;

        // 子窗口列表
        idx = (int)SendMessageW(g_hChildList, LB_ADDSTRING, 0, (LPARAM)display);
        SendMessageW(g_hChildList, LB_SETITEMDATA, idx, (LPARAM)i);

        // 恢复子窗口选择
        for (HWND h : curChildren) {
            if (w.hWnd == h)
                SendMessageW(g_hChildList, LB_SETSEL, TRUE, idx);
        }
        shownCount++;
    }

    if (parentSel >= 0)
        SendMessageW(g_hParentList, LB_SETCURSEL, parentSel, 0);

    UpdateInfoLabels();
    UpdateSyncButtons();

    wchar_t buf[128];
    swprintf_s(buf, L"显示 %d 个窗口 (共 %zu 个)", shownCount, windows.size());
    SetWindowTextW(g_hStatus, buf);
}

// ─── 信息标签 ────────────────────────────────────────────

void UpdateInfoLabels() {
    HWND hParent = g_syncEngine.GetParent();
    if (hParent && IsWindow(hParent)) {
        const auto& windows = g_windowMgr.GetWindows();
        for (const auto& w : windows) {
            if (w.hWnd == hParent) {
                RECT r = w.rect;
                wchar_t buf[256];
                swprintf_s(buf,
                    L"已选: %s\n类名: %s | %dx%d 位置: (%d,%d)",
                    w.title.c_str(), w.className.c_str(),
                    r.right - r.left, r.bottom - r.top,
                    r.left, r.top);
                SetWindowTextW(g_hParentInfo, buf);
                break;
            }
        }
    } else {
        SetWindowTextW(g_hParentInfo, L"未选择父窗口");
    }

    const auto& children = g_syncEngine.GetChildren();
    if (!children.empty()) {
        wchar_t buf[256];
        swprintf_s(buf, L"已选 %zu 个子窗口", children.size());
        SetWindowTextW(g_hChildInfo, buf);
    } else {
        SetWindowTextW(g_hChildInfo, L"未选择子窗口");
    }
}

// ─── 按钮状态 ────────────────────────────────────────────

void UpdateSyncButtons() {
    bool hasParent   = g_syncEngine.GetParent() != nullptr;
    bool hasChildren = !g_syncEngine.GetChildren().empty();
    bool running     = g_syncEngine.IsRunning();

    EnableWindow(g_hBtnStart, hasParent && hasChildren && !running);
    EnableWindow(g_hBtnStop,  running);
}

// ─── 开始/停止 ──────────────────────────────────────────

void StartSync() {
    if (g_syncEngine.Start()) {
        UpdateSyncButtons();
        InvalidateRect(g_hStatus, nullptr, TRUE);
    }
}

void StopSync() {
    g_syncEngine.Stop();
    UpdateSyncButtons();
    InvalidateRect(g_hStatus, nullptr, TRUE);
}
