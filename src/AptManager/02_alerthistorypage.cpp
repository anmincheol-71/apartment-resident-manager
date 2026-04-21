#include "02_alerthistorypage.h"
#include "00_cctvutils.h"
#include "02_securityalert.h"
#include "02_securityalertmanager.h"

#include <QAudioOutput>
#include <QDialog>
#include <QFile>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMediaPlayer>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QUrl>
#include <QVBoxLayout>
#include <QVideoWidget>

// ── 생성자 ─────────────────────────────────────────────────────────────────
AlertHistoryPage::AlertHistoryPage(SecurityAlertManager *mgr, QWidget *parent)
    : QWidget(parent)
    , m_mgr(mgr)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(12);

    // ── 헤더 ─────────────────────────────────────────────
    auto *header = new QHBoxLayout;

    auto *title = new QLabel("보안 알림 내역");
    title->setStyleSheet("font-size: 20px; font-weight: bold;");
    header->addWidget(title);

    m_countLabel = new QLabel;
    m_countLabel->setStyleSheet("color: #888; font-size: 13px; margin-left: 10px;");
    header->addWidget(m_countLabel);
    header->addStretch();

    // CCTV_alarm / CCTV_stranger 영상을 번갈아 사용하는 테스트 버튼
    auto *simBtn = new QPushButton("🚨  테스트 알림 발생");
    simBtn->setCursor(Qt::PointingHandCursor);
    simBtn->setStyleSheet(
        "QPushButton { background-color:#E53935; color:white; border-radius:5px;"
        "              padding:8px 18px; font-weight:bold; font-size:13px; }"
        "QPushButton:hover { background-color:#C62828; }"
    );
    header->addWidget(simBtn);

    auto *refreshBtn = new QPushButton("새로고침");
    refreshBtn->setCursor(Qt::PointingHandCursor);
    refreshBtn->setStyleSheet(
        "QPushButton { background-color:#1976D2; color:white; border-radius:5px;"
        "              padding:8px 16px; font-size:13px; }"
        "QPushButton:hover { background-color:#1565C0; }"
    );
    header->addWidget(refreshBtn);

    root->addLayout(header);

    auto *hint = new QLabel("행을 더블클릭하면 CCTV 영상을 재생합니다.");
    hint->setStyleSheet("color: #999; font-size: 12px;");
    root->addWidget(hint);

    // ── 테이블 ─────────────────────────────────────────────
    // 컬럼: ID | 발생시각 | 위치 | 유형 | 영상 | 확인 여부
    m_table = new QTableWidget;
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({"ID", "발생시각", "위치", "유형", "영상", "확인 여부"});
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->hide();
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    m_table->setStyleSheet(
        "QTableWidget { font-size:13px; }"
        "QHeaderView::section { background-color:#37474F; color:white; padding:6px;"
        "                       font-weight:bold; border:none; }"
    );
    root->addWidget(m_table);

    connect(simBtn,     &QPushButton::clicked,            this, &AlertHistoryPage::onSimulateAlert);
    connect(refreshBtn, &QPushButton::clicked,            this, &AlertHistoryPage::refresh);
    connect(m_table,    &QTableWidget::cellDoubleClicked, this, &AlertHistoryPage::onRowDoubleClicked);

    refresh();
}

// ── 슬롯 구현 ──────────────────────────────────────────────────────────────

// 알림 목록 새로고침 - DB 재조회 후 테이블 갱신
void AlertHistoryPage::refresh()
{
    QList<SecurityAlert> alerts = m_mgr->getAlerts();
    m_table->setRowCount(alerts.size());

    int unconfirmed = 0;
    for (int i = 0; i < alerts.size(); ++i) {
        const SecurityAlert &a = alerts[i];
        if (!a.isConfirmed) ++unconfirmed;

        auto centered = [](const QString &text) {
            auto *item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            return item;
        };

        m_table->setItem(i, 0, centered(QString::number(a.id)));
        m_table->setItem(i, 1, new QTableWidgetItem(
            a.occurredAt.toString("yyyy-MM-dd  hh:mm:ss")));
        m_table->setItem(i, 2, new QTableWidgetItem(a.location));
        m_table->setItem(i, 3, new QTableWidgetItem(a.type));

        // 영상 열 - UserRole에 파일 경로 저장 (더블클릭 시 재생용)
        auto *videoItem = centered(a.videoPath.isEmpty() ? "—" : "▶ 재생");
        videoItem->setData(Qt::UserRole, a.videoPath);
        if (!a.videoPath.isEmpty())
            videoItem->setForeground(QColor("#1565C0"));
        m_table->setItem(i, 4, videoItem);

        // 미확인은 빨간색으로 강조
        auto *confItem = centered(a.isConfirmed ? "✅ 확인됨" : "🔴 미확인");
        if (!a.isConfirmed)
            confItem->setForeground(QColor("#D32F2F"));
        m_table->setItem(i, 5, confItem);
    }

    m_countLabel->setText(
        QString("전체 %1건  |  미확인 %2건").arg(alerts.size()).arg(unconfirmed)
    );
}

// 테스트 알림 발생 - 위치/유형/영상을 순환하며 알림 추가
void AlertHistoryPage::onSimulateAlert()
{
    static const char *locations[] = {
        "101동 지하주차장", "102동 정문", "103동 후면 출입구",
        "단지 내 공원",     "104동 엘리베이터홀"
    };
    static const char *types[] = {"침입", "배회", "이상행동", "화재감지"};
    // CCTV_alarm, CCTV_stranger 두 영상을 번갈아 사용
    static const char *videos[] = {"CCTV_alarm.mp4", "CCTV_stranger.mp4"};
    static int counter = 0;

    QString loc   = QString::fromUtf8(locations[counter % 5]);
    QString type  = QString::fromUtf8(types[counter % 4]);
    QString video = findVideo(QString::fromUtf8(videos[counter % 2]));
    ++counter;

    m_mgr->addAlert(loc, type, video);
}

// 행 더블클릭 - 영상 열에서 경로 꺼내 재생
void AlertHistoryPage::onRowDoubleClicked(int row, int col)
{
    Q_UNUSED(col)

    auto *item = m_table->item(row, 4); // 영상 열
    if (!item) return;

    QString path = item->data(Qt::UserRole).toString();
    if (path.isEmpty()) {
        QMessageBox::information(this, "영상 없음", "이 알림에 연결된 영상이 없습니다.");
        return;
    }
    playVideo(path);
}

// QMediaPlayer 팝업으로 영상 재생
void AlertHistoryPage::playVideo(const QString &path)
{
    if (!QFile::exists(path)) {
        QMessageBox::warning(this, "파일 없음",
            "영상 파일을 찾을 수 없습니다.\n경로: " + path);
        return;
    }

    auto *dlg = new QDialog(this, Qt::Window);
    dlg->setWindowTitle("CCTV 영상 재생");
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
    player->setSource(QUrl::fromLocalFile(path));
    player->play();

    dlg->show();
}
