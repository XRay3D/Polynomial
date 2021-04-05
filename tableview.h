#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QTableView>

class TableView : public QTableView {
    Q_OBJECT

signals:

public:
    explicit TableView(QWidget* parent = nullptr);
    template <class T>
    T* model() { return dynamic_cast<T*>(QTableView::model()); }

private:
    void customMenuRequested(const QPoint& pos);
};

#endif // TABLEVIEW_H
