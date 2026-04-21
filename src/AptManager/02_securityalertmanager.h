#ifndef SECURITYALERTMANAGER_H
#define SECURITYALERTMANAGER_H

#include <QObject>
#include <QList>
#include "02_securityalert.h"

class SecurityAlertManager : public QObject {
    Q_OBJECT
public:
    explicit SecurityAlertManager(QObject *parent = nullptr);

    // security_alert 테이블 생성 - 성공 시 true
    bool initialize();

    // 보안 알림 등록 - 성공 시 새 id 반환, 실패 시 -1
    // location   발생 위치
    // type       알림 유형
    // videoPath  연결할 CCTV 영상 경로
    int addAlert(const QString &location, const QString &type, const QString &videoPath);

    // id의 알림을 확인됨으로 표시 - 성공 시 true
    bool confirmAlert(int id);

    // id로 단일 알림 조회 - 성공 시 SecurityAlert 반환
    SecurityAlert getAlertById(int id);

    // 전체 알림 목록 조회 - 최신순
    QList<SecurityAlert> getAlerts();

signals:
    // 새 알림 등록 시 방출 -> 팝업 표시 및 내역 갱신
    void newAlert(int alertId);
};

#endif // SECURITYALERTMANAGER_H
