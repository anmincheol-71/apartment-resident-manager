#ifndef RESIDENT_H
#define RESIDENT_H

#include <QString>
#include <QDate>

struct Resident {
    int id = 0;         // 입주민 구분 번호 (기본값 0)
    int dong = 0;       // 동 정보 (기본값 0)
    int ho = 0;         // 호 정보 (기본값 0)
    QString name;       // 입주자 이름
    QString phone;      // 입주자 전화번호
    QDate moveInDate;   // 입주날짜
    // string vs date 비교
    // 데이터 출력에는 string이 더 편함 (양식 정해서 넣으면 됨)
    // 데이터 비교/정렬시에는 date가 더 편함
    // 날짜 칸 누르면 오름차순 정렬하기로 했으니 date로
    QString memo;       // 특이사항 등등의 정보 기입
};



#endif // RESIDENT_H
