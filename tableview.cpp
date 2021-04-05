#include "tableview.h"
#include "types.h"

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QHeaderView>
#include <QMenu>

TableView::TableView(QWidget* parent)
    : QTableView(parent) //
{
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    verticalHeader()->setDefaultSectionSize(QFontMetrics(font()).height());
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &TableView::customMenuRequested);
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
