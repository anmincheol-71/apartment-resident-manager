#include "02_securityalertmanager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

// 생성자 정의
SecurityAlertManager::SecurityAlertManager(QObject *parent) : QObject(parent) {}

// security_alert 테이블 생성 정의
bool SecurityAlertManager::initialize()
{
    QSqlQuery q;
    if (!q.exec(
        "CREATE TABLE IF NOT EXISTS security_alert ("
        "  id          INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  occurredAt  TEXT    NOT NULL,"
        "  location    TEXT    NOT NULL,"
        "  type        TEXT    NOT NULL,"
        "  videoPath   TEXT,"
        "  isConfirmed INTEGER DEFAULT 0"
        ")")) {
        qWarning() << "security_alert 테이블 생성 실패:" << q.lastError().text();
        return false;
    }
    return true;
}

// 보안 알림 등록 정의 - 성공 시 새 id 반환, 실패 시 -1
// location   발생 위치
// type       알림 유형
// videoPath  연결할 CCTV 영상 경로
int SecurityAlertManager::addAlert(const QString &location, const QString &type, const QString &videoPath)
{
    QSqlQuery q;
    q.prepare(
        "INSERT INTO security_alert (occurredAt, location, type, videoPath, isConfirmed) "
        "VALUES (?, ?, ?, ?, 0)"
    );
    q.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    q.addBindValue(location);
    q.addBindValue(type);
    q.addBindValue(videoPath);

    if (!q.exec()) {
        qWarning() << "알림 등록 실패:" << q.lastError().text();
        return -1;
    }

    int id = q.lastInsertId().toInt();
    // 새 알림 발생을 알림 -> 팝업 표시 및 내역 갱신
    emit newAlert(id);
    return id;
}

// id의 알림을 확인됨으로 표시 정의
bool SecurityAlertManager::confirmAlert(int id)
{
    QSqlQuery q;
    q.prepare("UPDATE security_alert SET isConfirmed = 1 WHERE id = ?");
    q.addBindValue(id);
    return q.exec();
}

// id로 단일 알림 조회 정의 - 성공시 SecurityAlert 반환
SecurityAlert SecurityAlertManager::getAlertById(int id)
{
    SecurityAlert a;
    QSqlQuery q;
    q.prepare(
        "SELECT id, occurredAt, location, type, videoPath, isConfirmed "
        "FROM security_alert WHERE id = ?"
    );
    q.addBindValue(id);
    if (q.exec() && q.next()) {
        a.id = q.value(0).toInt();
        a.occurredAt = QDateTime::fromString(q.value(1).toString(), Qt::ISODate);
        a.location = q.value(2).toString();
        a.type = q.value(3).toString();
        a.videoPath = q.value(4).toString();
        a.isConfirmed = q.value(5).toBool();
    }
    return a;
}

// 전체 알림 목록 조회 정의 - 최신순 반환
QList<SecurityAlert> SecurityAlertManager::getAlerts()
{
    QList<SecurityAlert> list;
    QSqlQuery q(
        "SELECT id, occurredAt, location, type, videoPath, isConfirmed "
        "FROM security_alert ORDER BY id DESC"
    );
    while (q.next()) {
        SecurityAlert a;
        a.id = q.value(0).toInt();
        a.occurredAt = QDateTime::fromString(q.value(1).toString(), Qt::ISODate);
        a.location = q.value(2).toString();
        a.type = q.value(3).toString();
        a.videoPath = q.value(4).toString();
        a.isConfirmed = q.value(5).toBool();
        list.append(a);
    }
    return list;
}
