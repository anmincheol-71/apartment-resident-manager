#ifndef ALERTHISTORYPAGE_H
#define ALERTHISTORYPAGE_H

#include <QWidget>

class SecurityAlertManager;
class QLabel;
class QTableWidget;

// 보안 알림 발생 이력을 표시하는 페이지
// 테스트 알림 발생 버튼으로 시나리오 시뮬레이션 가능
class AlertHistoryPage : public QWidget {
    Q_OBJECT

public:
    explicit AlertHistoryPage(SecurityAlertManager *mgr, QWidget *parent = nullptr);

public slots:
    // 목록 새로고침 - SecurityAlertManager::newAlert 시그널에도 연결됨
    void refresh();

private slots:
    // 테스트 알림 발생 - 두 CCTV 영상을 번갈아 사용
    void onSimulateAlert();

    // 행 더블클릭 시 해당 알림의 CCTV 영상 재생
    void onRowDoubleClicked(int row, int col);

private:
    SecurityAlertManager *m_mgr;
    QTableWidget *m_table;
    QLabel *m_countLabel;

    // QMediaPlayer 팝업으로 영상 재생
    void playVideo(const QString &path);
};

#endif // ALERTHISTORYPAGE_H
