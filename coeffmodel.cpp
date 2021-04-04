#include "coeffmodel.h"

#include "ctre.hpp"

#include <QDataStream>
#include <QFile>
#include <QTextStream>

CoeffModel::CoeffModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int CoeffModel::rowCount(const QModelIndex&) const { return static_cast<int>(m_data.size()); }

int CoeffModel::columnCount(const QModelIndex&) const { return 1; }

QVariant CoeffModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole)
        return QString::number(m_data[index.row()], 'G', m_precision).replace('.', ',');
    else if (role == Qt::EditRole)
        return m_data[index.row()];
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

void CoeffModel::setCoeffData(const Degrees& data)
{
    auto lastSize { static_cast<int>(m_data.size()) };
    m_data = data;
    if (lastSize > static_cast<int>(m_data.size()))
        emit rowsRemoved({}, static_cast<int>(m_data.size()), lastSize - 1, {});
    else if (lastSize < static_cast<int>(m_data.size()))
        emit rowsInserted({}, lastSize, static_cast<int>(m_data.size()) - 1, {});
    else
        emit dataChanged(createIndex(0, 0), createIndex(static_cast<int>(m_data.size()) - 1, 0), { Qt::DisplayRole });
    emit dataChanged_(m_data);
}

void CoeffModel::setPrecision(int prec)
{
    m_precision = prec;
    emit dataChanged(createIndex(0, 0), createIndex(static_cast<int>(m_data.size()) - 1, 1), { Qt::DisplayRole });
}

void CoeffModel::load(const QString& fileName)
{
    qDebug(__FUNCTION__);
    if (QFile file(fileName); file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        auto lastSize { static_cast<int>(m_data.size()) };
        m_data.clear();
        if (0) {
            Timer { 0 };
            QString line;
            QTextStream stream(&file);
            while (stream.readLineInto(&line)) {
                m_data.push_back(line.toDouble());
            }
        } else {
            Timer { 1 };
            QByteArray fileData(file.readAll());
            static constexpr auto pattern = ctll::fixed_string { "\\s*(\\S+)\\s*\n" };
            auto range = ctre::range<pattern>(std::string_view(fileData.data(), fileData.size()));
            for (auto [wholeStr, xStr] : range) {
                if (wholeStr) {
                    m_data.push_back(xStr.size() ? QByteArray { xStr.data(), static_cast<int>(xStr.size()) }.toDouble() : 0.0);
                }
            }
        }

        if (lastSize > static_cast<int>(m_data.size()))
            emit rowsRemoved({}, static_cast<int>(m_data.size()), lastSize - 1, {});
        else if (lastSize < static_cast<int>(m_data.size()))
            emit rowsInserted({}, lastSize, static_cast<int>(m_data.size()) - 1, {});
        else
            emit dataChanged(createIndex(0, 0), createIndex(static_cast<int>(m_data.size()) - 1, 0), { Qt::DisplayRole });
        emit dataChanged_(m_data);
    }
}

void CoeffModel::save(const QString& fileName) const
{
    qDebug(__FUNCTION__);
    if (QFile file(fileName); file.open(QIODevice::WriteOnly)) {
        if (fileName.endsWith("bin")) {
            QDataStream out(&file);
            out << uint32_t(static_cast<int>(m_data.size()));
            for (auto&& val : m_data)
                out << val;
        } else {
            QTextStream out(&file);
            for (size_t i = 0; i < m_data.size(); ++i)
                out << QString("%1\n").arg(m_data[i], 0, 'G', m_precision);
        }
    }
}
