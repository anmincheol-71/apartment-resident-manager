#include "02_alertpopup.h"
#include "02_securityalertmanager.h"

#include <QAudioOutput>
#include <QDialog>
#include <QFile>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMediaPlayer>
#include <QMessageBox>
#include <QPushButton>
#include <QScreen>
#include <QUrl>
#include <QVBoxLayout>
#include <QVideoWidget>

// ── 생성자 ─────────────────────────────────────────────────────────────────
AlertPopup::AlertPopup(int alertId,
                       const QString &location,
                       const QString &type,
                       const QDateTime &occurredAt,
                       const QString &videoPath,
                       SecurityAlertManager *mgr,
                       QWidget *parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , m_alertId(alertId)
    , m_location(location)
    , m_type(type)
    , m_videoPath(videoPath)
    , m_mgr(mgr)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedWidth(400);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── 헤더 ─────────────────────────────────────────────
    auto *header = new QWidget;
    header->setFixedHeight(52);
    header->setStyleSheet("background-color: #B71C1C;");
    auto *hLayout = new QHBoxLayout(header);
    hLayout->setContentsMargins(16, 0, 16, 0);

    auto *icon = new QLabel("🚨");
    icon->setStyleSheet("font-size: 22px;");
    auto *title = new QLabel("  보안 이상징후 감지");
    title->setStyleSheet("color: white; font-size: 15px; font-weight: bold;");

    hLayout->addWidget(icon);
    hLayout->addWidget(title);
    hLayout->addStretch();
    root->addWidget(header);

    // ── 정보 영역 ─────────────────────────────────────────
    auto *body = new QWidget;
    body->setStyleSheet("background-color: #212121;");
    auto *bLayout = new QVBoxLayout(body);
    bLayout->setContentsMargins(18, 14, 18, 14);
    bLayout->setSpacing(7);

    auto makeLbl = [](const QString &text) {
        auto *l = new QLabel(text);
        l->setStyleSheet("color: #EEEEEE; font-size: 13px;");
        return l;
    };

    bLayout->addWidget(makeLbl("📍 위치 : " + location));
    bLayout->addWidget(makeLbl("⚠️  유형 : " + type));
    bLayout->addWidget(makeLbl("🕐 시각 : " + occurredAt.toString("yyyy-MM-dd  hh:mm:ss")));
    root->addWidget(body);

    // ── 버튼 영역 ─────────────────────────────────────────
    auto *btnArea = new QWidget;
    btnArea->setStyleSheet("background-color: #1A1A1A;");
    auto *btnLayout = new QVBoxLayout(btnArea);
    btnLayout->setContentsMargins(12, 10, 12, 10);
    btnLayout->setSpacing(6);

    // 1행: CCTV 영상 확인 + 확인 완료
    auto *row1 = new QHBoxLayout;

    auto *videoBtn = new QPushButton("CCTV 영상 확인");
    videoBtn->setCursor(Qt::PointingHandCursor);
    videoBtn->setStyleSheet(
        "QPushButton { background-color:#E53935; color:white; border-radius:4px;"
        "              padding:7px 14px; font-weight:bold; font-size:13px; }"
        "QPushButton:hover { background-color:#C62828; }"
    );

    auto *confirmBtn = new QPushButton("확인 완료");
    confirmBtn->setCursor(Qt::PointingHandCursor);
    confirmBtn->setStyleSheet(
        "QPushButton { background-color:#555; color:white; border-radius:4px;"
        "              padding:7px 14px; font-size:13px; }"
        "QPushButton:hover { background-color:#333; }"
    );

    row1->addWidget(videoBtn);
    row1->addWidget(confirmBtn);
    btnLayout->addLayout(row1);

    // 2행: 주의 공지 등록 (전체 너비)
    auto *noticeBtn = new QPushButton("📋  주의 공지 등록");
    noticeBtn->setCursor(Qt::PointingHandCursor);
    noticeBtn->setStyleSheet(
        "QPushButton { background-color:#E65100; color:white; border-radius:4px;"
        "              padding:7px 14px; font-size:13px; }"
        "QPushButton:hover { background-color:#BF360C; }"
    );
    btnLayout->addWidget(noticeBtn);

    root->addWidget(btnArea);

    connect(videoBtn,   &QPushButton::clicked, this, &AlertPopup::onPlayVideo);
    connect(confirmBtn, &QPushButton::clicked, this, &AlertPopup::onConfirm);
    connect(noticeBtn,  &QPushButton::clicked, this, &AlertPopup::onRegisterNotice);

    adjustSize();

    // 화면 우측 하단에 배치 (작업표시줄 위 20px 여백)
    QRect screen = QGuiApplication::primaryScreen()->availableGeometry();
    move(screen.right() - width() - 24, screen.bottom() - height() - 60);
}

// ── 슬롯 구현 ──────────────────────────────────────────────────────────────

// DB에 확인 완료 기록 후 팝업 닫기
void AlertPopup::onConfirm()
{
    m_mgr->confirmAlert(m_alertId);
    emit confirmed(m_alertId);
    close();
}

// QMediaPlayer 팝업으로 영상 재생
void AlertPopup::onPlayVideo()
{
    if (m_videoPath.isEmpty() || !QFile::exists(m_videoPath)) {
        QMessageBox::warning(this, "영상 없음",
            "영상 파일을 찾을 수 없습니다.\n경로: " + m_videoPath);
        return;
    }

    auto *dlg = new QDialog(nullptr, Qt::Window);
    dlg->setWindowTitle("CCTV 영상 재생 - " + m_videoPath.section('/', -1));
    dlg->setMinimumSize(854, 520);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    auto *layout = new QVBoxLayout(dlg);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *videoWidget = new QVideoWidget;
    videoWidget->setStyleSheet("background:black;");
    layout->addWidget(videoWidget);

    auto *player = new QMediaPlayer(dlg);
    auto *audio = new QAudioOutput(dlg);
    player->setAudioOutput(audio);
    player->setVideoOutput(videoWidget);
    player->setSource(QUrl::fromLocalFile(m_videoPath));
    player->play();

    dlg->show();
}

// 알림 유형에 따라 공지 유형/제목 결정 후 MainWindow로 신호 전달
void AlertPopup::onRegisterNotice()
{
    // 배회/의심/이상행동 -> 주의인물, 나머지 -> 보안경고
    QString noticeType = "보안경고";
    if (m_type.contains("배회") || m_type.contains("의심") || m_type.contains("이상행동"))
        noticeType = "주의인물";

    QString noticeTitle = QString("[%1] %2 %3 감지").arg(noticeType, m_location, m_type);

    emit noticeRequested(noticeType, noticeTitle, m_videoPath);
}
