#ifndef NOTICEPAGE_H
#define NOTICEPAGE_H

#include <QWidget>

class NoticeManager;
class QComboBox;
class QLabel;
class QTableWidget;

// 공지사항 목록을 표시하고 등록/열람 기능을 제공하는 페이지
class NoticePage : public QWidget {
    Q_OBJECT

public:
    explicit NoticePage(NoticeManager *mgr, QWidget *parent = nullptr);

public slots:
    // 목록 새로고침 - 외부에서도 호출 가능
    void refresh();

private slots:
    // 공지 등록 버튼 클릭 시 NoticeDialog(작성 모드) 열기
    void onAddNotice();

    // 행 더블클릭 시 NoticeDialog(열람 모드) 열기
    void onRowDoubleClicked(int row, int col);

    // 유형 필터 변경 시 목록 갱신
    void onFilterChanged();

private:
    NoticeManager *m_mgr;
    QTableWidget *m_table;
    QComboBox *m_filterCombo;  // 유형 필터
    QLabel *m_countLabel;

    // 공지 유형에 따른 표시 색상 반환
    static QColor typeColor(const QString &type);
};

#endif // NOTICEPAGE_H
