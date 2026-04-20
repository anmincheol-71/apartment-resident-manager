#include "dashboardpage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QProgressBar>
#include <QGroupBox>
#include <QFrame>
#include <QPainter>
#include <QDate>
#include <QtMath>
#include <QSqlQuery>

static constexpr int FLOORS = 3;

// ── 도넛 차트 위젯 ────────────────────────────────────────────────────────────
class DonutWidget : public QWidget
{
public:
    explicit DonutWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setMinimumSize(200, 200);
    }
    void setData(int occupied, int total)
    {
        m_occupied = occupied;
        m_total    = total;
        update();
    }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        int side     = qMin(width(), height()) - 20;
        int outerR   = side / 2;
        int innerR   = outerR * 55 / 100;
        int ringW    = outerR - innerR;
        int centerR  = (outerR + innerR) / 2;

        QPoint ctr(width() / 2, height() / 2);
        QRect  arcRect(ctr.x() - centerR, ctr.y() - centerR, centerR * 2, centerR * 2);

        // 빈 자리 (초록 전체)
        QPen greenPen(QColor("#22C55E"), ringW, Qt::SolidLine, Qt::FlatCap);
        p.setPen(greenPen);
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(arcRect);

        // 주차 중 (빨강 호)
        double frac = (m_total > 0) ? double(m_occupied) / m_total : 0.0;
        if (frac > 0.0) {
            QPen redPen(QColor("#EF4444"), ringW, Qt::SolidLine, Qt::FlatCap);
            p.setPen(redPen);
            int startAngle = 90 * 16;
            int spanAngle  = -qRound(frac * 360.0 * 16);
            p.drawArc(arcRect, startAngle, spanAngle);
        }

        // 중앙 텍스트: 퍼센트
        int pct = (m_total > 0) ? qRound(frac * 100) : 0;
        QFont bigFont("Malgun Gothic", 22, QFont::Bold);
        p.setFont(bigFont);
        p.setPen(QColor("#1E293B"));
        QRect upper = arcRect.adjusted(0, arcRect.height() / 5, 0, 0);
        p.drawText(upper, Qt::AlignHCenter | Qt::AlignTop, QString("%1%").arg(pct));

        QFont smallFont("Malgun Gothic", 10);
        p.setFont(smallFont);
        p.setPen(QColor("#64748B"));
        QRect lower = arcRect.adjusted(0, arcRect.height() * 3 / 5, 0, 0);
        p.drawText(lower, Qt::AlignHCenter | Qt::AlignTop, "주차 점유율");
    }
private:
    int m_occupied = 0;
    int m_total    = 90;
};

// ─────────────────────────────────────────────────────────────────────────────

static QLabel* rowLabel(const QString &text, const QString &style)
{
    auto *l = new QLabel(text);
    l->setStyleSheet(style);
    return l;
}

DashboardPage::DashboardPage(ResidentManager *mgr, QWidget *parent)
    : QWidget(parent), m_mgr(mgr)
{
    setObjectName("dashboardPage");

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(20);

    // ── 환영 문구 ──
    auto *hdr = new QHBoxLayout;
    auto *welcome = rowLabel("대시보드", "font-size:20px; font-weight:bold; color:#1E293B;");
    auto *dateLbl = rowLabel(QDate::currentDate().toString("yyyy년 MM월 dd일"),
                             "font-size:13px; color:#94A3B8;");
    hdr->addWidget(welcome);
    hdr->addStretch();
    hdr->addWidget(dateLbl);
    root->addLayout(hdr);

    // ── 통계 카드 4개 ──
    auto *cardsRow = new QHBoxLayout;
    cardsRow->setSpacing(16);
    cardsRow->addWidget(makeCard("총 세대수",   m_lblResidents, "#2563EB"));
    cardsRow->addWidget(makeCard("등록 차량",   m_lblCars,      "#7C3AED"));
    cardsRow->addWidget(makeCard("주차 중",     m_lblParked,    "#EF4444"));
    cardsRow->addWidget(makeCard("빈 자리",     m_lblEmpty,     "#22C55E"));
    root->addLayout(cardsRow);

    // ── 하단 2열 ──
    auto *contentRow = new QHBoxLayout;
    contentRow->setSpacing(16);

    // 왼쪽: 층별 프로그레스바
    auto *floorGroup = new QGroupBox("층별 주차 점유율");
    floorGroup->setStyleSheet(
        "QGroupBox { background:white; border:1px solid #E2E8F0; border-radius:12px;"
        "  margin-top:14px; padding:16px; font-weight:bold; color:#374151; }"
        "QGroupBox::title { subcontrol-origin:margin; left:14px;"
        "  padding:0 6px; background:white; }"
    );
    auto *floorLayout = new QVBoxLayout(floorGroup);
    floorLayout->setSpacing(18);

    static const char *floorNames[] = {"1층", "2층", "3층"};
    for (int i = 0; i < 3; ++i) {
        auto *row = new QHBoxLayout;
        row->setSpacing(10);

        auto *nameLbl = rowLabel(floorNames[i],
            "font-size:13px; font-weight:bold; color:#374151; min-width:28px;");

        m_bar[i] = new QProgressBar;
        m_bar[i]->setRange(0, 30);
        m_bar[i]->setValue(0);
        m_bar[i]->setTextVisible(false);
        m_bar[i]->setFixedHeight(14);
        m_bar[i]->setStyleSheet(
            "QProgressBar { background:#F1F5F9; border-radius:7px; border:none; }"
            "QProgressBar::chunk { background:#2563EB; border-radius:7px; }"
        );

        m_barLbl[i] = rowLabel("0 / 30",
            "font-size:12px; color:#64748B; min-width:52px; text-align:right;");
        m_barLbl[i]->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        row->addWidget(nameLbl);
        row->addWidget(m_bar[i], 1);
        row->addWidget(m_barLbl[i]);
        floorLayout->addLayout(row);
    }
    floorLayout->addStretch();

    // 오른쪽: 도넛 차트
    auto *donutGroup = new QGroupBox("주차 현황");
    donutGroup->setStyleSheet(
        "QGroupBox { background:white; border:1px solid #E2E8F0; border-radius:12px;"
        "  margin-top:14px; padding:16px; font-weight:bold; color:#374151; }"
        "QGroupBox::title { subcontrol-origin:margin; left:14px;"
        "  padding:0 6px; background:white; }"
    );
    auto *donutLayout = new QVBoxLayout(donutGroup);
    donutLayout->setSpacing(12);

    m_donut = new DonutWidget(donutGroup);
    donutLayout->addWidget(m_donut, 1, Qt::AlignCenter);

    // 범례
    auto *legend = new QHBoxLayout;
    legend->setSpacing(20);
    auto addLegend = [&](const QString &color, const QString &text) {
        auto *dot = new QLabel;
        dot->setFixedSize(12, 12);
        dot->setStyleSheet(QString("background:%1; border-radius:6px;").arg(color));
        auto *lbl = rowLabel(text, "color:#64748B; font-size:12px;");
        legend->addWidget(dot);
        legend->addWidget(lbl);
    };
    legend->addStretch();
    addLegend("#EF4444", "주차 중");
    addLegend("#22C55E", "빈 자리");
    legend->addStretch();
    donutLayout->addLayout(legend);

    contentRow->addWidget(floorGroup, 3);
    contentRow->addWidget(donutGroup, 2);
    root->addLayout(contentRow, 1);

    refresh();
}

QWidget* DashboardPage::makeCard(const QString &title, QLabel *&valueLbl, const QString &color)
{
    auto *card = new QFrame(this);
    card->setStyleSheet(
        "QFrame { background:white; border:1px solid #E2E8F0; border-radius:12px; }"
    );
    auto *vbox = new QVBoxLayout(card);
    vbox->setContentsMargins(20, 18, 20, 18);
    vbox->setSpacing(6);

    auto *titleLbl = new QLabel(title, card);
    titleLbl->setStyleSheet("color:#94A3B8; font-size:12px; font-weight:bold;");

    valueLbl = new QLabel("—", card);
    valueLbl->setStyleSheet(
        QString("color:%1; font-size:30px; font-weight:bold;").arg(color)
    );

    vbox->addWidget(titleLbl);
    vbox->addWidget(valueLbl);
    card->setMinimumWidth(160);
    return card;
}

void DashboardPage::refresh()
{
    // 세대 수
    int residents = 0;
    {
        QSqlQuery q("SELECT COUNT(*) FROM resident");
        if (q.next()) residents = q.value(0).toInt();
    }

    // 차량 집계
    QList<Car> cars = m_mgr->getAllCars();
    int totalCars = cars.size();
    int parked = 0;
    int floorCount[3] = {0, 0, 0};

    for (const Car &c : cars) {
        if (!c.isParked) continue;
        ++parked;
        QString fl = c.parkingSpot.section('-', 0, 0);
        if      (fl == "1") ++floorCount[0];
        else if (fl == "2") ++floorCount[1];
        else if (fl == "3") ++floorCount[2];
    }

    const int SPOTS = 30;  // 층당 자리 수
    int empty = FLOORS * SPOTS - parked;

    // 카드 갱신
    m_lblResidents->setText(QString::number(residents));
    m_lblCars->setText(QString::number(totalCars));
    m_lblParked->setText(QString::number(parked));
    m_lblEmpty->setText(QString::number(empty < 0 ? 0 : empty));

    // 층별 프로그레스바
    for (int i = 0; i < 3; ++i) {
        m_bar[i]->setValue(floorCount[i]);
        m_barLbl[i]->setText(QString("%1 / %2").arg(floorCount[i]).arg(SPOTS));

        // 점유율에 따라 색상 변경
        QString chunk = floorCount[i] >= SPOTS * 9 / 10 ? "#EF4444"
                      : floorCount[i] >= SPOTS * 6 / 10 ? "#F59E0B"
                      : "#2563EB";
        m_bar[i]->setStyleSheet(
            "QProgressBar { background:#F1F5F9; border-radius:7px; border:none; }"
            "QProgressBar::chunk { background:" + chunk + "; border-radius:7px; }"
        );
    }

    // 도넛 갱신
    m_donut->setData(parked, FLOORS * SPOTS);
}
