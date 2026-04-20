#include "parkingpage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>

// 주차장 구성
// 3층 × (3행 A/B/C × 10열 1–10) = 30자리/층 = 90자리 총
static constexpr int FLOORS     = 3;
static constexpr int ROWS       = 3;   // A, B, C
static constexpr int COLS       = 10;  // 1~10  (좌 1~5 / 우 6~10)
static constexpr int SPOTS_FLOOR = ROWS * COLS;

// 자리 ID: "{floor}-{rowLetter}{col}"  예) "1-A3", "2-C8"
static QString makeSpotId(int floor, int row, int col)
{
    return QString("%1-%2%3").arg(floor).arg(QChar('A' + row)).arg(col);
}

// 주차 버튼 스타일
static const char *kStyleEmpty =
    "QPushButton { background:#22C55E; color:white; border-radius:6px;"
    "  font-size:11px; font-weight:bold; }"
    "QPushButton:disabled { background:#22C55E; color:white; }";

static const char *kStyleOccupied =
    "QPushButton { background:#EF4444; color:white; border-radius:6px;"
    "  font-size:10px; font-weight:bold; }"
    "QPushButton:disabled { background:#EF4444; color:white; }";

// ─────────────────────────────────────────────────────────────────────────────

ParkingPage::ParkingPage(ResidentManager *mgr, QWidget *parent)
    : QWidget(parent), m_mgr(mgr)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 20, 20, 16);
    root->setSpacing(14);

    // 통계 라벨
    m_statsLabel = new QLabel(this);
    m_statsLabel->setAlignment(Qt::AlignCenter);
    m_statsLabel->setStyleSheet(
        "background:#F8FAFC; border:1px solid #E2E8F0; border-radius:8px;"
        "padding:10px 20px; color:#374151; font-size:13px;"
    );
    root->addWidget(m_statsLabel);

    // 탭
    m_tabs = new QTabWidget(this);
    m_tabs->setStyleSheet(
        "QTabWidget::pane {"
        "  border:1px solid #E2E8F0; border-radius:8px; background:#FFFFFF;"
        "}"
        "QTabBar::tab {"
        "  padding:9px 28px; color:#94A3B8; font-size:13px;"
        "  background:transparent; border:none;"
        "}"
        "QTabBar::tab:selected {"
        "  color:#2563EB; font-weight:bold;"
        "  border-bottom:2px solid #2563EB;"
        "}"
    );
    root->addWidget(m_tabs, 1);

    // 범례
    auto *legend = new QHBoxLayout;
    auto addLeg = [&](const char *color, const QString &text) {
        auto *dot = new QLabel;
        dot->setFixedSize(12, 12);
        dot->setStyleSheet(QString("background:%1; border-radius:6px;").arg(color));
        auto *lbl = new QLabel(text);
        lbl->setStyleSheet("color:#64748B; font-size:12px;");
        legend->addWidget(dot);
        legend->addWidget(lbl);
        legend->addSpacing(20);
    };
    legend->addStretch();
    addLeg("#22C55E", "빈 자리");
    addLeg("#EF4444", "주차 중");
    legend->addStretch();
    root->addLayout(legend);

    refresh();
}

void ParkingPage::refresh()
{
    // 점유 맵 구성
    QList<Car> allCars = m_mgr->getAllCars();
    QMap<QString, Car> occupied;
    int parkedCount = 0;
    for (const Car &c : allCars) {
        if (c.isParked && !c.parkingSpot.isEmpty()) {
            occupied[c.parkingSpot] = c;
            ++parkedCount;
        }
    }

    const int total = FLOORS * SPOTS_FLOOR;
    m_statsLabel->setText(
        QString("  주차 중  <b>%1</b>대   /   전체  <b>%2</b>자리   "
                "/   빈 자리  <b>%3</b>자리  ")
            .arg(parkedCount).arg(total).arg(total - parkedCount)
    );

    // 탭 재생성
    while (m_tabs->count() > 0) {
        QWidget *w = m_tabs->widget(0);
        m_tabs->removeTab(0);
        delete w;
    }
    for (int f = 1; f <= FLOORS; ++f)
        m_tabs->addTab(buildFloor(f, occupied), QString("  %1층  ").arg(f));
}

QWidget* ParkingPage::buildFloor(int floor, const QMap<QString, Car> &occupied)
{
    // 스크롤 가능하게 감싸기
    auto *scroll  = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *container = new QWidget;
    scroll->setWidget(container);

    auto *grid = new QGridLayout(container);
    grid->setSpacing(5);
    grid->setContentsMargins(20, 16, 20, 20);

    // ── 열 번호 헤더 (row 0) ──
    // col 0: blank (행 라벨 자리)
    // cols 1–5: "1"~"5"  (좌 구역)
    // col 6: 통로 라벨
    // cols 7–11: "6"~"10" (우 구역)

    auto headerLbl = [](const QString &t) {
        auto *l = new QLabel(t);
        l->setAlignment(Qt::AlignCenter);
        l->setStyleSheet("color:#94A3B8; font-size:11px; font-weight:bold;");
        return l;
    };
    grid->addWidget(headerLbl(""), 0, 0);
    for (int c = 1; c <= 5;  ++c) grid->addWidget(headerLbl(QString::number(c)),     0, c);
    grid->addWidget(headerLbl(""),  0, 6);   // 통로
    for (int c = 6; c <= 10; ++c) grid->addWidget(headerLbl(QString::number(c)),     0, c + 1);

    // ── 진입 방향 화살표 (열 헤더 위) ──
    // (생략 — 범례로 충분)

    // ── 통로 구분선 ──
    for (int r = 0; r < ROWS; ++r) {
        auto *lane = new QFrame(container);
        lane->setFixedWidth(20);
        lane->setStyleSheet("background:#E2E8F0; border-radius:3px;");
        grid->addWidget(lane, r + 1, 6);
    }

    // ── 행 라벨 + 주차 자리 ──
    for (int r = 0; r < ROWS; ++r) {
        // 행 라벨 (A/B/C)
        auto *rowLbl = new QLabel(QString(QChar('A' + r)));
        rowLbl->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        rowLbl->setFixedWidth(26);
        rowLbl->setStyleSheet("color:#374151; font-size:13px; font-weight:bold;");
        grid->addWidget(rowLbl, r + 1, 0);

        for (int c = 1; c <= COLS; ++c) {
            QString id  = makeSpotId(floor, r, c);
            bool    occ = occupied.contains(id);

            auto *btn = new QPushButton(container);
            btn->setFixedSize(66, 56);
            btn->setEnabled(false);

            if (occ) {
                btn->setText(QString("%1%2\n주차 중").arg(QChar('A' + r)).arg(c));
                btn->setStyleSheet(kStyleOccupied);
            } else {
                btn->setText(QString("%1%2\n빈 자리").arg(QChar('A' + r)).arg(c));
                btn->setStyleSheet(kStyleEmpty);
            }

            // 좌(1–5): grid col c,  우(6–10): grid col c+1 (통로 건너뜀)
            int gridCol = (c <= 5) ? c : c + 1;
            grid->addWidget(btn, r + 1, gridCol);
        }
    }

    return scroll;
}

