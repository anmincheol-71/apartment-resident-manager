#include "residentmanager.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QDebug>

ResidentManager::ResidentManager(QObject *parent)
    : QObject(parent)
{}

ResidentManager::~ResidentManager() {}

bool ResidentManager::initialize()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QDir::currentPath() + "/aptmanager.db");

    if (!db.open()) {
        qWarning() << "DB 연결 실패:" << db.lastError().text();
        return false;
    }

    QSqlQuery q;
    q.exec("PRAGMA foreign_keys = ON");

    if (!q.exec(
        "CREATE TABLE IF NOT EXISTS resident ("
        "  id         INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  dong       INTEGER NOT NULL,"
        "  ho         INTEGER NOT NULL,"
        "  name       TEXT    NOT NULL,"
        "  phone      TEXT,"
        "  moveInDate TEXT,"
        "  memo       TEXT,"
        "  UNIQUE(dong, ho)"
        ")"))
    {
        qWarning() << "resident 테이블 생성 실패:" << q.lastError().text();
        return false;
    }

    if (!q.exec(
        "CREATE TABLE IF NOT EXISTS car ("
        "  id         INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  residentId INTEGER NOT NULL,"
        "  carNumber  TEXT    NOT NULL,"
        "  isParked   INTEGER DEFAULT 0,"
        "  parkingSpot TEXT   DEFAULT '',"
        "  FOREIGN KEY(residentId) REFERENCES resident(id) ON DELETE CASCADE"
        ")"))
    {
        qWarning() << "car 테이블 생성 실패:" << q.lastError().text();
        return false;
    }

    // 기존 DB에 컬럼이 없을 경우 마이그레이션
    migrateSchema();

    m_residentModel = new QSqlTableModel(this);
    m_residentModel->setTable("resident");
    m_residentModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_residentModel->setHeaderData(0, Qt::Horizontal, "ID");
    m_residentModel->setHeaderData(1, Qt::Horizontal, "동");
    m_residentModel->setHeaderData(2, Qt::Horizontal, "호");
    m_residentModel->setHeaderData(3, Qt::Horizontal, "이름");
    m_residentModel->setHeaderData(4, Qt::Horizontal, "연락처");
    m_residentModel->setHeaderData(5, Qt::Horizontal, "입주일");
    m_residentModel->setHeaderData(6, Qt::Horizontal, "메모");
    m_residentModel->setSort(1, Qt::AscendingOrder);

    QSqlQuery cnt;
    cnt.exec("SELECT COUNT(*) FROM resident");
    if (cnt.next() && cnt.value(0).toInt() == 0)
        populateDummyData();

    m_residentModel->select();
    return true;
}

void ResidentManager::migrateSchema()
{
    // car 테이블에 주차 관련 컬럼이 없으면 추가
    QSqlQuery info;
    info.exec("PRAGMA table_info(car)");
    QStringList columns;
    while (info.next())
        columns << info.value(1).toString();

    QSqlQuery q;
    if (!columns.contains("isParked"))
        q.exec("ALTER TABLE car ADD COLUMN isParked INTEGER DEFAULT 0");
    if (!columns.contains("parkingSpot"))
        q.exec("ALTER TABLE car ADD COLUMN parkingSpot TEXT DEFAULT ''");
}

QSqlTableModel* ResidentManager::residentModel()
{
    return m_residentModel;
}

Resident ResidentManager::getResidentById(int id)
{
    Resident r;
    QSqlQuery q;
    q.prepare("SELECT id, dong, ho, name, phone, moveInDate, memo FROM resident WHERE id = ?");
    q.addBindValue(id);
    if (q.exec() && q.next()) {
        r.id         = q.value(0).toInt();
        r.dong       = q.value(1).toInt();
        r.ho         = q.value(2).toInt();
        r.name       = q.value(3).toString();
        r.phone      = q.value(4).toString();
        r.moveInDate = QDate::fromString(q.value(5).toString(), Qt::ISODate);
        r.memo       = q.value(6).toString();
    }
    return r;
}

static Car rowToCar(const QSqlQuery &q)
{
    Car c;
    c.id          = q.value(0).toInt();
    c.residentId  = q.value(1).toInt();
    c.carNumber   = q.value(2).toString();
    c.isParked    = q.value(3).toBool();
    c.parkingSpot = q.value(4).toString();
    return c;
}

QList<Car> ResidentManager::getCarsByResidentId(int residentId)
{
    QList<Car> cars;
    QSqlQuery q;
    q.prepare(
        "SELECT id, residentId, carNumber, isParked, parkingSpot "
        "FROM car WHERE residentId = ?"
    );
    q.addBindValue(residentId);
    if (q.exec())
        while (q.next())
            cars.append(rowToCar(q));
    return cars;
}

QList<Car> ResidentManager::getAllCars()
{
    QList<Car> cars;
    QSqlQuery q(
        "SELECT id, residentId, carNumber, isParked, parkingSpot FROM car"
    );
    while (q.next())
        cars.append(rowToCar(q));
    return cars;
}

QList<Car> ResidentManager::getUnparkedCars()
{
    QList<Car> cars;
    QSqlQuery q(
        "SELECT id, residentId, carNumber, isParked, parkingSpot "
        "FROM car WHERE isParked = 0"
    );
    while (q.next())
        cars.append(rowToCar(q));
    return cars;
}

bool ResidentManager::parkCar(int carId, const QString &spotId)
{
    // 해당 자리가 이미 점유됐는지 확인
    QSqlQuery chk;
    chk.prepare("SELECT id FROM car WHERE parkingSpot = ? AND isParked = 1");
    chk.addBindValue(spotId);
    chk.exec();
    if (chk.next()) return false;

    QSqlQuery q;
    q.prepare("UPDATE car SET isParked = 1, parkingSpot = ? WHERE id = ?");
    q.addBindValue(spotId);
    q.addBindValue(carId);
    return q.exec();
}

bool ResidentManager::unparkCar(int carId)
{
    QSqlQuery q;
    q.prepare("UPDATE car SET isParked = 0, parkingSpot = '' WHERE id = ?");
    q.addBindValue(carId);
    return q.exec();
}

bool ResidentManager::addResident(const Resident &r, const QStringList &carNumbers)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) return false;

    QSqlQuery q;
    q.prepare(
        "INSERT INTO resident (dong, ho, name, phone, moveInDate, memo) "
        "VALUES (?, ?, ?, ?, ?, ?)"
    );
    q.addBindValue(r.dong);
    q.addBindValue(r.ho);
    q.addBindValue(r.name);
    q.addBindValue(r.phone);
    q.addBindValue(r.moveInDate.toString(Qt::ISODate));
    q.addBindValue(r.memo);

    if (!q.exec()) {
        qWarning() << "등록 실패:" << q.lastError().text();
        db.rollback();
        return false;
    }

    int newId = q.lastInsertId().toInt();

    for (const QString &num : carNumbers) {
        QString trimmed = num.trimmed();
        if (trimmed.isEmpty()) continue;
        QSqlQuery cq;
        cq.prepare("INSERT INTO car (residentId, carNumber) VALUES (?, ?)");
        cq.addBindValue(newId);
        cq.addBindValue(trimmed);
        if (!cq.exec()) {
            db.rollback();
            return false;
        }
    }

    db.commit();
    m_residentModel->select();
    emit dataChanged();
    return true;
}

bool ResidentManager::updateResident(const Resident &r, const QStringList &carNumbers)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) return false;

    QSqlQuery q;
    q.prepare(
        "UPDATE resident SET dong=?, ho=?, name=?, phone=?, moveInDate=?, memo=? WHERE id=?"
    );
    q.addBindValue(r.dong);
    q.addBindValue(r.ho);
    q.addBindValue(r.name);
    q.addBindValue(r.phone);
    q.addBindValue(r.moveInDate.toString(Qt::ISODate));
    q.addBindValue(r.memo);
    q.addBindValue(r.id);

    if (!q.exec()) {
        db.rollback();
        return false;
    }

    // 차량 교체 시 기존 주차 상태 보존: 번호가 같으면 유지
    QList<Car> oldCars = getCarsByResidentId(r.id);

    QSqlQuery dq;
    dq.prepare("DELETE FROM car WHERE residentId = ?");
    dq.addBindValue(r.id);
    if (!dq.exec()) { db.rollback(); return false; }

    for (const QString &num : carNumbers) {
        QString trimmed = num.trimmed();
        if (trimmed.isEmpty()) continue;

        // 같은 번호의 기존 차량 주차 상태 복원
        bool prevParked = false;
        QString prevSpot;
        for (const Car &old : oldCars) {
            if (old.carNumber == trimmed) {
                prevParked = old.isParked;
                prevSpot   = old.parkingSpot;
                break;
            }
        }

        QSqlQuery cq;
        cq.prepare(
            "INSERT INTO car (residentId, carNumber, isParked, parkingSpot) VALUES (?, ?, ?, ?)"
        );
        cq.addBindValue(r.id);
        cq.addBindValue(trimmed);
        cq.addBindValue(prevParked ? 1 : 0);
        cq.addBindValue(prevSpot);
        if (!cq.exec()) { db.rollback(); return false; }
    }

    db.commit();
    m_residentModel->select();
    emit dataChanged();
    return true;
}

bool ResidentManager::removeResident(int id)
{
    QSqlQuery fk;
    fk.exec("PRAGMA foreign_keys = ON");

    QSqlQuery q;
    q.prepare("DELETE FROM resident WHERE id = ?");
    q.addBindValue(id);

    if (!q.exec()) {
        qWarning() << "삭제 실패:" << q.lastError().text();
        return false;
    }

    m_residentModel->select();
    emit dataChanged();
    return true;
}

void ResidentManager::setSearchFilter(const QString &field, const QString &keyword)
{
    if (keyword.isEmpty()) { clearFilter(); return; }
    QString escaped = keyword;
    escaped.replace("'", "''");
    QString filter;
    if (field == "car_number")
        filter = QString("id IN (SELECT residentId FROM car WHERE carNumber LIKE '%%%1%%')").arg(escaped);
    else
        filter = field + " LIKE '%" + escaped + "%'";
    m_residentModel->setFilter(filter);
    m_residentModel->select();
}

void ResidentManager::clearFilter()
{
    m_residentModel->setFilter(QString());
    m_residentModel->select();
}

// ── 더미 데이터 ───────────────────────────────────────────────────────────────

void ResidentManager::populateDummyData()
{
    struct R { int dong, ho; const char *name, *phone, *date, *memo; };
    static const R res[] = {
        {101,  101, "김민준", "010-1234-5678", "2021-03-15", ""},
        {101,  201, "이서연", "010-2345-6789", "2020-07-22", "반려동물 있음"},
        {101,  301, "박지훈", "010-3456-7890", "2019-11-05", ""},
        {101,  401, "최수아", "010-4567-8901", "2022-01-10", ""},
        {101,  501, "정민서", "010-5678-9012", "2021-08-30", ""},
        {101,  601, "강도현", "010-6789-0123", "2020-04-18", "장기 입주"},
        {101,  701, "윤하은", "010-7890-1234", "2023-02-14", ""},
        {101,  801, "임재원", "010-8901-2345", "2018-06-25", ""},
        {101,  901, "한지수", "010-9012-3456", "2022-09-01", ""},
        {101, 1001, "오승현", "010-1111-2222", "2021-05-20", "주차 문의 多"},
        {101, 1101, "신예린", "010-2222-3333", "2023-07-11", ""},
        {101, 1201, "배현우", "010-3333-4444", "2019-03-08", ""},
        {102,  101, "조아름", "010-4444-5555", "2022-10-03", ""},
        {102,  202, "유민혁", "010-5555-6666", "2021-12-19", ""},
        {102,  303, "나경원", "010-6666-7777", "2020-02-28", ""},
        {102,  401, "황태양", "010-7777-8888", "2023-04-07", ""},
        {102,  502, "문서진", "010-8888-9999", "2022-06-15", ""},
        {102,  601, "손지아", "010-9999-0000", "2019-09-30", ""},
        {102,  703, "전민준", "010-1122-3344", "2021-01-22", ""},
        {102,  801, "류하늘", "010-2233-4455", "2020-11-13", ""},
        {102,  902, "김태희", "010-3344-5566", "2023-03-25", ""},
        {102, 1001, "이준혁", "010-4455-6677", "2022-08-09", ""},
        {102, 1102, "박서영", "010-5566-7788", "2018-12-01", ""},
        {102, 1201, "최민준", "010-6677-8899", "2021-07-04", ""},
        {103,  101, "정지원", "010-7788-9900", "2022-05-16", ""},
        {103,  201, "강수현", "010-8899-0011", "2020-10-27", ""},
        {103,  302, "윤민석", "010-9900-1122", "2023-01-08", ""},
        {103,  401, "임서아", "010-1010-2020", "2021-04-30", ""},
        {103,  503, "한도윤", "010-2020-3030", "2019-07-14", ""},
        {103,  601, "오지현", "010-3030-4040", "2022-11-02", ""},
        {103,  701, "신민서", "010-4040-5050", "2020-08-19", ""},
        {103,  802, "배수진", "010-5050-6060", "2023-06-23", ""},
        {103,  901, "조현우", "010-6060-7070", "2021-09-17", ""},
        {103, 1001, "유지수", "010-7070-8080", "2018-04-05", "장기 입주"},
        {103, 1103, "나민준", "010-8080-9090", "2022-03-12", ""},
        {103, 1201, "황서연", "010-9090-1010", "2020-06-08", ""},
        {104,  102, "문하은", "010-1212-3434", "2021-10-21", ""},
        {104,  204, "손재원", "010-2323-4545", "2023-05-30", ""},
        {104,  301, "전서진", "010-3434-5656", "2019-02-16", ""},
        {104,  403, "류민아", "010-4545-6767", "2022-07-09", ""},
        {104,  501, "김지호", "010-5656-7878", "2020-12-24", ""},
        {104,  602, "이수민", "010-6767-8989", "2021-06-03", "반려동물 있음"},
        {104,  701, "박현준", "010-7878-9090", "2023-08-18", ""},
        {104,  804, "최아린", "010-8989-1010", "2022-02-26", ""},
        {104,  903, "정도연", "010-9090-2121", "2019-05-11", ""},
        {104, 1001, "강민준", "010-1021-3243", "2020-09-07", ""},
        {104, 1102, "윤서현", "010-2132-4354", "2021-11-29", ""},
        {104, 1204, "임지원", "010-3243-5465", "2023-04-14", ""},
        {105,  101, "한수아", "010-4354-6576", "2022-12-05", ""},
        {105,  203, "오민준", "010-5465-7687", "2020-03-19", ""},
        {105,  301, "신하은", "010-6576-8798", "2021-02-08", ""},
        {105,  402, "배도현", "010-7687-9809", "2023-09-22", ""},
        {105,  504, "조서연", "010-8798-0910", "2019-10-31", ""},
        {105,  601, "유현준", "010-9809-1021", "2022-04-15", ""},
        {105,  703, "나지수", "010-1021-2132", "2020-07-06", ""},
        {105,  801, "황민서", "010-2132-3243", "2021-08-12", ""},
        {105,  902, "문재원", "010-3243-4354", "2023-10-01", ""},
        {105, 1004, "손예린", "010-4354-5465", "2018-11-18", "장기 입주"},
        {105, 1101, "전현우", "010-5465-6576", "2022-01-27", ""},
        {105, 1203, "류지아", "010-6576-7687", "2020-05-14", ""},
    };

    // {residentIndex, carNumber, isParked, parkingSpot}
    // spot ID 형식: "{층}-{행A/B/C}{열1-10}"  예) "1-A3", "2-C8"
    struct C { int idx; const char *num; int parked; const char *spot; };
    static const C cars[] = {
        {0,  "12가1234",    1, "1-A1"},
        {1,  "34나5678",    1, "1-A2"},
        {1,  "56다9012",    0, ""},
        {2,  "78라3456",    1, "1-A3"},
        {3,  "90마7890",    1, "1-A4"},
        {4,  "11바1234",    0, ""},
        {5,  "22사5678",    1, "1-A5"},
        {6,  "33아9012",    1, "1-B1"},
        {7,  "44자3456",    0, ""},
        {8,  "55차7890",    1, "2-A1"},
        {9,  "66카1234",    1, "2-A2"},
        {9,  "77타5678",    0, ""},
        {10, "88파9012",    1, "2-A3"},
        {11, "99하3456",    1, "2-A4"},
        {12, "경기11가1111", 0, ""},
        {13, "서울22나2222", 1, "2-A5"},
        {14, "인천33다3333", 1, "2-B1"},
        {15, "부산44라4444", 0, ""},
        {16, "대구55마5555", 1, "3-A1"},
        {17, "광주66바6666", 1, "3-A2"},
        {17, "울산77사7777", 0, ""},
        {18, "경기88아8888", 1, "3-A3"},
        {19, "서울99자9999", 0, ""},
        {20, "12나1357",    1, "3-A4"},
        {21, "34다2468",    1, "3-A5"},
        {22, "56라3579",    0, ""},
        {23, "78마4680",    1, "1-B2"},
        {23, "90바5791",    0, ""},
        {24, "11사6802",    1, "1-B3"},
        {25, "22아7913",    0, ""},
        {26, "33자8024",    1, "2-B2"},
        {27, "44차9135",    1, "2-B3"},
        {28, "55카0246",    0, ""},
        {29, "66타1357",    1, "3-B1"},
    };

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    QSqlQuery q;
    q.prepare(
        "INSERT INTO resident (dong, ho, name, phone, moveInDate, memo) "
        "VALUES (?, ?, ?, ?, ?, ?)"
    );

    QList<int> ids;
    for (const auto &r : res) {
        q.addBindValue(r.dong);
        q.addBindValue(r.ho);
        q.addBindValue(QString::fromUtf8(r.name));
        q.addBindValue(QString::fromUtf8(r.phone));
        q.addBindValue(QString::fromUtf8(r.date));
        q.addBindValue(QString::fromUtf8(r.memo));
        q.exec();
        ids.append(q.lastInsertId().toInt());
    }

    QSqlQuery cq;
    cq.prepare(
        "INSERT INTO car (residentId, carNumber, isParked, parkingSpot) VALUES (?, ?, ?, ?)"
    );
    for (const auto &c : cars) {
        if (c.idx < ids.size()) {
            cq.addBindValue(ids[c.idx]);
            cq.addBindValue(QString::fromUtf8(c.num));
            cq.addBindValue(c.parked);
            cq.addBindValue(QString::fromUtf8(c.spot));
            cq.exec();
        }
    }

    db.commit();
}
