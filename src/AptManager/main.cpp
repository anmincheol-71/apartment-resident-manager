#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

/*

     Raw String Literal (C++ 문법)
     R"( ... )" 내부 문자열을 그대로 사용
     줄바꿈, 따옴표 등을 escape(\", \n) 없이 사용

    * { } - 모든 위젯
    QMainWindow { } - 특정 위젯 타입
    QWidget#objectName { } - 특정 이름(objectName)의 위젯
    QPushButton#addBtn { } - 특정 이름의 버튼
    QPushButton:hover { } - 마우스 올렸을 때
    QPushButton:pressed { } - 클릭했을 때

    #F1F5F9 - 색상 코드 (HEX)
     RRGGBB (빨 초 파) 16진수로 표현
*/

    a.setStyleSheet(R"(

/*  *{ } 전체 공통 */
/* 모든 위젯에 폰트: 맑은 고딕, 13px 적용 */
* {
    font-family: "Malgun Gothic", sans-serif;
    font-size: 13px;
}

/* 메인 윈도우 */
/* 앱 전체 배경: 연한 회색 */
QMainWindow { background: #F1F5F9; }
QWidget#centralwidget { background: #F1F5F9; }

/* 네비게이션 바 */
/* 배경: 거의 검정 */
QWidget#navWidget {
    background: #0F172A;
}
/* 네비 바 안의 타이틀 라벨: 흰색, 굵게 */
QWidget#navWidget QLabel {
    background: transparent;
    border: none;
    color: #F8FAFC;
    font-size: 15px;
    font-weight: bold;
}

/* 툴바 (입주민 페이지) */
/* 검색/버튼 영역: 흰 배경, 하단에 연한 회색 테두리 */
QWidget#toolbarWidget {
    background: #FFFFFF;
    border-bottom: 1px solid #E2E8F0;
}

/* 테이블 컨테이너 */
/* 테이블을 감싸는 영역: 연한 회색 배경 */
QWidget#tableContainer { background: #F1F5F9; }

/*버튼 공통*/
/* 모든 버튼 기본: 흰 배경, 회색 테두리, 둥근 모서리 */
QPushButton {
    background: #FFFFFF;
    border: 1px solid #CBD5E1;
    border-radius: 6px;
    padding: 6px 16px;
    color: #374151;
}
/* 마우스 올렸을 때: 배경 살짝 어둡게, 테두리 진하게 */
QPushButton:hover  { background: #F8FAFC; border-color: #94A3B8; }
/* 클릭했을 때: 배경 더 어둡게 */
QPushButton:pressed { background: #F1F5F9; }

/*등록 버튼 */
/* 파란 배경, 흰 글자, 굵게 */
QPushButton#addBtn {
    background: #2563EB;
    color: #FFFFFF;
    border: none;
    font-weight: bold;
}
/* 마우스 올리면 진한 파랑 */
QPushButton#addBtn:hover  { background: #1D4ED8; }
/* 클릭 시 더 진한 파랑 */
QPushButton#addBtn:pressed { background: #1E40AF; }

/*수정 버튼*/
/* 파란 글자, 연한 파란 테두리 */
QPushButton#editBtn { color: #2563EB; border-color: #BFDBFE; }
/* 마우스 올리면 아주 연한 파란 배경 */
QPushButton#editBtn:hover { background: #EFF6FF; }

/*삭제 버튼 */
/* 빨간 글자, 연한 빨간 테두리 */
QPushButton#deleteBtn { color: #DC2626; border-color: #FECACA; }
/* 마우스 올리면 아주 연한 빨간 배경 */
QPushButton#deleteBtn:hover { background: #FEF2F2; }

/*검색 버튼*/
/* 파란 배경, 흰 글자 */
QPushButton#searchBtn {
    background: #2563EB;
    color: #FFFFFF;
    border: none;
}
/* 마우스 올리면 진한 파랑 */
QPushButton#searchBtn:hover { background: #1D4ED8; }

/*전체보기 버튼*/
/* 회색 글자 */
QPushButton#clearBtn { color: #64748B; }

/*텍스트 입력창*/
/* 흰 배경, 회색 테두리, 둥근 모서리 */
QLineEdit {
    border: 1px solid #CBD5E1;
    border-radius: 6px;
    padding: 6px 10px;
    background: #FFFFFF;
    color: #1E293B;
    selection-background-color: #DBEAFE; /* 텍스트 선택 시 연한 파랑 */
}
/* 마우스 올리면 테두리 진하게 */
QLineEdit:hover  { border-color: #94A3B8; }
/* 포커스(클릭) 시 파란 테두리 2px */
QLineEdit:focus  { border: 2px solid #2563EB; }

/* 드롭다운 (검색 조건 선택)*/
QComboBox {
    border: 1px solid #CBD5E1;
    border-radius: 6px;
    padding: 6px 10px;
    background: #FFFFFF;
    color: #1E293B;
}
QComboBox:hover { border-color: #94A3B8; }
QComboBox:focus { border-color: #2563EB; }
/* 드롭다운 펼쳤을 때 목록 스타일 */
QComboBox QAbstractItemView {
    border: 1px solid #E2E8F0;
    background: #FFFFFF;
    selection-background-color: #DBEAFE; /* 선택 항목: 연한 파랑 */
    selection-color: #1E293B;
    outline: none;
    padding: 4px;
}

/*숫자 입력(동/호), 날짜 입력(입주일)*/
QSpinBox, QDateEdit {
    border: 1px solid #CBD5E1;
    border-radius: 6px;
    padding: 6px 8px;
    background: #FFFFFF;
    color: #1E293B;
}
/* 포커스 시 파란 테두리 */
QSpinBox:focus, QDateEdit:focus { border: 2px solid #2563EB; }

/*거주자 목록 테이블*/
/* 흰 배경, 연한 회색 테두리, 둥근 모서리 */
QTableView {
    background: #FFFFFF;
    border: 1px solid #E2E8F0;
    border-radius: 8px;
    gridline-color: #F1F5F9;             /* 행 구분선: 연한 회색 */
    selection-background-color: #DBEAFE; /* 선택된 행: 연한 파랑 */
    selection-color: #1E293B;
    alternate-background-color: #F8FAFC; /* 홀짝 행 번갈아 배경색 */
}
/* 각 셀 패딩 */
QTableView::item {
    padding: 6px 12px;
    border: none;
}
/* 선택된 셀 */
QTableView::item:selected {
    background: #DBEAFE;
    color: #1E293B;
}
/* 컬럼 헤더 (동/호/이름/연락처/입주일/메모) */
QHeaderView::section {
    background: #F8FAFC;
    color: #64748B;
    font-size: 12px;
    font-weight: bold;
    border: none;
    border-bottom: 2px solid #E2E8F0;   /* 헤더 하단 테두리 */
    border-right: 1px solid #E2E8F0;    /* 컬럼 구분선 */
    padding: 8px 12px;
    letter-spacing: 1px;                /* 글자 간격 */
}
/* 마지막 컬럼은 오른쪽 테두리 없음 */
QHeaderView::section:last-child { border-right: none; }

/*하단 상태바*/
/* 흰 배경, 상단 테두리, 회색 글자 */
QStatusBar {
    background: #FFFFFF;
    border-top: 1px solid #E2E8F0;
    color: #94A3B8;
    font-size: 12px;
    padding: 2px 8px;
}

/*팝업 다이얼로그 (등록/수정/상세보기)*/
QDialog { background: #FFFFFF; }

/* 다이얼로그 안의 그룹박스 (세대정보/차량정보) */
QGroupBox {
    border: 1px solid #E2E8F0;
    border-radius: 8px;
    margin-top: 14px;
    padding-top: 6px;
    font-weight: bold;
    color: #374151;
}
/* 그룹박스 제목 위치 조정 */
QGroupBox::title {
    subcontrol-origin: margin;
    left: 12px;
    padding: 0 6px;
    background: #FFFFFF;
    color: #374151;
}

/* 일반 텍스트 라벨: 진한 회색 */
QLabel { color: #374151; }

/* 차량 목록 리스트 */
QListWidget {
    border: 1px solid #E2E8F0;
    border-radius: 6px;
    background: #FFFFFF;
    outline: none;
}
QListWidget::item {
    padding: 6px 10px;
    border-radius: 4px;
    color: #374151;
}
/* 마우스 올리면 연한 회색 배경 */
QListWidget::item:hover { background: #F8FAFC; }

/* 다이얼로그 확인/취소 버튼 최소 너비 */
QDialogButtonBox QPushButton { min-width: 72px; }

/*스크롤바*/
/* 세로 스크롤바: 얇고 투명하게 */
QScrollBar:vertical {
    background: transparent;
    width: 8px;
    margin: 0;
}
/* 스크롤 핸들: 연한 회색, 둥글게 */
QScrollBar::handle:vertical {
    background: #CBD5E1;
    border-radius: 4px;
    min-height: 30px;
}
/* 핸들 hover 시 진한 회색 */
QScrollBar::handle:vertical:hover { background: #94A3B8; }
/* 위아래 화살표 버튼 숨김 */
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }

/* 가로 스크롤바 */
QScrollBar:horizontal {
    background: transparent;
    height: 8px;
}
QScrollBar::handle:horizontal {
    background: #CBD5E1;
    border-radius: 4px;
}
/* 좌우 화살표 버튼 숨김 */
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
)");

    MainWindow w;
    w.show();
    return QCoreApplication::exec();
}
