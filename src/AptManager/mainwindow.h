#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "residentmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void onSearch();
    void onClear();
    void onDataChanged();
    void onRowDoubleClicked(const QModelIndex &index);

private:
    Ui::MainWindow *ui;
    ResidentManager *m_mgr;

    int  selectedResidentId() const;
    void showResidentDialog(bool isEdit);
    void updateStatusBar();
};

#endif // MAINWINDOW_H
