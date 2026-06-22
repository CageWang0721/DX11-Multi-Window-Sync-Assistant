#include "sync_controller.h"

SyncController::SyncController(QObject* parent)
    : QObject(parent)
{
    m_statusText = QStringLiteral("就绪 — 请刷新窗口列表并选择父/子窗口");

    // 引擎状态回调 → 更新 QML 属性
    m_engine.SetStatusCallback([this](const wchar_t* status) {
        m_statusText = QString::fromWCharArray(status);
        emit statusTextChanged();
        emit runningChanged();
        emit canStartChanged();
        emit canStopChanged();
    });
}

SyncController::~SyncController() {
    m_engine.Stop();
}

bool SyncController::isRunning() const {
    return m_engine.IsRunning();
}

QString SyncController::statusText() const {
    return m_statusText;
}

QString SyncController::parentInfo() const {
    HWND hParent = m_engine.GetParent();
    if (!hParent || !IsWindow(hParent))
        return QStringLiteral("未选择父窗口");

    const auto& windows = m_windowManager.GetWindows();
    for (const auto& w : windows) {
        if (w.hWnd == hParent) {
            int ww = w.rect.right - w.rect.left;
            int wh = w.rect.bottom - w.rect.top;
            return QStringLiteral("已选: %1\n类名: %2 | %3×%4  位置: (%5, %6)")
                .arg(QString::fromStdWString(w.title),
                     QString::fromStdWString(w.className))
                .arg(ww).arg(wh)
                .arg(w.rect.left).arg(w.rect.top);
        }
    }
    return QStringLiteral("父窗口已失效");
}

QString SyncController::childInfo() const {
    const auto& children = m_engine.GetChildren();
    if (children.empty())
        return QStringLiteral("未选择子窗口");
    return QStringLiteral("已选 %1 个子窗口").arg(static_cast<int>(children.size()));
}

bool SyncController::canStart() const {
    return m_engine.GetParent() != nullptr &&
           !m_engine.GetChildren().empty() &&
           !m_engine.IsRunning();
}

bool SyncController::canStop() const {
    return m_engine.IsRunning();
}

// ─── 选择操作 ──────────────────────────────────────────

void SyncController::setParentHwnd(quintptr hwnd) {
    m_engine.SetParent(reinterpret_cast<HWND>(hwnd));
    updateInfoLabels();
}

void SyncController::addChildHwnd(quintptr hwnd) {
    m_engine.AddChild(reinterpret_cast<HWND>(hwnd));
    updateInfoLabels();
}

void SyncController::removeChildHwnd(quintptr hwnd) {
    m_engine.RemoveChild(reinterpret_cast<HWND>(hwnd));
    updateInfoLabels();
}

void SyncController::clearChildren() {
    m_engine.ClearChildren();
    updateInfoLabels();
}

// ─── 启停 ──────────────────────────────────────────────

void SyncController::start() {
    m_engine.Start();
    emitStateChanged();
}

void SyncController::stop() {
    m_engine.Stop();
    emitStateChanged();
}

void SyncController::toggle() {
    if (m_engine.IsRunning())
        m_engine.Stop();
    else
        m_engine.Start();
    emitStateChanged();
}

void SyncController::refreshWindows() {
    m_windowManager.Refresh();
    updateInfoLabels();
}

// ─── 内部 ──────────────────────────────────────────────

void SyncController::emitStateChanged() {
    emit runningChanged();
    emit canStartChanged();
    emit canStopChanged();
}

void SyncController::updateInfoLabels() {
    emit parentInfoChanged();
    emit childInfoChanged();
    emitStateChanged();
}
