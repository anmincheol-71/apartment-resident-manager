#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "01_residentmanager.h"
#include "02_securityalertmanager.h"
#include "03_noticemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class AlertHistoryPage;
class NoticePage;
class ResidentPage;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    // SecurityAlertManager::newAlert 시그널 수신 -> 경고 팝업 생성
    void showAlertPopup(int alertId);

    // AlertPopup::noticeRequested 시그널 수신 -> NoticeDialog(작성 모드) 오픈
    void onRegisterNotice(const QString &noticeType,
                          const QString &title,
                          const QString &videoPath);

    // 상태바 갱신 - 총 세대수 + 미확인 보안 알림 건수
    void updateStatusBar();

private:
    Ui::MainWindow *ui;
    ResidentManager *m_mgr;
    SecurityAlertManager *m_alertMgr;
    NoticeManager *m_noticeMgr;

    ResidentPage *m_residentPage;     // stackedWidget index 0
    AlertHistoryPage *m_alertPage;    // stackedWidget index 1
    NoticePage *m_noticePage;         // stackedWidget index 2
};

#endif // MAINWINDOW_H
