#include "coeffmodel.h"

CoeffModel::CoeffModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int CoeffModel::rowCount(const QModelIndex&) const { return m_data.size(); }

int CoeffModel::columnCount(const QModelIndex&) const { return 2; }

QVariant CoeffModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole)
        return QString::number(m_data[index.row()][index.column()], 'G', m_prec).replace('.', ',');
    else if (role == Qt::TextAlignmentRole)
        return Qt::AlignCenter;
    return {};
}

QVariant CoeffModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    static const auto hd = { "Compile\nTime", "Run\nTime" };
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal)
            return hd.begin()[section];
        else {
            switch (section) {
            case 0:
                return "A0";
            case 1:
                return "A1x";
            default:
                return QString("A%1x^%1").arg(section);
            }
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

void CoeffModel::setData(const std::vector<double>& data, int column)
{
    if (data.size() < m_data.size()) {
        beginRemoveRows({}, data.size(), m_data.size() - 1);
        m_data.resize(data.size());
        endRemoveRows();
    } else if (data.size() > m_data.size()) {
        beginInsertRows({}, m_data.size(), data.size() - 1);
        m_data.resize(data.size());
        endInsertRows();
    }
    for (size_t i = 0; i < data.size(); ++i) {
        m_data[i][column] = data[i];
    }
}

void CoeffModel::setPrec(int prec)
{
    m_prec = prec;
    emit dataChanged(createIndex(0, 0), createIndex(m_data.size() - 1, 1), { Qt::DisplayRole });
}
