#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QProcess>
#include "diskvisualizer.h"
#include "isowriterdialog.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void refreshDiskList();
    void scanDisk();
    void showContextMenu(const QPoint &pos);


    void deletePartition();
    void formatPartition();
    void resizePartition();
    void toggleMount();
    void backupPartition();
    void openCloneWizard();
    void openIsoWizard();
    void restoreUsb();
    void renamePartition();


private:
    QTableWidget *tableWidget;
    QComboBox *diskCombo;
    QPushButton *refreshButton;
    QPushButton *scanButton;
    DiskVisualizer *visualizer;
};

#endif // MAINWINDOW_H
