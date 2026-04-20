#ifndef CAR_H
#define CAR_H

/**
 * @file    car.h
 * @brief   차량 데이터 모델
 * @note    UI/DB 공통 사용. 양측 합의 없이 변경 금지.
 *          Resident와 1:N 관계. 세대 삭제 시 CASCADE로 함께 삭제됨.
 */

#include <QString>

/**
 * @struct  Car
 * @brief   차량 1대를 표현하는 데이터 구조체.
 *
 *  - residentId는 소속 세대의 Resident::id를 가리키는 외래키.
 *  - 세대 삭제 시 ON DELETE CASCADE로 자동 삭제됨.
 */
struct Car {
    int     id = 0;          ///< 차량 PK (AUTOINCREMENT)
    int     residentId = 0;  ///< 소속 세대 id (외래키)
    QString carNumber;       ///< 차량 번호 (예: "12가1234")
};

#endif // CAR_H