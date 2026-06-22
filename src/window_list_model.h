#pragma once

#include <QAbstractListModel>
#include <vector>
#include "window_manager.h"

class WindowListModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        TitleRole       = Qt::UserRole + 1,
        ProcessNameRole,
        ProcessIdRole,
        ClassNameRole,
        HwndRole,           // quintptr (HWND)
        RectLeftRole,
        RectTopRole,
        RectRightRole,
        RectBottomRole,
    };
    Q_ENUM(Roles)

    explicit WindowListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refresh(const QString& titleFilter = QString(),
                             const QString& procFilter = QString());

    // 通过行号获取 HWND（供 SyncController 调用）
    HWND hwndAt(int row) const;

private:
    std::vector<WindowInfo> m_windows;
    WindowManager m_manager;
};
