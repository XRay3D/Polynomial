#include "coeffmodel.h"

CoeffModel::CoeffModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int CoeffModel::rowCount(const QModelIndex&) const { return m_data.size(); }

int CoeffModel::columnCount(const QModelIndex&) const { return 1; }

QVariant CoeffModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole)
        return QString::number(m_data[index.row()], 'G', m_prec).replace('.', ',');
    else if (role == Qt::TextAlignmentRole)
        return Qt::AlignCenter;
    return {};
}

QVariant CoeffModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    static const QString chars("⁰¹²³⁴⁵⁶⁷⁸⁹");
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal)
            return "Коэффициенты";
        else {
            switch (section) {
            case 0:
                return "A0";
            case 1:
                return "A1x";
            default: {
                auto str { QString("A%1x").arg(section) };
                for (char c : QByteArray::number(section))
                    str += chars[c - '0'];
                return str;
            }
            }
        }
    }
    if (role == Qt::TextAlignmentRole)
        return Qt::AlignCenter;
    return QAbstractTableModel::headerData(section, orientation, role);
}

void CoeffModel::setData(const std::vector<double>& data)
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
        m_data[i] = data[i];
    }
}

void CoeffModel::setPrec(int prec)
{
    m_prec = prec;
    emit dataChanged(createIndex(0, 0), createIndex(m_data.size() - 1, 1), { Qt::DisplayRole });
}

void CoeffModel::save(const QString& fileName)
{
    //    auto lastSize { m_data.size() };
    //    QFile file("data.txt");
    //    if (file.open(QIODevice::ReadOnly)) {
    //        QTextStream stream(&file);
    //        QString line;
    //        m_data.clear();

    //        while (stream.readLineInto(&line)) {
    //            auto list { line.split('\t') };
    //            m_data.push_back({ list.front().toDouble(), list.back().toDouble() });
    //        }
    //    }
    //    if (lastSize > m_data.size())
    //        emit rowsRemoved({}, m_data.size(), lastSize - 1, {});
    //    else if (lastSize > m_data.size())
    //        emit rowsInserted({}, lastSize, m_data.size() - 1, {});
    //    else
    //        emit dataChanged(createIndex(0, X), createIndex(m_data.size() - 1, X), { Qt::DisplayRole });
}
