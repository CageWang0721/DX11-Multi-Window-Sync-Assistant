# dx11_sync — DX11 游戏多窗口同步器 v1.0

一款 Windows 原生桌面工具，将一个"主窗口"的键盘/鼠标输入实时镜像广播到多个"副窗口"。适用于 DirectX 11 游戏的多开 / 多窗口同步操控场景。

## 功能特性

- **输入广播** — 键盘按键（含系统键）和鼠标（移动、点击、滚轮、侧键）从主窗口同步到所有副窗口
- **双过滤器** — 支持按窗口标题子串 AND/OR 进程名（如 `game.exe`）筛选候选窗口
- **多选副窗口** — 主窗口单选，副窗口支持 `Ctrl+点击` 多选，刷新列表后保留选择
- **热键开关** — `Ctrl+Shift+F12` 随时启停同步（F12 默认不注入，避免误触）
- **坐标转换** — 鼠标屏幕坐标自动转换为各副窗口的客户区坐标
- **修饰键携带** — Ctrl、Shift、鼠标按钮状态通过 `WPARAM` 正确传递
- **视觉反馈** — 状态栏实时显示：同步中（绿色）/ 已停止（灰色）
- **窗口枚举** — 自动发现顶层可见窗口，过滤系统 UI（任务栏、桌面、工具窗口等）
- **单文件部署** — 静态链接 CRT，零运行时依赖，一份 `.exe` 即可运行
- **DPI 感知** — 清单声明 `PerMonitorV2`，高 DPI 下显示清晰
- **管理员权限** — 清单强制 `requireAdministrator`（全局钩子所需），启动时自动提权

## 截图

<!-- 可在此处放置截图 -->
<!-- ![主界面](assets/screenshot.png) -->

## 构建

### 依赖

- **CMake** ≥ 3.15
- **C++17** 编译器（MSVC 2019+ 或 MinGW-w64）
- **系统库**：`user32`、`gdi32`、`comctl32`（均为 Windows 自带，无需额外安装）
- **零第三方依赖**

### 编译

```bash
# MSVC (Visual Studio)
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release

# MSVC + Ninja
cmake -B build -G "Ninja"
cmake --build build

# MinGW-w64
cmake -B build -G "Ninja" -DCMAKE_CXX_COMPILER=g++
cmake --build build
```

产物为 `build/dx11_sync.exe`（Release 模式约几百 KB）。

## 使用

1. 以**管理员身份**运行 `dx11_sync.exe`（清单自动触发 UAC 提权）
2. 先启动目标游戏，打开多个窗口
3. 在工具中**刷新列表**，按标题 / 进程名筛选
4. **选择一个主窗口**（单选）和 **多个副窗口**（Ctrl+点击多选）
5. 点击 **开始同步**，或按 `Ctrl+Shift+F12`
6. 激活主窗口开始操作 — 输入将同步到所有副窗口
7. 再次按 `Ctrl+Shift+F12` 或点击 **停止同步** 结束

## 项目结构

```
dx11_sync/
├── CMakeLists.txt          # CMake 构建配置
├── app.manifest            # Windows 清单（提权、DPI、Win10+ 兼容性、Common Controls v6）
├── resource.rc.in          # MinGW 资源模板（CMake 处理）
└── src/
    ├── main.cpp            # WinMain 入口 + GUI 布局 + 控件事件
    ├── window_manager.h    # 窗口枚举 API
    ├── window_manager.cpp  # EnumWindows + 过滤 + 进程名解析
    ├── sync_engine.h       # 同步引擎 API
    └── sync_engine.cpp     # 全局低级钩子 + PostMessage 注入
```

## 技术原理

- **输入捕获**：通过 `SetWindowsHookExW` 安装 `WH_KEYBOARD_LL` 和 `WH_MOUSE_LL` 全局低级钩子
- **条件注入**：仅在主窗口处于前台（`GetForegroundWindow`）时转发
- **消息发送**：使用 `PostMessageW` 异步注入 `WM_KEYDOWN/UP`、`WM_MOUSEMOVE`（限流 ~120 Hz）、鼠标按钮、滚轮等消息
- **坐标转换**：`ScreenToClient` 将屏幕坐标转为各副窗口客户区坐标
- **安全防护**：注入互斥锁（`m_inInject`）防止重入；F12 默认黑名单，避免热键注入自身

## 许可

未声明许可证。如需使用或分发，请联系作者。

## 作者
QQ：2055078975

