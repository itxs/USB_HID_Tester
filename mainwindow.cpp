#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include "treeitem.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    insertRowAction = new QAction(this);
    insertRowAction->setObjectName(QString::fromUtf8("insertRowAction"));
    removeRowAction = new QAction(this);
    removeRowAction->setObjectName(QString::fromUtf8("removeRowAction"));
    insertColumnAction = new QAction(this);
    insertColumnAction->setObjectName(QString::fromUtf8("insertColumnAction"));
    removeColumnAction = new QAction(this);
    removeColumnAction->setObjectName(QString::fromUtf8("removeColumnAction"));
    insertChildAction = new QAction(this);
    insertChildAction->setObjectName(QString::fromUtf8("insertChildAction"));

    insertRowAction->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+I, R", nullptr));
    removeRowAction->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+R, R", nullptr));
    insertColumnAction->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+I, C", nullptr));
    removeColumnAction->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+R, C", nullptr));
    insertChildAction->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+N", nullptr));

    const QStringList headers({tr("Title"), tr("Description")});
    QFile file(":/default.txt");
    file.open(QIODevice::ReadOnly);
    model = new TreeModel(headers, file.readAll());
    file.close();

    ui->viewTx->setModel(model);
    for (int column = 0; column < model->columnCount(); ++column)
        ui->viewTx->resizeColumnToContents(column);

    connect(ui->viewTx->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::updateActions);
    connect(insertRowAction, &QAction::triggered, this, &MainWindow::insertRow);
    connect(insertColumnAction, &QAction::triggered, this, &MainWindow::insertColumn);
    connect(removeRowAction, &QAction::triggered, this, &MainWindow::removeRow);
    connect(removeColumnAction, &QAction::triggered, this, &MainWindow::removeColumn);
    connect(insertChildAction, &QAction::triggered, this, &MainWindow::insertChild);

    lStatusTitle = new QLabel(this);
    lStatusTitle->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    lStatusTitle->setText("Last command status:");

    lStatusValue = new QLabel(this);
    lStatusValue->setPixmap(QPixmap(":/icon_ok.png", "PNG"));

    lMsgCntValue = new QLabel(this);
    lMsgCntValue->setText("Message count:     0");

    ui->statusbar->addPermanentWidget(lStatusTitle);
    ui->statusbar->addPermanentWidget(lStatusValue);
    ui->statusbar->addWidget(lMsgCntValue);

    usbHid = new USBHIDDevice(this, 0000, 0000);
}

void MainWindow::updateActions()
{
    const bool hasSelection = !ui->viewTx->selectionModel()->selection().isEmpty();
    removeRowAction->setEnabled(hasSelection);
    removeColumnAction->setEnabled(hasSelection);

    const bool hasCurrent = ui->viewTx->selectionModel()->currentIndex().isValid();
    insertRowAction->setEnabled(hasCurrent);
    insertColumnAction->setEnabled(hasCurrent);

    if (hasCurrent) {
        ui->viewTx->closePersistentEditor(ui->viewTx->selectionModel()->currentIndex());

        const int row = ui->viewTx->selectionModel()->currentIndex().row();
        const int column = ui->viewTx->selectionModel()->currentIndex().column();
        if (ui->viewTx->selectionModel()->currentIndex().parent().isValid())
            statusBar()->showMessage(tr("Position: (%1,%2)").arg(row).arg(column));
        else
            statusBar()->showMessage(tr("Position: (%1,%2) in top level").arg(row).arg(column));
    }
}

void MainWindow::insertChild()
{
    const QModelIndex index = ui->viewTx->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->viewTx->model();

    if (model->columnCount(index) == 0) {
        if (!model->insertColumn(0, index))
            return;
    }

    if (!model->insertRow(0, index))
        return;

    for (int column = 0; column < model->columnCount(index); ++column) {
        const QModelIndex child = model->index(0, column, index);
        model->setData(child, QVariant(tr("[No data]")), Qt::EditRole);
        if (!model->headerData(column, Qt::Horizontal).isValid())
            model->setHeaderData(column, Qt::Horizontal, QVariant(tr("[No header]")), Qt::EditRole);
    }

    ui->viewTx->selectionModel()->setCurrentIndex(model->index(0, 0, index),
                                            QItemSelectionModel::ClearAndSelect);
    updateActions();
}

bool MainWindow::insertColumn()
{
    QAbstractItemModel *model = ui->viewTx->model();
    int column = ui->viewTx->selectionModel()->currentIndex().column();

    // Insert a column in the parent item.
    bool changed = model->insertColumn(column + 1);
    if (changed)
        model->setHeaderData(column + 1, Qt::Horizontal, QVariant("[No header]"), Qt::EditRole);

    updateActions();

    return changed;
}

void MainWindow::insertRow()
{
    const QModelIndex index = ui->viewTx->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->viewTx->model();

    if (!model->insertRow(index.row()+1, index.parent()))
        return;

    updateActions();

    for (int column = 0; column < model->columnCount(index.parent()); ++column) {
        const QModelIndex child = model->index(index.row() + 1, column, index.parent());
        model->setData(child, QVariant(tr("[No data]")), Qt::EditRole);
    }
}

bool MainWindow::removeColumn()
{
    QAbstractItemModel *model = ui->viewTx->model();
    const int column = ui->viewTx->selectionModel()->currentIndex().column();

    // Insert columns in each child of the parent item.
    const bool changed = model->removeColumn(column);
    if (changed)
        updateActions();

    return changed;
}

void MainWindow::removeRow()
{
    const QModelIndex index = ui->viewTx->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->viewTx->model();
    if (model->removeRow(index.row(), index.parent()))
        updateActions();
}

MainWindow::~MainWindow()
{
    delete ui;
}

