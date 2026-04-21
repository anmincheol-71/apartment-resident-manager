#ifndef SECURITYALERT_H
#define SECURITYALERT_H

#include <QString>
#include <QDateTime>

// 보안 알림 항목 구조체
struct SecurityAlert {
    int id = 0;
    QDateTime occurredAt;   // 알림 발생 시각
    QString location;       // 발생 위치
    QString type;           // 알림 유형
    QString videoPath;      // 연결된 CCTV 영상 경로 (없으면 빈 문자열)
    bool isConfirmed = false; // 관리자 확인 여부
};

#endif // SECURITYALERT_H
