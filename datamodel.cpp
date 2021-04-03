#include "datamodel.h"
#include <QDebug>
#include <QFile>

DataModel::DataModel(QObject* parent)
    : QAbstractTableModel(parent) //
{
    QFile file("new 1.txt");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        QString line;
        m_data.reserve(1000000);
        while (stream.readLineInto(&line)) {
            auto list { line.split('\t') };
            m_data.push_back({ list.front().toDouble(), list.back().toDouble() });
        }
        m_data.shrink_to_fit();
    }
}

DataModel::~DataModel() { qDebug(__FUNCTION__); }

int DataModel::rowCount(const QModelIndex&) const { return m_data.size(); }

int DataModel::columnCount(const QModelIndex&) const { return ColumnCount; }

QVariant DataModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole)
        return index.column() ? m_data[index.row()].x()
                              : m_data[index.row()].y();
    else if (role == Qt::TextAlignmentRole)
        return Qt::AlignCenter;
    return {};
}

bool DataModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    return {};
}

QVariant DataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return QChar { "XY"[section] };
    return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags DataModel::flags(const QModelIndex& index) const
{
    return QAbstractTableModel::flags(index);
}
