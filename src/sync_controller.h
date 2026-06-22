#pragma once

#include <QObject>
#include <QString>
#include <vector>
#include "sync_engine.h"
#include "window_manager.h"

class SyncController : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString parentInfo READ parentInfo NOTIFY parentInfoChanged)
    Q_PROPERTY(QString childInfo READ childInfo NOTIFY childInfoChanged)
    Q_PROPERTY(bool canStart READ canStart NOTIFY canStartChanged)
    Q_PROPERTY(bool canStop READ canStop NOTIFY canStopChanged)

public:
    explicit SyncController(QObject* parent = nullptr);
    ~SyncController() override;

    bool isRunning() const;
    QString statusText() const;
    QString parentInfo() const;
    QString childInfo() const;
    bool canStart() const;
    bool canStop() const;

    Q_INVOKABLE void setParentHwnd(quintptr hwnd);
    Q_INVOKABLE void addChildHwnd(quintptr hwnd);
    Q_INVOKABLE void removeChildHwnd(quintptr hwnd);
    Q_INVOKABLE void clearChildren();
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void toggle();
    Q_INVOKABLE void refreshWindows();

    WindowManager& windowManager() { return m_windowManager; }

signals:
    void runningChanged();
    void statusTextChanged();
    void parentInfoChanged();
    void childInfoChanged();
    void canStartChanged();
    void canStopChanged();

private:
    void emitStateChanged();
    void updateInfoLabels();

    SyncEngine    m_engine;
    WindowManager m_windowManager;
    QString       m_statusText;
};
