// DX11 多窗口同步器 — Qt 6 + QML GUI
//
// 保持原有 sync_engine / window_manager 核心逻辑不变，
// 仅将 Win32 GDI 界面替换为 Qt Quick (QML) 渲染。

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>

#include <windows.h>
#include <dwmapi.h>

#include "window_list_model.h"
#include "sync_controller.h"

// Win11 Mica 背景材质
static void applyMica(QQuickWindow* window) {
    if (!window) return;

    HWND hwnd = reinterpret_cast<HWND>(window->winId());

    // 尝试 Win11 22H2+ Mica (首选)
    // DWMWA_SYSTEMBACKDROP_TYPE = 38, DWMSBT_MAINWINDOW = 2
    constexpr DWORD DWMWA_BACKDROP = 38;
    constexpr DWORD DWMSBT_MICA    = 2;
    HRESULT hr = DwmSetWindowAttribute(hwnd, DWMWA_BACKDROP,
                                       &DWMSBT_MICA, sizeof(DWORD));

    if (FAILED(hr)) {
        // 回退：Win11 21H2 Mica (DWMWA_MICA = 1029)
        constexpr DWORD DWMWA_MICA = 1029;
        BOOL enable = TRUE;
        DwmSetWindowAttribute(hwnd, DWMWA_MICA, &enable, sizeof(BOOL));
    }

    // 深色标题栏（与亮色 Mica 搭配）
    BOOL darkMode = FALSE;
    DwmSetWindowAttribute(hwnd, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */,
                          &darkMode, sizeof(BOOL));

    // 圆角（Win11 默认已有，显式确保）
    constexpr DWORD DWMWA_WINDOW_CORNER_PREFERENCE = 33;
    constexpr DWORD DWMWCP_ROUND = 2;
    DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE,
                          &DWMWCP_ROUND, sizeof(DWORD));
}

int main(int argc, char* argv[]) {
    // ── Qt 初始化（Alpha 通道，供 Mica 透出） ──────────
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("DX11 多窗口同步器"));
    app.setApplicationVersion(QStringLiteral("2.0"));
    app.setOrganizationName(QStringLiteral("dx11sync"));

    // ── 核心模型 ──────────────────────────────────────
    WindowListModel windowModel;
    SyncController   syncController;

    // 初始刷新窗口列表
    windowModel.refresh();

    // ── QML 引擎 ──────────────────────────────────────
    QQmlApplicationEngine engine;

    // 暴露 C++ 对象到 QML
    engine.rootContext()->setContextProperty("windowModel",    &windowModel);
    engine.rootContext()->setContextProperty("syncController", &syncController);

    // QML 加载失败时退出
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.load(QUrl(QStringLiteral("qrc:/dx11sync/src/qml/MainWindow.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    // ── Win11 Mica 材质 ───────────────────────────────
    QQuickWindow* rootWindow =
        qobject_cast<QQuickWindow*>(engine.rootObjects().first());
    applyMica(rootWindow);

    // ── 事件循环（同时驱动 Windows 消息泵，供钩子使用） ──
    return app.exec();
}
