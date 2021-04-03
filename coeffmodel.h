#pragma once

#include <QAbstractListModel>

class CoeffModel : public QAbstractTableModel {
    Q_OBJECT
    std::vector<double> m_data;
    int m_prec { 3 };

public:
    explicit CoeffModel(QObject* parent = nullptr);

    // QAbstractItemModel interface
    int rowCount(const QModelIndex& = {}) const override;
    int columnCount(const QModelIndex& = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void setData(const std::vector<double>& data);
    void setPrec(int prec);
    void save(const QString& fileName);
};
