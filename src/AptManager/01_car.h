#ifndef CAR_H
#define CAR_H

#include <QString>

struct Car {
    int id = 0;         // 차량 구분 번호 (기본값 0)
    int residentId = 0; // 차량 소유주 -> 입주민 구분번호 (기본값 0)
    QString carNumber;  // 차량 번호 (ex 12가1234 형식이라 string)
};

#endif // CAR_H
