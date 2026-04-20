#ifndef CAR_H
#define CAR_H

/**
 * @file    car.h
 * @brief   차량 데이터 모델
 * @note    UI/DB 공통 사용. 양측 합의 없이 변경 금지.
 */

#include <QString>

struct Car {
    int     id = 0;
    int     residentId = 0;
    QString carNumber;
    bool    isParked = false;      ///< 현재 주차 여부
    QString parkingSpot;           ///< 주차 자리 ID (예: "1-03"), 미주차 시 빈 문자열
};

#endif // CAR_H
