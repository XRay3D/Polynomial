#include "tableview.h"
#include "types.h"

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QHeaderView>
#include <QMenu>
#include <QStyledItemDelegate>

class ItemDelegate : public QStyledItemDelegate {
public:
    ItemDelegate(QWidget* parent)
        : QStyledItemDelegate { parent } { }
    ~ItemDelegate() override { }

    // QAbstractItemDelegate interface
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const override {
        auto doubleSpinBox { new QDoubleSpinBox(parent) };
        doubleSpinBox->setDecimals(16);
        doubleSpinBox->setRange(-std::numeric_limits<double>::max(), +std::numeric_limits<double>::max());
        return doubleSpinBox;
    }
    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        static_cast<QDoubleSpinBox*>(editor)->setValue(index.data(Qt::EditRole).toDouble());
    }
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
        model->setData(index, static_cast<QDoubleSpinBox*>(editor)->value(), Qt::EditRole);
    }
};

TableView::TableView(QWidget* parent)
    : QTableView(parent) //
{
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    verticalHeader()->setDefaultSectionSize(QFontMetrics(font()).height());
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &TableView::customMenuRequested);
    setItemDelegate(new ItemDelegate(this));
}

void TableView::customMenuRequested(const QPoint& pos) {
    QMenu menu;
    menu.addAction(QIcon::fromTheme("edit-copy"), "Копировать", [this] {
        QApplication::clipboard()->setText(model<Clipboard>()->copy());
    });
    menu.addAction(QIcon::fromTheme("edit-paste"), "Вставить", [this] {
        model<Clipboard>()->paste(QApplication::clipboard()->text());
    });
    menu.exec(viewport()->mapToGlobal(pos));
}
