#ifndef RESIDENTPAGE_H
#define RESIDENTPAGE_H

#include <QWidget>
#include "01_residentmanager.h"

class QTableView;
class QLineEdit;
class QComboBox;

class ResidentPage : public QWidget {
private:
    Q_OBJECT
public:
    // 생성자 residentmanager 주소를 받아서 manager를 통해 데이터 관리
    explicit ResidentPage(ResidentManager *mgr, QWidget *parent = nullptr);

private slots:
    // 등록
    void onAdd();
    // 수정
    void onEdit();
    // 삭제
    void onDelete();
    // 검색
    void onSearch();
    // 초기화
    void onClear();
    // 더블클릭 - 상세정보 조회
    void onRowDoubleClicked(const QModelIndex &index);

private:
    // ResidentManager
    ResidentManager *m_mgr;

    // 입주민 목록 출력할 Table 위젯
    QTableView *m_table;
    // 이름 전화번호 등 검색기준 고를 콤보박스
    QComboBox  *m_searchField;
    // 검색어 입력할 텍스트 칸
    QLineEdit  *m_searchEdit;

    // 현재 목록에서 어떤 입주민을 선택했는지 ID 번호
    int  selectedResidentId() const;
    // 입주민 정보 입력 및 수정등 사용할 팝업창
    void showResidentDialog(bool isEdit);
};

#endif // RESIDENTPAGE_H
