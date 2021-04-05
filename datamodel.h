#ifndef DATAMODEL_H
#define DATAMODEL_H

#include "types.h"
#include <QAbstractTableModel>

class DataModel final : public QAbstractTableModel, public Clipboard {
    Q_OBJECT

    Data m_data;
    Data m_dataComp;
    int m_precision{3};

signals:
    void dataChanged_(const Data&);
    void dataCompChanged(const Degrees&);

public:
    explicit DataModel(QObject* parent = nullptr);
    ~DataModel();

    // QAbstractItemModel interface
    int rowCount(const QModelIndex& = {}) const override;
    int columnCount(const QModelIndex& = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& = {}) const override;

    enum {
        ColumnX,
        ColumnY,
        ColumnComp,
        ColumnDelta,
        ColumnCount
    };

    const Data& data() const noexcept;
    void setNewData(const Data& data);
    void setPrecision(int prec);

    void load(const QString& fileName);
    void save(const QString& fileName) const;

    // Clipboard interface
    QString copy() const override;
    void paste(QString&&) override;
};

#endif // DATAMODEL_H
