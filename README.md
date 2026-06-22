# dx11_sync — DX11 游戏多窗口同步器 v2.0

一款 Windows 桌面工具，将**主窗口**的键盘/鼠标输入实时广播到多个**副窗口**。适用于 DirectX 11 游戏的多开 / 多窗口同步操控场景。

基于 Qt 6 + QML 构建，支持 Win11 Mica 毛玻璃外观，提供流畅的现代化 UI 体验。

## 功能特性

- **输入广播** — 键盘按键（含系统键）和鼠标（移动、点击、滚轮、侧键）从主窗口同步到所有副窗口
- **双过滤器** — 支持按窗口标题子串 AND/OR 进程名筛选候选窗口
- **多选副窗口** — 主窗口单选，副窗口支持多选，刷新列表后保留选择
- **热键开关** — `Ctrl+Shift+F12` 随时启停同步（F12 默认黑名单，避免注入自身）
- **坐标转换** — 鼠标屏幕坐标自动转换为各副窗口的客户区坐标
- **修饰键携带** — Ctrl、Shift、鼠标按钮状态通过 `WPARAM` 正确传递
- **视觉反馈** — 状态栏实时显示：同步中（绿色）/ 已停止（灰色）
- **窗口枚举** — 自动发现顶层可见窗口，过滤系统 UI（任务栏、桌面、工具窗口等）
- **Win11 Mica** — 启用 Mica 材质背景，搭配亚克力半透明面板，与系统外观融合
- **DPI 感知** — `PerMonitorV2` DPI 感知，高 DPI 下清晰渲染
- **管理员权限** — 清单声明 `requireAdministrator`（全局钩子所需），启动时自动 UAC 提权

## 截图

<!-- 可在此处放置截图 -->
<!-- ![主界面](assets/screenshot.png) -->

## 构建

### 依赖

| 组件 | 版本要求 | 说明 |
|------|---------|------|
| **Qt** | ≥ 6.5（推荐 6.11+） | Quick、QuickControls2 模块 |
| **CMake** | ≥ 3.16 | 构建系统 |
| **C++17 编译器** | MSVC 2019+ 或 MinGW-w64 13.1+ | |
| **系统库** | user32、gdi32、comctl32、dwmapi | Windows 自带，无需安装 |

零第三方 C++ 库依赖。

### 编译

项目提供了 `CMakePresets.json`，内含 MinGW-w64 + Qt 6.11.1 预设。请根据本机 Qt 路径修改预设中的 `CMAKE_PREFIX_PATH`。

```bash
# MinGW-w64（使用预设）
cmake --preset qt6-mingw
cmake --build --preset qt6-mingw

# MSVC（Visual Studio）
cmake -B build -G "Visual Studio 17 2022" ^
    -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2022_64"
cmake --build build --config Release
```

### 打包

构建完成后，使用 `scripts/package.ps1` 一键部署 Qt 依赖并打包：

```powershell
powershell -ExecutionPolicy Bypass -File scripts/package.ps1
```

该脚本执行 `windeployqt` 收集所需 DLL，并可选将产物打包为 `dx11_sync-windows-x64.zip`。

## 使用

1. 以**管理员身份**运行 `dx11_sync.exe`（清单自动触发 UAC 提权）
2. 先启动目标游戏，打开多个窗口
3. 在工具中输入窗口标题 / 进程名关键词，点击**刷新列表**
4. **选择一个主窗口**（单选）和**多个副窗口**（多选）
5. 点击**开始同步**，或按 `Ctrl+Shift+F12`
6. 激活主窗口并开始操作 — 输入将同步到所有副窗口
7. 再次按 `Ctrl+Shift+F12` 或点击**停止同步**结束

## 项目结构

```
dx11_sync/
├── CMakeLists.txt              # CMake 构建配置
├── CMakePresets.json           # CMake 预设（Qt 6 + MinGW + Ninja）
├── app.manifest                # Windows 清单（提权、DPI、Win10+ 兼容性）
├── resource.rc.in              # MinGW 资源模板（清单 + 版本信息）
├── scripts/
│   └── package.ps1             # 打包脚本（windeployqt + zip）
└── src/
    ├── main.cpp                # 入口：QGuiApplication + QML 引擎 + Mica 背景
    ├── qml/
    │   └── MainWindow.qml      # QML GUI 布局（~500 行）
    ├── sync_controller.h       # Qt 控制器接口
    ├── sync_controller.cpp     # QML ↔ 引擎 + 窗口管理器的桥接层
    ├── sync_engine.h           # 同步引擎接口
    ├── sync_engine.cpp         # 核心：全局钩子 + PostMessage 注入
    ├── window_manager.h        # 窗口枚举接口
    ├── window_manager.cpp      # EnumWindows + 过滤 + 进程名解析
    ├── window_list_model.h     # Qt 列表模型接口
    └── window_list_model.cpp   # QAbstractListModel，向 QML 暴露窗口列表
```

## 架构

```
QML UI (MainWindow.qml)
    ↕  Qt 属性绑定 + 信号/槽 + Context Property
SyncController (Qt C++ 适配层)
    ↕
├── WindowListModel → WindowManager → EnumWindows (Win32)
└── SyncEngine → SetWindowsHookEx + PostMessage (Win32)
```

- **SyncEngine** — 纯 Win32 核心（无 Qt 依赖），可复用于其他前端
- **SyncController** — 薄 `QObject` 封装，将引擎回调转为 QML 属性通知
- **WindowListModel** — `QAbstractListModel` 子类，将 `WindowInfo` 结构体暴露为 QML 列表角色
- **QML UI** — 使用 Win11 Mica 背景 + 亚克力半透明面板，渲染现代化界面

## 技术原理

- **输入捕获**：通过 `SetWindowsHookExW` 安装 `WH_KEYBOARD_LL` 和 `WH_MOUSE_LL` 全局低级钩子
- **条件注入**：仅在主窗口处于前台（`GetForegroundWindow`）时转发输入
- **消息发送**：使用 `PostMessageW` 异步注入 `WM_KEYDOWN/UP`、`WM_MOUSEMOVE`（限流 ~120 Hz）、鼠标按钮、滚轮等
- **坐标转换**：`ScreenToClient` 将屏幕坐标转为各副窗口客户区坐标
- **安全防护**：注入互斥锁（`m_inInject`）防止重入；F12 默认黑名单，避免热键注入自身

## 许可

未声明许可证。如需使用或分发，请联系作者。

## 作者

QQ：2055078975
