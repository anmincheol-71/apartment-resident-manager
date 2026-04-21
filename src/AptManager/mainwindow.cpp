#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "residentpage.h"

#include <QMessageBox>
#include <QSqlQuery>

// 생성자
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

    // 페이지 생성
    m_residentPage = new ResidentPage(m_mgr, this);
    ui->stackedWidget->addWidget(m_residentPage);
    ui->stackedWidget->setCurrentIndex(0);

    // 데이터 변경 시 상태바 갱신
    connect(m_mgr, &ResidentManager::dataChanged, this, &MainWindow::updateStatusBar);

    updateStatusBar();
}

// 소멸자 ui 삭제
MainWindow::~MainWindow()
{
    delete ui;
}

// 상태바 업데이트 -> 총 세대수 카운트 및 출력
void MainWindow::updateStatusBar()
{
    QSqlQuery q("SELECT COUNT(*) FROM resident");
    int residents = (q.next()) ? q.value(0).toInt() : 0;

    ui->statusbar->showMessage(
        QString("총 %1세대 등록").arg(residents)
    );
}
