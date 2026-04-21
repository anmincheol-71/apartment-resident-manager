#ifndef ALERTPOPUP_H
#define ALERTPOPUP_H

#include <QDateTime>
#include <QWidget>

class SecurityAlertManager;

// 보안 이상징후 발생 시 화면 우측 하단에 고정 표시되는 경고 팝업
// Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint 로 항상 최상단 유지
class AlertPopup : public QWidget {
    Q_OBJECT

public:
    explicit AlertPopup(int alertId,
                        const QString &location,
                        const QString &type,
                        const QDateTime &occurredAt,
                        const QString &videoPath,
                        SecurityAlertManager *mgr,
                        QWidget *parent = nullptr);

signals:
    // 확인 완료 버튼 클릭 시 방출 - 내역 페이지 갱신 트리거용
    void confirmed(int alertId);

    // 주의 공지 등록 버튼 클릭 시 방출 - MainWindow가 받아 NoticeDialog 오픈
    // noticeType  보안경고 또는 주의인물 (알림 유형에 따라 자동 결정)
    void noticeRequested(const QString &noticeType,
                         const QString &title,
                         const QString &videoPath);

private slots:
    void onConfirm();
    void onPlayVideo();
    // 공지 등록 버튼 -> noticeRequested 시그널 방출
    void onRegisterNotice();

private:
    int m_alertId;
    QString m_location;  // 위치 정보 (공지 제목 생성용)
    QString m_type;      // 알림 유형 (공지 유형 결정용)
    QString m_videoPath;
    SecurityAlertManager *m_mgr;
};

#endif // ALERTPOPUP_H
