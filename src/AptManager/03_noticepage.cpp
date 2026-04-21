#include "03_noticepage.h"
#include "03_noticemanager.h"
#include "03_notice.h"
#include "03_noticedialog.h"

#include <QColor>
#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

// ── 생성자 ─────────────────────────────────────────────────────────────────
NoticePage::NoticePage(NoticeManager *mgr, QWidget *parent)
    : QWidget(parent)
    , m_mgr(mgr)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(12);

    // ── 헤더 ─────────────────────────────────────────────
    auto *header = new QHBoxLayout;

    auto *title = new QLabel("공지사항");
    title->setStyleSheet("font-size: 20px; font-weight: bold;");
    header->addWidget(title);

    m_countLabel = new QLabel;
    m_countLabel->setStyleSheet("color: #888; font-size: 13px; margin-left: 10px;");
    header->addWidget(m_countLabel);
    header->addStretch();

    // 유형 필터 콤보박스
    m_filterCombo = new QComboBox;
    m_filterCombo->addItems({"전체", "일반", "보안경고", "주의인물"});
    m_filterCombo->setFixedWidth(110);
    header->addWidget(new QLabel("필터:"));
    header->addWidget(m_filterCombo);

    auto *addBtn = new QPushButton("📋  공지 등록");
    addBtn->setCursor(Qt::PointingHandCursor);
    addBtn->setStyleSheet(
        "QPushButton { background-color:#37474F; color:white; border-radius:5px;"
        "              padding:8px 16px; font-size:13px; }"
        "QPushButton:hover { background-color:#263238; }"
    );
    header->addWidget(addBtn);

    root->addLayout(header);

    auto *hint = new QLabel("행을 더블클릭하면 상세 내용 및 영상을 확인할 수 있습니다.");
    hint->setStyleSheet("color: #999; font-size: 12px;");
    root->addWidget(hint);

    // ── 테이블 ─────────────────────────────────────────────
    // 컬럼: 유형 | 제목 | 수신대상 | 영상 | 등록일시
    m_table = new QTableWidget;
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels({"유형", "제목", "수신대상", "영상", "등록일시"});
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->hide();
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_table->setStyleSheet(
        "QTableWidget { font-size:13px; }"
        "QHeaderView::section { background-color:#37474F; color:white; padding:6px;"
        "                       font-weight:bold; border:none; }"
    );
    root->addWidget(m_table);

    connect(addBtn,        &QPushButton::clicked,            this, &NoticePage::onAddNotice);
    connect(m_filterCombo, &QComboBox::currentIndexChanged,  this, &NoticePage::onFilterChanged);
    connect(m_table,       &QTableWidget::cellDoubleClicked, this, &NoticePage::onRowDoubleClicked);

    refresh();
}

// ── 슬롯 구현 ──────────────────────────────────────────────────────────────

// 공지 목록 새로고침 - 현재 필터 기준으로 DB 조회 후 테이블 갱신
void NoticePage::refresh()
{
    QString filter = m_filterCombo->currentText();
    QList<Notice> notices = m_mgr->getNotices(filter == "전체" ? QString() : filter);

    m_table->setRowCount(notices.size());

    for (int i = 0; i < notices.size(); ++i) {
        const Notice &n = notices[i];

        // 유형 - 색상으로 구분
        auto *typeItem = new QTableWidgetItem(n.type);
        typeItem->setTextAlignment(Qt::AlignCenter);
        typeItem->setForeground(typeColor(n.type));
        typeItem->setFont(QFont(QString(), -1, QFont::Bold));
        m_table->setItem(i, 0, typeItem);

        // 제목 - UserRole에 notice id 저장 (더블클릭 시 조회용)
        auto *titleItem = new QTableWidgetItem(n.title);
        titleItem->setData(Qt::UserRole, n.id);
        m_table->setItem(i, 1, titleItem);

        // 수신대상 표시
        QString target;
        if (n.targetType == "동")        target = QString("%1동").arg(n.targetDong);
        else if (n.targetType == "세대") target = QString("%1동 %2호").arg(n.targetDong).arg(n.targetHo);
        else                             target = "전체 세대";
        auto *targetItem = new QTableWidgetItem(target);
        targetItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(i, 2, targetItem);

        // 영상 첨부 여부
        auto *videoItem = new QTableWidgetItem(n.videoPath.isEmpty() ? "—" : "▶ 재생");
        videoItem->setTextAlignment(Qt::AlignCenter);
        if (!n.videoPath.isEmpty())
            videoItem->setForeground(QColor("#1565C0"));
        m_table->setItem(i, 3, videoItem);

        m_table->setItem(i, 4, new QTableWidgetItem(
            n.createdAt.toString("yyyy-MM-dd  hh:mm:ss")));
    }

    m_countLabel->setText(QString("총 %1건").arg(notices.size()));
}

// 공지 등록 버튼 -> 빈 폼으로 NoticeDialog 열기
void NoticePage::onAddNotice()
{
    NoticeDialog dlg({}, m_mgr, this);
    if (dlg.exec() == QDialog::Accepted)
        refresh();
}

// 행 더블클릭 -> 해당 공지 NoticeDialog(열람 모드) 열기
void NoticePage::onRowDoubleClicked(int row, int col)
{
    Q_UNUSED(col)

    auto *titleItem = m_table->item(row, 1);
    if (!titleItem) return;

    int noticeId = titleItem->data(Qt::UserRole).toInt();
    Notice n = m_mgr->getNoticeById(noticeId);
    if (n.id == 0) return;

    // mgr = nullptr -> 열람 모드
    NoticeDialog dlg(n, nullptr, this);
    dlg.exec();
}

// 유형 필터 변경 -> 목록 갱신
void NoticePage::onFilterChanged()
{
    refresh();
}

// ── private 헬퍼 ───────────────────────────────────────────────────────────

// 공지 유형에 따른 텍스트 색상 반환
QColor NoticePage::typeColor(const QString &type)
{
    if (type == "주의인물") return QColor("#D32F2F"); // 빨강
    if (type == "보안경고") return QColor("#E65100"); // 주황
    return QColor("#1565C0");                          // 파랑 (일반)
}
