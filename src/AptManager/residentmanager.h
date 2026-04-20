#ifndef RESIDENTMANAGER_H
#define RESIDENTMANAGER_H

/**
 * @file    residentmanager.h
 * @brief   입주민 / 차량 CRUD 비즈니스 로직 인터페이스
 * @note    UI 담당은 이 헤더에 선언된 public 시그니처만 신뢰하고 호출한다.
 *          DB 담당은 이 시그니처를 변경하지 않는 범위 내에서 내부 구현을 작성한다.
 *          시그니처 변경이 꼭 필요할 경우 양측이 함께 결정한다.
 */

#include <QObject>
#include <QList>
#include <QString>
#include <QSqlTableModel>

#include "resident.h"
#include "car.h"

class ResidentManager : public QObject
{
    Q_OBJECT

public:
    explicit ResidentManager(QObject *parent = nullptr);
    ~ResidentManager();

    // -------------------------------------------------------------
    //  초기화
    // -------------------------------------------------------------

    /**
     * @brief  SQLite DB 연결을 열고, 필요하면 스키마를 생성한다.
     * @return 성공 시 true
     * @note   MainWindow 생성 직후 한 번만 호출된다.
     *         실패 시 UI 는 QMessageBox 로 치명적 오류를 표시하고 종료한다.
     */
    bool initialize();

    // -------------------------------------------------------------
    //  Model / View 바인딩용
    // -------------------------------------------------------------

    /**
     * @brief  QTableView 에 setModel() 로 붙일 입주민 테이블 모델.
     * @return 내부 소유 QSqlTableModel 포인터 (delete 금지)
     */
    QSqlTableModel* residentModel();

    // -------------------------------------------------------------
    //  조회
    // -------------------------------------------------------------

    /** @brief 특정 id 세대의 전체 정보를 반환한다. 없으면 id==0 인 기본값. */
    Resident getResidentById(int id);

    /** @brief 특정 세대가 소유한 모든 차량 목록을 반환한다. */
    QList<Car> getCarsByResidentId(int residentId);

    // -------------------------------------------------------------
    //  등록 / 수정 / 삭제 (CRUD)
    // -------------------------------------------------------------

    /**
     * @brief  새 세대를 등록한다. 차량이 있으면 함께 등록한다.
     * @param  r            등록할 세대 정보 (id 는 무시되고 DB가 자동 부여)
     * @param  carNumbers   함께 등록할 차량 번호 목록. 비어 있어도 OK.
     * @return 성공 시 true. (중복 dong/ho 등 제약 위반 시 false)
     */
    bool addResident(const Resident &r, const QStringList &carNumbers);

    /**
     * @brief  기존 세대 정보를 갱신하고, 차량 목록도 교체한다.
     * @param  r            수정할 세대 정보 (id 가 반드시 유효해야 함)
     * @param  carNumbers   새 차량 번호 목록 (기존 차량은 이 목록으로 교체됨)
     * @return 성공 시 true
     */
    bool updateResident(const Resident &r, const QStringList &carNumbers);

    /**
     * @brief  세대를 삭제한다. FK CASCADE 로 연결된 차량도 함께 삭제된다.
     * @param  id   삭제 대상 세대의 id
     */
    bool removeResident(int id);

    // -------------------------------------------------------------
    //  검색 / 필터
    // -------------------------------------------------------------

    /**
     * @brief  residentModel 에 필터를 걸어 검색 결과만 노출한다.
     * @param  field   검색 기준 컬럼명 ("name", "dong", "ho", "phone")
     * @param  keyword 검색어. 부분 일치(LIKE '%keyword%') 로 처리된다.
     * @note   빈 keyword 는 clearFilter() 와 동일하게 동작한다.
     */
    void setSearchFilter(const QString &field, const QString &keyword);

    /** @brief 필터를 해제하고 전체 목록을 다시 보여준다. */
    void clearFilter();

signals:
    /**
     * @brief  데이터가 바뀌었을 때 방출되는 시그널.
     *         UI 는 이것을 받아 상태바의 "총 N세대" 같은 표시를 갱신한다.
     */
    void dataChanged();

private:
    QSqlTableModel *m_residentModel = nullptr;

    void populateDummyData();
};

#endif // RESIDENTMANAGER_H
