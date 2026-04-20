#ifndef PARKINGPAGE_H
#define PARKINGPAGE_H

#include <QWidget>
#include <QMap>
#include "residentmanager.h"

class QTabWidget;
class QLabel;

class ParkingPage : public QWidget
{
    Q_OBJECT
public:
    explicit ParkingPage(ResidentManager *mgr, QWidget *parent = nullptr);
    void refresh();

private:
    QWidget* buildFloor(int floor, const QMap<QString, Car> &occupied);

    ResidentManager *m_mgr;
    QTabWidget *m_tabs;
    QLabel     *m_statsLabel;
};

#endif // PARKINGPAGE_H
