#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTreeView>
#include <usbhiddevice.h>
#include "treemodel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void updateActions();
    void insertChild();
    bool insertColumn();
    void insertRow();
    bool removeColumn();
    void removeRow();

private:
    Ui::MainWindow *ui;
    QLabel *lStatusTitle;
    QLabel *lStatusValue;
    QLabel *lMsgCntValue;
    USBHIDDevice* usbHid;

    QTreeView *view;
    TreeModel *model;

    QAction *insertRowAction;
    QAction *removeRowAction;
    QAction *insertColumnAction;
    QAction *removeColumnAction;
    QAction *insertChildAction;
};
#endif // MAINWINDOW_H
