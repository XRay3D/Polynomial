#pragma once

#include <QAbstractListModel>

class CoeffModel : public QAbstractTableModel {
    Q_OBJECT
    struct double_data {
        double data[2] {};
        double& operator[](size_t i) noexcept { return data[i]; }
        double operator[](size_t i) const noexcept { return data[i]; }
    };

    std::vector<double_data> m_data;
    int m_prec { 3 };
signals:

public:
    explicit CoeffModel(QObject* parent = nullptr);

    // QAbstractItemModel interface
    int rowCount(const QModelIndex& = {}) const override;
    int columnCount(const QModelIndex& = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    void setData(const std::vector<double>& data, int column = {});
    void setPrec(int prec) ;
};
