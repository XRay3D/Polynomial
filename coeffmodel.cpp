#include "coeffmodel.h"

CoeffModel::CoeffModel(QObject* parent)
    : QAbstractListModel(parent) {
}

int CoeffModel::rowCount(const QModelIndex&) const { return m_data.size(); }

int CoeffModel::columnCount(const QModelIndex&) const { return 1; }

QVariant CoeffModel::data(const QModelIndex& index, int role) const {
    if(role == Qt::DisplayRole)
        return m_data[index.row()];
    else if(role == Qt::TextAlignmentRole)
        return Qt::AlignCenter;
    return {};
}

void CoeffModel::setData(const std::vector<double>& data) {
    if(m_data.size()) {
        beginRemoveRows({}, 0, m_data.size() - 1);
        m_data.clear();
        endRemoveRows();
    }
    if(data.size()) {
        beginInsertRows({}, 0, data.size() - 1);
        m_data = data;
        endInsertRows();
    }
}
