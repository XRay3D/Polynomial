#pragma once

#include "types.h"
#include <QAbstractListModel>

class CoeffModel : public QAbstractTableModel {
    Q_OBJECT
    Degrees m_data;
    int m_precision { 3 };

signals:
    void dataChanged_(const Degrees&);

public:
    explicit CoeffModel(QObject* parent = nullptr);

    // QAbstractItemModel interface
    int rowCount(const QModelIndex& = {}) const override;
    int columnCount(const QModelIndex& = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void setCoeffData(const Degrees& data);
    void setPrecision(int prec);

    void load(const QString& fileName);
    void save(const QString& fileName) const;
};
