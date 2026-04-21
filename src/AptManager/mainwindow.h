#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "residentmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ResidentPage;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void updateStatusBar();

    Ui::MainWindow  *ui;
    ResidentManager *m_mgr;
    ResidentPage    *m_residentPage;
};

#endif // MAINWINDOW_H
