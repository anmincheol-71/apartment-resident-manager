#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "residentmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class DashboardPage;
class ResidentPage;
class ParkingPage;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void setActivePage(int index);
    void updateStatusBar();

    Ui::MainWindow  *ui;
    ResidentManager *m_mgr;

    DashboardPage *m_dashPage;
    ResidentPage  *m_residentPage;
    ParkingPage   *m_parkingPage;

    QPushButton *m_navBtns[3];
};

#endif // MAINWINDOW_H
