#pragma once

#include <QAbstractListModel>

class CoeffModel : public QAbstractListModel {
    Q_OBJECT

    std::vector<double> m_data;

signals:

public:
    explicit CoeffModel(QObject* parent = nullptr);

    // QAbstractItemModel interface
    int rowCount(const QModelIndex& = {}) const override;
    int columnCount(const QModelIndex& = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    void setData(const std::vector<double> &data);
};
