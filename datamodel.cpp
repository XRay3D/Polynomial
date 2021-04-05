#include "datamodel.h"

#include "ctre.hpp"

#include <QDataStream>
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

DataModel::DataModel(QObject* parent)
    : QAbstractTableModel(parent) //
{
    m_data.reserve(1000000);
}

DataModel::~DataModel() { qDebug(__FUNCTION__); }

int DataModel::rowCount(const QModelIndex&) const { return m_data.size(); }

int DataModel::columnCount(const QModelIndex&) const { return ColumnCount; }

QVariant DataModel::data(const QModelIndex& index, int role) const {
    if(role == Qt::DisplayRole)
        switch(index.column()) {
        case ColumnX:
            return QString::number(m_data[index.row()].x(), 'G', m_precision).replace('.', ',');
        case ColumnY:
            return QString::number(m_data[index.row()].y(), 'G', m_precision).replace('.', ',');
        case ColumnComp:
            return QString::number(m_dataComp[index.row()].y(), 'G', m_precision).replace('.', ',');
        case ColumnDelta:
            return QString::number(m_data[index.row()].y() - m_dataComp[index.row()].y(), 'G', m_precision).replace('.', ',');
        }
    else if(role == Qt::EditRole)
        switch(index.column()) {
        case ColumnX:
            return m_data[index.row()].x();
        case ColumnY:
            return m_data[index.row()].y();
        case ColumnComp:
            return m_dataComp[index.row()].y();
        case ColumnDelta:
            return m_data[index.row()].y() - m_dataComp[index.row()].y();
        }
    else if(role == Qt::TextAlignmentRole)
        return Qt::AlignCenter;
    return {};
}

bool DataModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if(role == Qt::EditRole)
        switch(index.column()) {
        case ColumnX:
            m_data[index.row()].setX(value.toDouble());
            emit dataChanged_(m_data);
            return true;
        case ColumnY:
            m_data[index.row()].setY(value.toDouble());
            emit dataChanged_(m_data);
            return true;
        default:
            return {};
        }
    return {};
}

QVariant DataModel::headerData(int section, Qt::Orientation orientation, int role) const {
    static QStringList hd{"X", "Y", "Расчёт", "Δ"};
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return hd[section];
    return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags DataModel::flags(const QModelIndex& index) const {
    if(index.column() < ColumnComp)
        return Qt::ItemIsEnabled | Qt::ItemIsEditable;
    return Qt::ItemIsEnabled;
}

const Data& DataModel::data() const noexcept { return m_data; }

void DataModel::setNewData(const Data& data) {
    int maxIndex = std::min(data.size(), m_dataComp.size());
    for(int i = 0; i < maxIndex; ++i)
        m_dataComp[i].ry() = data[i].y();
    emit dataChanged(createIndex(0, ColumnComp), createIndex(m_data.size() - 1, ColumnComp), {Qt::DisplayRole});
}

void DataModel::setPrecision(int prec) {
    m_precision = prec;
    emit dataChanged(createIndex(0, 0), createIndex(m_data.size() - 1, 1), {Qt::DisplayRole});
}

void DataModel::load(const QString& fileName) {
    if(QFile file(fileName); file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        auto lastSize{m_data.size()};
        m_data.clear();
        m_dataComp.clear();
        if(0) {
            Timer{__FUNCTION__};
            QString line;
            QTextStream stream(&file);
            while(stream.readLineInto(&line)) {
                auto list{line.split(QRegularExpression{"\\s"})};
                m_data.push_back({list.value(0).toDouble(), list.value(1).toDouble()});
                m_dataComp.push_back({m_data.back().x(), list.value(2).toDouble()});
            }
        } else {
            Timer{__FUNCTION__};
            QByteArray fileData(file.readAll());
            static constexpr auto pattern = ctll::fixed_string{"(\\S+)(?:\\s+(\\S+))?(?:\\s+(\\S+))?\n"};
            auto range = ctre::range<pattern>(std::string_view(fileData.data(), fileData.size()));
            for(auto [wholeStr, xStr, yStr, zStr] : range) {
                m_data.push_back({//
                                  xStr.size() ? QByteArray{xStr.data(), static_cast<int>(xStr.size())}.toDouble() : 0.0,
                                  yStr.size() ? QByteArray{yStr.data(), static_cast<int>(yStr.size())}.toDouble() : 0.0});
                m_dataComp.push_back({m_data.back().x(), zStr.size() ? QByteArray{zStr.data(), static_cast<int>(zStr.size())}.toDouble() : 0.0});
            }
        }

        if(lastSize > m_data.size())
            emit rowsRemoved({}, m_data.size(), lastSize - 1, {});
        else if(lastSize < m_data.size())
            emit rowsInserted({}, lastSize, m_data.size() - 1, {});
        else
            emit dataChanged(createIndex(0, ColumnX), createIndex(m_data.size() - 1, ColumnX), {Qt::DisplayRole});
        emit dataChanged_(m_data);
    }
}

void DataModel::save(const QString& fileName) const {
    qDebug(__FUNCTION__);
    if(QFile file(fileName); file.open(QIODevice::WriteOnly)) {
        if(fileName.endsWith("bin")) {
            QDataStream out(&file);
            out << uint32_t(m_data.size());
            for(auto&& val : m_data)
                out << val.x() << val.y();
        } else {
            QTextStream out(&file);
            for(int i = 0; i < m_data.size(); ++i)
                out << QString("%1\t%2\t%3\n")
                           .arg(m_data[i].x(), 0, 'G', m_precision)
                           .arg(m_data[i].y(), 0, 'G', m_precision)
                           .arg(m_dataComp[i].y(), 0, 'G', m_precision);
        }
    }
}

QString DataModel::copy() const {
    QString cpyStr;
    for(int row = 0; row < rowCount(); ++row) {
        cpyStr += QString::number(m_data[row].x(), 'G', m_precision).replace('.', ',') + '\t';
        cpyStr += QString::number(m_data[row].y(), 'G', m_precision).replace('.', ',') + '\t';
        cpyStr += QString::number(m_dataComp[row].y(), 'G', m_precision).replace('.', ',') + '\t';
        cpyStr += QString::number(m_data[row].y() - m_dataComp[row].y(), 'G', m_precision).replace('.', ',') + '\n';
    }
    return cpyStr;
}

void DataModel::paste(QString&& clipboardStr) {
    if(clipboardStr.isEmpty())
        return;
    clipboardStr.replace(',', '.');
    auto clipboardList{clipboardStr.split('\n', QString::SkipEmptyParts)};
    auto lastSize{m_data.size()};
    m_data.resize(clipboardList.size());
    m_dataComp.resize(clipboardList.size());

    for(int row = 0; row < m_data.size(); ++row) {
        auto&& list = clipboardList[row].split(QRegularExpression("\\s"));
        m_data[row].rx() = list.value(0).toDouble();
        m_data[row].ry() = list.value(1).toDouble();
        m_dataComp[row].ry() = list.value(2).toDouble();
    }

    if(lastSize > m_data.size())
        emit rowsRemoved({}, m_data.size(), lastSize - 1, {});
    else if(lastSize < m_data.size())
        emit rowsInserted({}, lastSize, m_data.size() - 1, {});
    else
        emit dataChanged(createIndex(0, ColumnX), createIndex(m_data.size() - 1, ColumnX), {Qt::DisplayRole});
    emit dataChanged_(m_data);
}
