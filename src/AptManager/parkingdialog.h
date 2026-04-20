#ifndef PARKINGDIALOG_H
#define PARKINGDIALOG_H

#include <QDialog>
#include <QMap>
#include "residentmanager.h"

class QTabWidget;
class QLabel;

class ParkingDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ParkingDialog(ResidentManager *mgr, QWidget *parent = nullptr);

private:
    void refresh();
    QWidget* buildFloor(int floor, const QMap<QString, Car> &occupied);
    void onSpotClicked(const QString &spotId, bool isOccupied);

    ResidentManager *m_mgr;
    QTabWidget *m_tabs;
    QLabel     *m_statsLabel;
};

#endif // PARKINGDIALOG_H
