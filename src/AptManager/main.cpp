#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setStyleSheet(R"(
* {
    font-family: "Malgun Gothic", sans-serif;
    font-size: 13px;
}

/* ── 메인 윈도우 ── */
QMainWindow { background: #F1F5F9; }
QWidget#centralwidget { background: #F1F5F9; }

/* ── 네비게이션 바 ── */
QWidget#navWidget {
    background: #0F172A;
}
QWidget#navWidget QLabel {
    background: transparent;
    border: none;
    color: #F8FAFC;
    font-size: 15px;
    font-weight: bold;
}

/* ── 툴바 (입주민 페이지) ── */
QWidget#toolbarWidget {
    background: #FFFFFF;
    border-bottom: 1px solid #E2E8F0;
}

/* ── 테이블 컨테이너 ── */
QWidget#tableContainer { background: #F1F5F9; }

/* ── 버튼 공통 ── */
QPushButton {
    background: #FFFFFF;
    border: 1px solid #CBD5E1;
    border-radius: 6px;
    padding: 6px 16px;
    color: #374151;
}
QPushButton:hover  { background: #F8FAFC; border-color: #94A3B8; }
QPushButton:pressed { background: #F1F5F9; }

/* 등록 */
QPushButton#addBtn {
    background: #2563EB;
    color: #FFFFFF;
    border: none;
    font-weight: bold;
}
QPushButton#addBtn:hover  { background: #1D4ED8; }
QPushButton#addBtn:pressed { background: #1E40AF; }

/* 수정 */
QPushButton#editBtn { color: #2563EB; border-color: #BFDBFE; }
QPushButton#editBtn:hover { background: #EFF6FF; }

/* 삭제 */
QPushButton#deleteBtn { color: #DC2626; border-color: #FECACA; }
QPushButton#deleteBtn:hover { background: #FEF2F2; }

/* 검색 */
QPushButton#searchBtn {
    background: #2563EB;
    color: #FFFFFF;
    border: none;
}
QPushButton#searchBtn:hover { background: #1D4ED8; }

/* 전체보기 */
QPushButton#clearBtn { color: #64748B; }

/* ── 입력 필드 ── */
QLineEdit {
    border: 1px solid #CBD5E1;
    border-radius: 6px;
    padding: 6px 10px;
    background: #FFFFFF;
    color: #1E293B;
    selection-background-color: #DBEAFE;
}
QLineEdit:hover  { border-color: #94A3B8; }
QLineEdit:focus  { border: 2px solid #2563EB; }

QComboBox {
    border: 1px solid #CBD5E1;
    border-radius: 6px;
    padding: 6px 10px;
    background: #FFFFFF;
    color: #1E293B;
}
QComboBox:hover { border-color: #94A3B8; }
QComboBox:focus { border-color: #2563EB; }
QComboBox QAbstractItemView {
    border: 1px solid #E2E8F0;
    background: #FFFFFF;
    selection-background-color: #DBEAFE;
    selection-color: #1E293B;
    outline: none;
    padding: 4px;
}

QSpinBox, QDateEdit {
    border: 1px solid #CBD5E1;
    border-radius: 6px;
    padding: 6px 8px;
    background: #FFFFFF;
    color: #1E293B;
}
QSpinBox:focus, QDateEdit:focus { border: 2px solid #2563EB; }

/* ── 테이블 ── */
QTableView {
    background: #FFFFFF;
    border: 1px solid #E2E8F0;
    border-radius: 8px;
    gridline-color: #F1F5F9;
    selection-background-color: #DBEAFE;
    selection-color: #1E293B;
    alternate-background-color: #F8FAFC;
}
QTableView::item {
    padding: 6px 12px;
    border: none;
}
QTableView::item:selected {
    background: #DBEAFE;
    color: #1E293B;
}
QHeaderView::section {
    background: #F8FAFC;
    color: #64748B;
    font-size: 12px;
    font-weight: bold;
    border: none;
    border-bottom: 2px solid #E2E8F0;
    border-right: 1px solid #E2E8F0;
    padding: 8px 12px;
    letter-spacing: 1px;
}
QHeaderView::section:last-child { border-right: none; }

/* ── 상태바 ── */
QStatusBar {
    background: #FFFFFF;
    border-top: 1px solid #E2E8F0;
    color: #94A3B8;
    font-size: 12px;
    padding: 2px 8px;
}

/* ── 다이얼로그 ── */
QDialog { background: #FFFFFF; }

QGroupBox {
    border: 1px solid #E2E8F0;
    border-radius: 8px;
    margin-top: 14px;
    padding-top: 6px;
    font-weight: bold;
    color: #374151;
}
QGroupBox::title {
    subcontrol-origin: margin;
    left: 12px;
    padding: 0 6px;
    background: #FFFFFF;
    color: #374151;
}

QLabel { color: #374151; }

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
QListWidget::item:hover { background: #F8FAFC; }

QDialogButtonBox QPushButton { min-width: 72px; }

/* ── 스크롤바 ── */
QScrollBar:vertical {
    background: transparent;
    width: 8px;
    margin: 0;
}
QScrollBar::handle:vertical {
    background: #CBD5E1;
    border-radius: 4px;
    min-height: 30px;
}
QScrollBar::handle:vertical:hover { background: #94A3B8; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }

QScrollBar:horizontal {
    background: transparent;
    height: 8px;
}
QScrollBar::handle:horizontal {
    background: #CBD5E1;
    border-radius: 4px;
}
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
)");

    MainWindow w;
    w.show();
    return QCoreApplication::exec();
}
