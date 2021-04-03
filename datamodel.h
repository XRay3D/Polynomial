#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QAbstractTableModel>
#include <QPolygonF>

class DataModel : public QAbstractTableModel {
    Q_OBJECT

    QPolygonF m_data;
signals:

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
        X,
        Y,
        ColumnCount
    };

    const QPolygonF& data() const noexcept { return m_data; };
};

#endif // DATAMODEL_H
