#include "00_mainwindow.h"
#include "ui_00_mainwindow.h"
#include "02_alerthistorypage.h"
#include "02_alertpopup.h"
#include "00_cctvutils.h"
#include "03_noticedialog.h"
#include "03_noticepage.h"
#include "01_residentpage.h"

#include <QMessageBox>
#include <QSqlQuery>
#include <QTimer>

// ── 생성자 ─────────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_mgr(new ResidentManager(this))
    , m_alertMgr(new SecurityAlertManager(this))
    , m_noticeMgr(new NoticeManager(this))
{
    ui->setupUi(this);
    setMenuBar(nullptr);
    setMinimumSize(1000, 660);

    // DB 초기화 - 실패 시 즉시 종료
    if (!m_mgr->initialize()) {
        QMessageBox::critical(this, "오류", "데이터베이스 초기화에 실패했습니다.");
        return;
    }
    if (!m_alertMgr->initialize()) {
        QMessageBox::critical(this, "오류", "보안 알림 DB 초기화에 실패했습니다.");
        return;
    }
    if (!m_noticeMgr->initialize()) {
        QMessageBox::critical(this, "오류", "공지사항 DB 초기화에 실패했습니다.");
        return;
    }

    // ── 페이지 구성 (stackedWidget 인덱스 순서) ─────────────────────────
    m_residentPage = new ResidentPage(m_mgr, this);
    ui->stackedWidget->addWidget(m_residentPage);   // index 0

    m_alertPage = new AlertHistoryPage(m_alertMgr, this);
    ui->stackedWidget->addWidget(m_alertPage);       // index 1

    m_noticePage = new NoticePage(m_noticeMgr, this);
    ui->stackedWidget->addWidget(m_noticePage);      // index 2

    ui->stackedWidget->setCurrentIndex(0);

    // ── 네비게이션 버튼 ───────────────────────────────────────────────────
    connect(ui->navResidentBtn, &QPushButton::clicked, this, [this]() {
        ui->stackedWidget->setCurrentIndex(0);
    });
    connect(ui->navAlertBtn, &QPushButton::clicked, this, [this]() {
        ui->stackedWidget->setCurrentIndex(1);
        m_alertPage->refresh();
    });
    connect(ui->navNoticeBtn, &QPushButton::clicked, this, [this]() {
        ui->stackedWidget->setCurrentIndex(2);
        m_noticePage->refresh();
    });

    // ── 시그널-슬롯 연결 ─────────────────────────────────────────────────

    // 새 보안 알림 발생 -> 팝업 표시 + 내역 페이지 갱신 + 상태바 갱신
    connect(m_alertMgr, &SecurityAlertManager::newAlert,
            this, &MainWindow::showAlertPopup);
    connect(m_alertMgr, &SecurityAlertManager::newAlert,
            m_alertPage, &AlertHistoryPage::refresh);
    connect(m_alertMgr, &SecurityAlertManager::newAlert,
            this, &MainWindow::updateStatusBar);

    // 새 공지 등록 -> 공지 페이지 갱신
    connect(m_noticeMgr, &NoticeManager::noticeAdded,
            m_noticePage, &NoticePage::refresh);

    // 입주민 데이터 변경 -> 상태바 갱신
    connect(m_mgr, &ResidentManager::dataChanged,
            this, &MainWindow::updateStatusBar);

    updateStatusBar();

    // ── 시나리오 시뮬레이션 ───────────────────────────────────────────────

    // 5초 후: 지하주차장 침입 감지 (CCTV_alarm.mp4)
    QTimer::singleShot(5000, this, [this]() {
        QString video = findVideo("CCTV_alarm.mp4");
        m_alertMgr->addAlert("101동 지하주차장", "침입", video);
    });

    // 12초 후: CCTV를 의도적으로 응시하며 배회하는 의심인물 감지 (CCTV_stranger.mp4)
    //          -> 보안 알림 + 전체 세대 주의인물 공지 자동 등록
    QTimer::singleShot(12000, this, [this]() {
        QString video = findVideo("CCTV_stranger.mp4");
        m_alertMgr->addAlert("단지 내 CCTV 구역", "의심인물 배회", video);

        // 자동 주의인물 공지 등록 - 전체 세대 수신
        m_noticeMgr->addNotice(
            "주의인물",
            "[주의인물] CCTV 포착 배회 의심인물 - 주민 여러분 주의 요망",
            "단지 내 CCTV를 지속적으로 응시하며 배회하는 의심인물이 포착되었습니다.\n\n"
            "해당 인물이 접근하거나 위협적 행동을 보일 경우 즉시 관리사무소(내선 0)로 "
            "신고해 주시기 바랍니다.\n\n"
            "첨부 CCTV 영상을 참고하시기 바랍니다.",
            video,
            "전체", 0, 0
        );
    });
}

// 소멸자
MainWindow::~MainWindow()
{
    delete ui;
}

// ── 슬롯 구현 ──────────────────────────────────────────────────────────────

// 보안 알림 팝업 생성 후 표시
void MainWindow::showAlertPopup(int alertId)
{
    SecurityAlert a = m_alertMgr->getAlertById(alertId);
    if (a.id == 0) return;

    auto *popup = new AlertPopup(
        a.id, a.location, a.type, a.occurredAt, a.videoPath,
        m_alertMgr, nullptr
    );

    // 확인 완료 -> 내역 갱신 + 상태바 갱신
    connect(popup, &AlertPopup::confirmed, m_alertPage,   &AlertHistoryPage::refresh);
    connect(popup, &AlertPopup::confirmed, this,          &MainWindow::updateStatusBar);

    // 주의 공지 등록 버튼 -> NoticeDialog 오픈
    connect(popup, &AlertPopup::noticeRequested, this, &MainWindow::onRegisterNotice);

    popup->show();
}

// 보안 알림 팝업의 '주의 공지 등록' -> 미리채움된 NoticeDialog 오픈
void MainWindow::onRegisterNotice(const QString &noticeType,
                                   const QString &title,
                                   const QString &videoPath)
{
    Notice prefill;
    prefill.type = noticeType;
    prefill.title = title;
    prefill.videoPath = videoPath;
    prefill.targetType = "전체";

    NoticeDialog dlg(prefill, m_noticeMgr, this);
    if (dlg.exec() == QDialog::Accepted)
        m_noticePage->refresh();
}

// 상태바: 총 세대수 + 미확인 보안 알림 건수 표시
void MainWindow::updateStatusBar()
{
    QSqlQuery rq("SELECT COUNT(*) FROM resident");
    int residents = (rq.next()) ? rq.value(0).toInt() : 0;

    QSqlQuery aq("SELECT COUNT(*) FROM security_alert WHERE isConfirmed = 0");
    int unconfirmed = (aq.next()) ? aq.value(0).toInt() : 0;

    QString msg = QString("총 %1세대 등록").arg(residents);
    if (unconfirmed > 0)
        msg += QString("   |   🔴 미확인 보안 알림 %1건").arg(unconfirmed);

    ui->statusbar->showMessage(msg);
}
