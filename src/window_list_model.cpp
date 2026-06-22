#include "window_list_model.h"
#include <algorithm>
#include <string>

WindowListModel::WindowListModel(QObject* parent)
    : QAbstractListModel(parent) {}

int WindowListModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_windows.size());
}

QVariant WindowListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_windows.size()))
        return {};

    const auto& w = m_windows[index.row()];

    switch (role) {
    case Qt::DisplayRole:
    case TitleRole: {
        // 显示格式: "[exe] 标题"
        QString display;
        if (!w.processName.empty())
            display = QStringLiteral("[%1] %2").arg(
                QString::fromStdWString(w.processName),
                QString::fromStdWString(w.title));
        else
            display = QString::fromStdWString(w.title);
        return display;
    }
    case ProcessNameRole:
        return QString::fromStdWString(w.processName);
    case ProcessIdRole:
        return QVariant::fromValue(static_cast<quint32>(w.processId));
    case ClassNameRole:
        return QString::fromStdWString(w.className);
    case HwndRole:
        return QVariant::fromValue(reinterpret_cast<quintptr>(w.hWnd));
    case RectLeftRole:
        return static_cast<int>(w.rect.left);
    case RectTopRole:
        return static_cast<int>(w.rect.top);
    case RectRightRole:
        return static_cast<int>(w.rect.right);
    case RectBottomRole:
        return static_cast<int>(w.rect.bottom);
    default:
        return {};
    }
}

QHash<int, QByteArray> WindowListModel::roleNames() const {
    return {
        { TitleRole,       "title" },
        { ProcessNameRole, "processName" },
        { ProcessIdRole,   "processId" },
        { ClassNameRole,   "className" },
        { HwndRole,        "hwnd" },
        { RectLeftRole,    "rectLeft" },
        { RectTopRole,     "rectTop" },
        { RectRightRole,   "rectRight" },
        { RectBottomRole,  "rectBottom" },
    };
}

void WindowListModel::refresh(const QString& titleFilter, const QString& procFilter) {
    beginResetModel();

    m_manager.Refresh();
    const auto& allWindows = m_manager.GetWindows();

    m_windows.clear();

    // 转小写用于过滤
    std::wstring titleFilterLower = titleFilter.toLower().toStdWString();
    std::wstring procFilterLower  = procFilter.toLower().toStdWString();

    for (const auto& w : allWindows) {
        // 标题过滤
        if (!titleFilterLower.empty()) {
            std::wstring title(w.title);
            std::transform(title.begin(), title.end(), title.begin(), ::towlower);
            if (title.find(titleFilterLower) == std::wstring::npos)
                continue;
        }

        // 进程名过滤
        if (!procFilterLower.empty()) {
            std::wstring pn(w.processName);
            std::transform(pn.begin(), pn.end(), pn.begin(), ::towlower);
            if (pn.find(procFilterLower) == std::wstring::npos)
                continue;
        }

        m_windows.push_back(w);
    }

    endResetModel();
}

HWND WindowListModel::hwndAt(int row) const {
    if (row < 0 || row >= static_cast<int>(m_windows.size()))
        return nullptr;
    return m_windows[row].hWnd;
}
