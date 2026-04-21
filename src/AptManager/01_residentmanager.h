#ifndef RESIDENTMANAGER_H
#define RESIDENTMANAGER_H

#include <QObject>
#include <QList>
#include <QString>
// #include <QSqlTableModel>
#include <QtSql/QSqlTableModel>

#include "01_resident.h"
#include "01_car.h"

class ResidentManager : public QObject {
private:
    //signal이나 slot 등의 mos를 사용하려는 클래스에서 private 영역에 선언해야하는 매크로
    Q_OBJECT // mos -> meta-object system은 object 간의 통신 매커니즘을 위한 시스템 제공

public:
    // 생성자
    // explicit - 형변환 막는 키워드
    // 부모 객체를 지정해 메모리 관리를 떠넘김 (부모가 해제되면 자식도 자동 해제)
    explicit ResidentManager(QObject *parent = nullptr);
    // 소멸자 - 관리하는 동적 메모리나 리소스 없으니 비워두기
    ~ResidentManager();

    // sqlite DB 연결 - 성공하면 true
    bool initialize();

    //
    QSqlTableModel* residentModel();

    // 입주민 구분번호를 통해 id 세대의 정보 반환
    // 성공시 입주민 정보 반환 - Resident
    Resident getResidentById(int id);

    // 입주민 구분번호를 통해 id 세대의 차량 정보 반환
    // 성공시 차량정보를 리스트로 반환(여러대) - QList<Car>
    QList<Car> getCarsByResidentId(int residentId);

    // 입주민 등록 함수 - 성공시 true - bool
    // r            등록할 세대 정보 (id는 DB 자동부여)
    // carNumbers   등록할 차량 번호 목록
    bool addResident(const Resident &r, const QStringList &carNumbers);

    // 입주민 정보 수정 함수 - 성공시 true - bool
    // r            수정할 세대 정보 (id가 존재해야함 -> 바꿀 세대가 있어야함)
    //        -> 어차피 목록에서 선택해야(여기서 이미 id 존재) 삭제 가능하니 굳이기도함)
    // carNumbers   수정할 차량 번호 목록
    bool updateResident(const Resident &r, const QStringList &carNumbers);

    // 입주민 정보 삭제 함수 - 성공시 true - bool
    // id   삭제할 세대 정보 (수정과 마찬가지로 id는 존재해야함
    //        -> 어차피 목록에서 선택해야(여기서 이미 id 존재) 삭제 가능하니 굳이기도함)
    bool removeResident(int id);

    // 검색 함수 - 화면으로 출력하니 void
    // field    검색 기준 (이름, 전화번호, 차량번호 등등)
    // keyword  검색어
    void setSearchFilter(const QString &field, const QString &keyword);

    // 검색결과 없애고 전체 입주민 출력 - 화면으로 출력하니 void
    void clearFilter();

    // 전체 차량 목록 불러오기 - (여러대) - QList<Car>
    QList<Car> getAllCars();

    // 더미데이터 생성 함수 - 테스트용도
    void populateDummyData();

signals:
    // 데이터가 바뀌었을 때 방출되는 시그널
    // UI 는 이것을 받아 상태바의 "총 N세대" 같은 표시 업데이트함
    void dataChanged();

private:
    // 입주민 DB 매핑 함수
    QSqlTableModel *m_residentModel = nullptr;

    // 스키마 생성 및 수정 함수
    // void migrateSchema();

};

#endif // RESIDENTMANAGER_H