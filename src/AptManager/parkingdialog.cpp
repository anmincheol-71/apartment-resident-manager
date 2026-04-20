#include "parkingdialog.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QInputDialog>
#include <QFrame>

// 주차장 구성: 3층 × 30자리 (3열 × 5행 좌 / 통로 / 3열 × 5행 우)
static const int FLOORS           = 3;
static const int SPOTS_PER_FLOOR  = 30;
static const int COLS_PER_SECTION = 3;
static const int ROWS             = 5;

static QString makeSpotId(int floor, int spot)
{
    return QString("%1-%2").arg(floor).arg(spot, 2, 10, QChar('0'));
}

// ── 스팟 버튼 스타일 ──────────────────────────────────────────────────────────
static const char *kStyleEmpty =
    "QPushButton {"
    "  background: #22C55E; color: white; border-radius: 6px;"
    "  font-size: 11px; font-weight: bold; padding: 4px;"
    "}"
    "QPushButton:hover { background: #16A34A; }";

static const char *kStyleOccupied =
    "QPushButton {"
    "  background: #EF4444; color: white; border-radius: 6px;"
    "  font-size: 10px; font-weight: bold; padding: 4px;"
    "}"
    "QPushButton:hover { background: #DC2626; }";

// ─────────────────────────────────────────────────────────────────────────────

ParkingDialog::ParkingDialog(ResidentManager *mgr, QWidget *parent)
    : QDialog(parent), m_mgr(mgr)
{
    setWindowTitle("주차장 현황");
    setMinimumSize(620, 520);

    auto *root = new QVBoxLayout(this);
    root->setSpacing(12);
    root->setContentsMargins(16, 16, 16, 12);

    // 통계 라벨
    m_statsLabel = new QLabel(this);
    m_statsLabel->setAlignment(Qt::AlignCenter);
    m_statsLabel->setStyleSheet(
        "background:#F8FAFC; border:1px solid #E2E8F0; border-radius:6px;"
        "padding:8px 16px; color:#374151; font-size:13px;"
    );
    root->addWidget(m_statsLabel);

    // 층 탭
    m_tabs = new QTabWidget(this);
    m_tabs->setStyleSheet(
        "QTabWidget::pane {"
        "  border:1px solid #E2E8F0; border-radius:8px; background:#FFFFFF;"
        "  margin-top:-1px;"
        "}"
        "QTabBar::tab {"
        "  padding:8px 24px; color:#94A3B8; font-size:13px;"
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
    auto addItem = [&](const char *color, const QString &text) {
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
    addItem("#22C55E", "빈 자리");
    addItem("#EF4444", "주차 중");
    legend->addStretch();
    root->addLayout(legend);

    // 닫기 버튼
    auto *btnBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    root->addWidget(btnBox);

    refresh();
}

void ParkingDialog::refresh()
{
    // 주차 중인 차량 맵 구성
    QList<Car> allCars = m_mgr->getAllCars();
    QMap<QString, Car> occupied;
    int parkedCount = 0;
    for (const Car &c : allCars) {
        if (c.isParked && !c.parkingSpot.isEmpty()) {
            occupied[c.parkingSpot] = c;
            ++parkedCount;
        }
    }

    // 통계 갱신
    const int total = FLOORS * SPOTS_PER_FLOOR;
    m_statsLabel->setText(
        QString("  주차 중  <b>%1</b>대   /   전체  <b>%2</b>자리   /   빈 자리  <b>%3</b>자리  ")
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

QWidget* ParkingDialog::buildFloor(int floor, const QMap<QString, Car> &occupied)
{
    auto *container = new QWidget;
    auto *grid      = new QGridLayout(container);
    grid->setSpacing(6);
    grid->setContentsMargins(20, 16, 20, 16);

    // 통로 구분선 (열 3)
    for (int row = 0; row < ROWS; ++row) {
        auto *lane = new QFrame(container);
        lane->setFixedWidth(18);
        lane->setStyleSheet("background:#E2E8F0; border-radius:3px;");
        grid->addWidget(lane, row, 3);
    }

    for (int spot = 1; spot <= SPOTS_PER_FLOOR; ++spot) {
        bool isLeft = (spot <= 15);
        int  idx    = isLeft ? (spot - 1) : (spot - 16);
        int  row    = idx / COLS_PER_SECTION;
        int  col    = idx % COLS_PER_SECTION + (isLeft ? 0 : 4);

        QString id   = makeSpotId(floor, spot);
        bool    isOcc = occupied.contains(id);

        auto *btn = new QPushButton(container);
        btn->setFixedSize(88, 64);
        btn->setCursor(Qt::PointingHandCursor);

        if (isOcc) {
            const Car &c = occupied[id];
            Resident r   = m_mgr->getResidentById(c.residentId);
            btn->setText(
                QString("%1\n%2\n%3동%4호")
                    .arg(spot, 2, 10, QChar('0'))
                    .arg(c.carNumber)
                    .arg(r.dong).arg(r.ho)
            );
            btn->setStyleSheet(kStyleOccupied);
        } else {
            btn->setText(QString("%1\n빈 자리").arg(spot, 2, 10, QChar('0')));
            btn->setStyleSheet(kStyleEmpty);
        }

        connect(btn, &QPushButton::clicked, this,
            [this, id, isOcc]() { onSpotClicked(id, isOcc); });

        grid->addWidget(btn, row, col);
    }

    return container;
}

void ParkingDialog::onSpotClicked(const QString &spotId, bool isOccupied)
{
    QList<Car> allCars = m_mgr->getAllCars();

    if (isOccupied) {
        // 해당 자리 차량 찾기
        Car parkedCar;
        for (const Car &c : allCars) {
            if (c.parkingSpot == spotId && c.isParked) { parkedCar = c; break; }
        }
        Resident r = m_mgr->getResidentById(parkedCar.residentId);

        QMessageBox mb(this);
        mb.setWindowTitle("주차 정보");
        mb.setTextFormat(Qt::RichText);
        mb.setText(
            QString(
                "<table cellspacing='6'>"
                "<tr><td><b>자리</b></td><td>%1층 %2번</td></tr>"
                "<tr><td><b>차량 번호</b></td><td>%3</td></tr>"
                "<tr><td><b>세대주</b></td><td>%4 (%5동 %6호)</td></tr>"
                "<tr><td><b>연락처</b></td><td>%7</td></tr>"
                "</table>"
            )
            .arg(spotId.section('-', 0, 0))
            .arg(spotId.section('-', 1, 1))
            .arg(parkedCar.carNumber)
            .arg(r.name).arg(r.dong).arg(r.ho)
            .arg(r.phone.isEmpty() ? "-" : r.phone)
        );
        auto *outBtn = mb.addButton("출차", QMessageBox::AcceptRole);
        mb.addButton("닫기", QMessageBox::RejectRole);
        mb.exec();

        if (mb.clickedButton() == outBtn) {
            m_mgr->unparkCar(parkedCar.id);
            refresh();
        }
    } else {
        // 미주차 차량 선택 후 입차
        QList<Car> unparked = m_mgr->getUnparkedCars();
        if (unparked.isEmpty()) {
            QMessageBox::information(this, "알림", "입차 가능한 차량이 없습니다.");
            return;
        }

        QStringList items;
        for (const Car &c : unparked) {
            Resident r = m_mgr->getResidentById(c.residentId);
            items << QString("%1   —   %2동 %3호  %4")
                        .arg(c.carNumber).arg(r.dong).arg(r.ho).arg(r.name);
        }

        bool ok;
        QString sel = QInputDialog::getItem(
            this, "입차",
            QString("%1층 %2번 자리에 입차할 차량을 선택하세요.")
                .arg(spotId.section('-', 0, 0))
                .arg(spotId.section('-', 1, 1)),
            items, 0, false, &ok
        );
        if (!ok) return;

        int idx = items.indexOf(sel);
        if (idx >= 0) {
            m_mgr->parkCar(unparked[idx].id, spotId);
            refresh();
        }
    }
}
