#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H

#include <QWidget>
#include "residentmanager.h"

class QLabel;
class QProgressBar;
class DonutWidget;

class DashboardPage : public QWidget
{
    Q_OBJECT
public:
    explicit DashboardPage(ResidentManager *mgr, QWidget *parent = nullptr);
    void refresh();

private:
    ResidentManager *m_mgr;

    // 통계 카드 값 라벨
    QLabel *m_lblResidents;
    QLabel *m_lblCars;
    QLabel *m_lblParked;
    QLabel *m_lblEmpty;

    // 층별 프로그레스바 + 수치 라벨
    QProgressBar *m_bar[3];
    QLabel       *m_barLbl[3];

    // 도넛 차트
    DonutWidget *m_donut;
    QLabel      *m_donutLbl;

    QWidget* makeCard(const QString &title, QLabel *&valueLbl, const QString &color);
};

#endif // DASHBOARDPAGE_H
