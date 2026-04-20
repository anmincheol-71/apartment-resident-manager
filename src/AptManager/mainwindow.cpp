#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dashboardpage.h"
#include "residentpage.h"
#include "parkingpage.h"

#include <QMessageBox>
#include <QStackedWidget>
#include <QPushButton>
#include <QSqlQuery>

// 네비게이션 버튼 스타일
static const QString kNavActive =
    "QPushButton { background:transparent; color:#FFFFFF; border:none;"
    "  border-bottom:3px solid #2563EB; padding:16px 22px;"
    "  font-size:14px; font-weight:bold; }";
static const QString kNavInactive =
    "QPushButton { background:transparent; color:#64748B; border:none;"
    "  border-bottom:3px solid transparent; padding:16px 22px; font-size:14px; }"
    "QPushButton:hover { color:#CBD5E1; }";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_mgr(new ResidentManager(this))
{
    ui->setupUi(this);
    setMenuBar(nullptr);
    setMinimumSize(900, 620);

    if (!m_mgr->initialize()) {
        QMessageBox::critical(this, "오류", "데이터베이스 초기화에 실패했습니다.");
        return;
    }

    // ── 페이지 생성 ──
    m_dashPage     = new DashboardPage(m_mgr, this);
    m_residentPage = new ResidentPage(m_mgr, this);
    m_parkingPage  = new ParkingPage(m_mgr, this);

    ui->stackedWidget->addWidget(m_dashPage);
    ui->stackedWidget->addWidget(m_residentPage);
    ui->stackedWidget->addWidget(m_parkingPage);

    // ── 네비 버튼 참조 ──
    m_navBtns[0] = ui->navHomeBtn;
    m_navBtns[1] = ui->navResidentBtn;
    m_navBtns[2] = ui->navParkingBtn;

    connect(ui->navHomeBtn,     &QPushButton::clicked, this, [this]{ setActivePage(0); });
    connect(ui->navResidentBtn, &QPushButton::clicked, this, [this]{ setActivePage(1); });
    connect(ui->navParkingBtn,  &QPushButton::clicked, this, [this]{ setActivePage(2); });

    // 데이터 변경 시 상태바 갱신
    connect(m_mgr, &ResidentManager::dataChanged, this, &MainWindow::updateStatusBar);

    setActivePage(0);
    updateStatusBar();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setActivePage(int index)
{
    ui->stackedWidget->setCurrentIndex(index);

    for (int i = 0; i < 3; ++i)
        m_navBtns[i]->setStyleSheet(i == index ? kNavActive : kNavInactive);

    // 페이지 전환 시 데이터 새로고침
    if (index == 0) m_dashPage->refresh();
    if (index == 2) m_parkingPage->refresh();
}

void MainWindow::updateStatusBar()
{
    QList<Car> cars = m_mgr->getAllCars();
    int parked = 0;
    for (const Car &c : cars)
        if (c.isParked) ++parked;

    QSqlQuery q("SELECT COUNT(*) FROM resident");
    int residents = (q.next()) ? q.value(0).toInt() : 0;

    ui->statusbar->showMessage(
        QString("총 %1세대   |   등록 차량 %2대   |   주차 중 %3대")
            .arg(residents).arg(cars.size()).arg(parked)
    );
}
