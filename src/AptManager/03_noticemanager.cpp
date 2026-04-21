#include "03_noticemanager.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

// 생성자 정의
NoticeManager::NoticeManager(QObject *parent) : QObject(parent) {}

// notice 테이블 생성 정의
bool NoticeManager::initialize()
{
    QSqlQuery q;
    if (!q.exec(
        "CREATE TABLE IF NOT EXISTS notice ("
        "  id         INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  createdAt  TEXT NOT NULL,"
        "  type       TEXT NOT NULL,"       // 일반 / 보안경고 / 주의인물
        "  title      TEXT NOT NULL,"
        "  content    TEXT,"
        "  videoPath  TEXT,"
        "  targetType TEXT DEFAULT '전체'," // 전체 / 동 / 세대
        "  targetDong INTEGER DEFAULT 0,"
        "  targetHo   INTEGER DEFAULT 0"
        ")")) {
        qWarning() << "notice 테이블 생성 실패:" << q.lastError().text();
        return false;
    }
    return true;
}

// 공지 등록 정의 - 성공 시 새 id 반환, 실패 시 -1
int NoticeManager::addNotice(const QString &type,
                              const QString &title,
                              const QString &content,
                              const QString &videoPath,
                              const QString &targetType,
                              int targetDong,
                              int targetHo)
{
    QSqlQuery q;
    q.prepare(
        "INSERT INTO notice "
        "  (createdAt, type, title, content, videoPath, targetType, targetDong, targetHo) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?)"
    );
    q.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    q.addBindValue(type);
    q.addBindValue(title);
    q.addBindValue(content);
    q.addBindValue(videoPath);
    q.addBindValue(targetType);
    q.addBindValue(targetDong);
    q.addBindValue(targetHo);

    if (!q.exec()) {
        qWarning() << "공지 등록 실패:" << q.lastError().text();
        return -1;
    }

    int id = q.lastInsertId().toInt();
    // 새 공지 등록을 알림 -> 공지 페이지 갱신
    emit noticeAdded(id);
    return id;
}

// id로 단일 공지 조회 정의
Notice NoticeManager::getNoticeById(int id)
{
    Notice n;
    QSqlQuery q;
    q.prepare(
        "SELECT id, createdAt, type, title, content, videoPath, targetType, targetDong, targetHo "
        "FROM notice WHERE id = ?"
    );
    q.addBindValue(id);
    if (q.exec() && q.next()) {
        n.id = q.value(0).toInt();
        n.createdAt = QDateTime::fromString(q.value(1).toString(), Qt::ISODate);
        n.type = q.value(2).toString();
        n.title = q.value(3).toString();
        n.content = q.value(4).toString();
        n.videoPath = q.value(5).toString();
        n.targetType = q.value(6).toString();
        n.targetDong = q.value(7).toInt();
        n.targetHo = q.value(8).toInt();
    }
    return n;
}

// 공지 목록 조회 정의 - typeFilter 없으면 전체
QList<Notice> NoticeManager::getNotices(const QString &typeFilter)
{
    QList<Notice> list;
    QSqlQuery q;

    // 필터 있으면 WHERE 절 추가
    if (typeFilter.isEmpty()) {
        q.exec(
            "SELECT id, createdAt, type, title, content, videoPath, targetType, targetDong, targetHo "
            "FROM notice ORDER BY id DESC"
        );
    } else {
        q.prepare(
            "SELECT id, createdAt, type, title, content, videoPath, targetType, targetDong, targetHo "
            "FROM notice WHERE type = ? ORDER BY id DESC"
        );
        q.addBindValue(typeFilter);
        q.exec();
    }

    while (q.next()) {
        Notice n;
        n.id = q.value(0).toInt();
        n.createdAt = QDateTime::fromString(q.value(1).toString(), Qt::ISODate);
        n.type = q.value(2).toString();
        n.title = q.value(3).toString();
        n.content = q.value(4).toString();
        n.videoPath = q.value(5).toString();
        n.targetType = q.value(6).toString();
        n.targetDong = q.value(7).toInt();
        n.targetHo = q.value(8).toInt();
        list.append(n);
    }
    return list;
}
