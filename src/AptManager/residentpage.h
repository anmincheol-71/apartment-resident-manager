#ifndef RESIDENTPAGE_H
#define RESIDENTPAGE_H

#include <QWidget>
#include "residentmanager.h" // m_mgr->addResident() 호출
                             // 멤버변수 선언 ResidentManager *m_mgr;

class QTableView; // 전방선언, 실제 선언은 Qt 헤더에 존재
                  // 테이블 화면(행/ 열로 데이터 표시)
class QLineEdit;  // 한줄 텍스트 입력창
class QComboBox;  // 드롭다운 선택박스

class ResidentPage : public QWidget
{
    Q_OBJECT // 매크로
             // Qt의 시그널/슬롯 기능을 사용하기 위해 선언
             // QWidget을 상속 받는 클래스에 필요
public:
    explicit ResidentPage(ResidentManager *mgr, QWidget *parent = nullptr); // ResidentPage 생성자 (백엔드, 부모 위젯(MainWindow) 지정)
private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void onSearch();
    void onClear();
    void onRowDoubleClicked(const QModelIndex &index); // QModelIndex - 몇 번째 행을 클릭했는지에 대한 위치정보

private:
    ResidentManager *m_mgr;

    QTableView *m_table;       // 거주자 목록 테이블
    QComboBox  *m_searchField; // 검색 조건 드롭 아웃(이름/연락처/차량번호)
    QLineEdit  *m_searchEdit;  // 검색어 입력창

    int  selectedResidentId() const;      // 선택된 거주자 ID 가져오기
    void showResidentDialog(bool isEdit); // 등록/수정 팝업창
};                                        // false : 등록/ true : 수정

#endif // RESIDENTPAGE_H
