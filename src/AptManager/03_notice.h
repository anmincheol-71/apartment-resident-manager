#ifndef NOTICE_H
#define NOTICE_H

#include <QDateTime>
#include <QString>

// 공지사항 항목 구조체
struct Notice {
    int id = 0;
    QDateTime createdAt;
    QString type;       // 일반 / 보안경고 / 주의인물
    QString title;
    QString content;
    QString videoPath;  // 첨부 영상 경로 (없으면 빈 문자열)
    QString targetType; // 전체 / 동 / 세대
    int targetDong = 0;
    int targetHo = 0;
};

#endif // NOTICE_H
