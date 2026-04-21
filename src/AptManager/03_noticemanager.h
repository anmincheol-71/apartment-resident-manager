#ifndef NOTICEMANAGER_H
#define NOTICEMANAGER_H

#include <QList>
#include <QObject>
#include "03_notice.h"

class NoticeManager : public QObject {
    Q_OBJECT

public:
    explicit NoticeManager(QObject *parent = nullptr);

    // notice 테이블 생성 - 성공 시 true
    bool initialize();

    // 공지 등록 - 성공 시 새 id 반환, 실패 시 -1
    // type        공지 유형 (일반 / 보안경고 / 주의인물)
    // title       공지 제목
    // content     공지 내용
    // videoPath   첨부 영상 경로 (없으면 빈 문자열)
    // targetType  수신 대상 (전체 / 동 / 세대)
    // targetDong  대상 동 번호 (전체면 0)
    // targetHo    대상 호 번호 (동/전체면 0)
    int addNotice(const QString &type,
                  const QString &title,
                  const QString &content,
                  const QString &videoPath,
                  const QString &targetType,
                  int targetDong,
                  int targetHo);

    // id로 단일 공지 조회 - 성공 시 Notice 반환
    Notice getNoticeById(int id);

    // 공지 목록 조회 - 최신순, typeFilter 비어있으면 전체
    QList<Notice> getNotices(const QString &typeFilter = {});

signals:
    // 새 공지 등록 시 방출 -> 공지 페이지 갱신
    void noticeAdded(int noticeId);
};

#endif // NOTICEMANAGER_H
