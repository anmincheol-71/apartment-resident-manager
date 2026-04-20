#ifndef RESIDENT_H
#define RESIDENT_H

/**
 * @file    resident.h
 * @brief   입주민(세대) 데이터 모델
 * @note    UI/DB 공통 사용. 양측 합의 없이 변경 금지.
 */

#include <QString>
#include <QDate>

/**
 * @struct  Resident
 * @brief   입주민(세대) 1건을 표현하는 데이터 구조체.
 *
 *  - id는 DB가 AUTOINCREMENT로 자동 부여. 신규 등록 시 0으로 넘긴다.
 *  - (dong, ho) 조합은 UNIQUE 제약.
 */
struct Resident {
    int     id = 0;        ///< DB PK (신규 등록 시 0)
    int     dong = 0;      ///< 동 번호 (예: 102)
    int     ho = 0;        ///< 호수   (예: 1101)
    QString name;          ///< 세대주 이름
    QString phone;         ///< 연락처 (예: "010-1234-5678")
    QDate   moveInDate;    ///< 입주일
    QString memo;          ///< 메모 (선택 입력)
};

#endif // RESIDENT_H