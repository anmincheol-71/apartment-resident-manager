#include "residentpage.h"

// Qt 레이아웃
#include <QVBoxLayout>
#include <QHBoxLayout>

// Qt 위젯
#include <QTableView>
#include <QHeaderView>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>

// Qt 다이얼로그 (팝업창)
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QSpinBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QGroupBox>

// 색상, 정렬
#include <QColor>
#include <QIdentityProxyModel>

// 검색 필드 목록 (콤보박스 순서와 동일)
static const QStringList kSearchFields = {"name", "phone", "car_number"};

// 가운데 정렬 프록시 클래스
// QSqlTableModel은 TextAlignmentRole을 지원하지 않기 때문에
// 프록시 모델로 감싸 모든 셀을 가운데 정렬
class CenterAlignProxy : public QIdentityProxyModel {
public:
    using QIdentityProxyModel::QIdentityProxyModel;
    QVariant data(const QModelIndex &index, int role) const override {
        //화면이 글자 정렬 방식을 물어보면, 가운데 정렬으로 대답
        if (role == Qt::TextAlignmentRole)
            return Qt::AlignCenter;
        // 실제 글자 내용은 원본으로 전달
        return QIdentityProxyModel::data(index, role);
    }
};


// 생성자 — 화면 레이아웃 구성
ResidentPage::ResidentPage(ResidentManager *mgr, QWidget *parent)
    : QWidget(parent),  // 부모 클래스(QWidget) 생성자 호출
    m_mgr(mgr)          // 멤버변수 m_mgr에 mgr 저장
{
    // 전체 세로 레이아웃 (상단 툴바 + 하단 테이블)
    auto *root = new QVBoxLayout(this);       // QVBoxLayout - 위젯을 세로로 쌓는 레이아웃
    root->setContentsMargins(0, 0, 0, 0);     // 레이아웃 안쪽 여백 설정 (왼쪽, 위, 오른쪽, 아래)
    root->setSpacing(0);                      // 레이아웃 안 위젯 간 간격 설정

    // 툴바 (검색 + CRUD 버튼)
    auto *toolbar = new QWidget(this);
    toolbar->setObjectName("toolbarWidget");  //setObjectName - main.cpp 스타일시트(QSS)에서 사용
    toolbar->setFixedHeight(56);

    auto *tbLayout = new QHBoxLayout(toolbar);    // QHBoxLayout - 툴바 안의 위젯을 가로로 배치하는 레이아웃
    tbLayout->setContentsMargins(16, 10, 16, 10); // 레이아웃 안쪽 여백 설정 (왼쪽, 위, 오른쪽, 아래)
    tbLayout->setSpacing(8);

    // 검색 조건 드롭다운 (이름 / 연락처 / 차량번호)
    m_searchField = new QComboBox(toolbar);                 // QComboBox - 여러 항목 중 하나를 선택하는 드롭다운 메뉴
    m_searchField->addItems({"이름", "연락처", "차량번호"});
    m_searchField->setFixedWidth(90);

    // 검색어 입력창
    m_searchEdit = new QLineEdit(toolbar);                  // QLineEdit - 짧은 글자를 입력하는 텍스트 박스
    m_searchEdit->setPlaceholderText("검색어를 입력하세요");  // 아무것도 안 썼을 때 보여주는 안내문구
    m_searchEdit->setMinimumWidth(200);

    // 검색 / 전체보기 버튼
    auto *searchBtn = new QPushButton("검색",    toolbar);
    auto *clearBtn  = new QPushButton("전체보기", toolbar);
    searchBtn->setObjectName("searchBtn");
    clearBtn->setObjectName("clearBtn");

    // 등록 / 수정 / 삭제 버튼
    auto *addBtn    = new QPushButton("+ 등록", toolbar);
    auto *editBtn   = new QPushButton("수정",   toolbar);
    auto *deleteBtn = new QPushButton("삭제",   toolbar);
    addBtn->setObjectName("addBtn");
    editBtn->setObjectName("editBtn");
    deleteBtn->setObjectName("deleteBtn");

    // 툴바 레이아웃: [검색조건][검색어][검색][전체보기] ─stretch─ [등록][수정][삭제]
    tbLayout->addWidget(m_searchField);
    tbLayout->addWidget(m_searchEdit);
    tbLayout->addWidget(searchBtn);
    tbLayout->addWidget(clearBtn);
    tbLayout->addStretch();              // 빈공간 만들기 - 검색 파트는 왼쪽, 관리 버튼은 오른쪽으로 밀어냄
    tbLayout->addWidget(addBtn);
    tbLayout->addWidget(editBtn);
    tbLayout->addWidget(deleteBtn);

    // 테이블 (데이터 목록 표시 영역)
    auto *tableContainer = new QWidget(this);
    tableContainer->setObjectName("tableContainer");

    auto *tcLayout = new QVBoxLayout(tableContainer);
    tcLayout->setContentsMargins(16, 16, 16, 16);

    m_table = new QTableView(tableContainer);

    // 가운데 정렬 프록시를 통해 모델 연결
    auto *proxy = new CenterAlignProxy(m_table);
    proxy->setSourceModel(m_mgr->residentModel());
    m_table->setModel(proxy);

    // 테이블 옵션 설정
    m_table->hideColumn(0);                                        // id 컬럼 숨김
    m_table->verticalHeader()->setVisible(false);                  // 맨 좌측 행 번호 숨김
    m_table->setShowGrid(true);                                    // 격자선 표시
    m_table->setAlternatingRowColors(true);                        // 홀짝 행 색상
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);  // 클릭 시 칸 단위가 아니라 '줄 전체' 선택
    m_table->setSelectionMode(QAbstractItemView::SingleSelection); // 단일 선택
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);   // 표를 더블클릭해서 글자를 바로 고치는 것 차단
    m_table->setSortingEnabled(true);                              // 헤더 클릭 정렬

    // 컬럼 너비 설정
    auto *hdr = m_table->horizontalHeader();
    hdr->setSectionResizeMode(QHeaderView::Interactive);  // 사용자가 직접 조절 가능
    m_table->setColumnWidth(1, 60);                       // 동
    m_table->setColumnWidth(2, 60);                       // 호
    m_table->setColumnWidth(3, 100);                      // 이름
    m_table->setColumnWidth(4, 130);                      // 연락처
    m_table->setColumnWidth(5, 100);                      // 입주일
    m_table->setColumnWidth(7, 150);                      // 차량번호
    hdr->setStretchLastSection(true);                     // 메모 컬럼이 남은 공간 차지

    // 차량번호(7번)를 메모(6번) 앞으로 이동
    // 결과 순서: 동 → 호 → 이름 → 연락처 → 입주일 → 차량번호 → 메모
    hdr->moveSection(7, 6);

    tcLayout->addWidget(m_table);

    // 전체 레이아웃 조립
    root->addWidget(toolbar);
    root->addWidget(tableContainer, 1);  // 1: 가중치
                                         // 화면 크기가 커지면 툴바 높이는 고정, 테이블 영역만 늘어남

    // 시그널/슬롯 연결
    connect(addBtn,       &QPushButton::clicked,        this, &ResidentPage::onAdd);
    connect(editBtn,      &QPushButton::clicked,        this, &ResidentPage::onEdit);
    connect(deleteBtn,    &QPushButton::clicked,        this, &ResidentPage::onDelete);
    connect(searchBtn,    &QPushButton::clicked,        this, &ResidentPage::onSearch);
    connect(clearBtn,     &QPushButton::clicked,        this, &ResidentPage::onClear);
    connect(m_searchEdit, &QLineEdit::returnPressed,    this, &ResidentPage::onSearch);           // 텍스트창에서 엔터 치면 검색
    connect(m_table,      &QTableView::doubleClicked,   this, &ResidentPage::onRowDoubleClicked); // 표 더블클릭 -> 상세창
}

// 선택된 데이터의 ID 찾기
int ResidentPage::selectedResidentId() const
{
    QModelIndex idx = m_table->currentIndex(); // currentIndex() - 선택한 칸(Cell)의 위치 정보를 가져오는 함수
    if (!idx.isValid()) return 0;              // 선택된 칸이 없으면 0을 돌려주고 종료

    // 화면용 주소(Proxy)를 실제 데이터용 주소(Source)로 변환
    //(정렬이나 필터링이 되어 있어도 정확한 데이터 위치를 찾기 위함)
    auto *proxy = qobject_cast<QIdentityProxyModel*>(m_table->model());
    QModelIndex src = proxy ? proxy->mapToSource(idx) : idx;

    // 실제 데이터 위치에서 0번 컬럼(id) 값 반환
    return m_mgr->residentModel()->data(
        m_mgr->residentModel()->index(src.row(), 0)
    ).toInt();
}

// 등록/수정 팝업 (isEdit: false=등록, true=수정)
void ResidentPage::showResidentDialog(bool isEdit)
{
    Resident r;                 // 세대 정보
    QList<Car> existingCars;    // 등록 차량 목록

    // 수정 모드 실행
    if (isEdit) {
        int id = selectedResidentId();                                          // 선택한 세대 찾기
        if (id == 0) {
            QMessageBox::information(this, "알림", "수정할 세대를 선택하세요.");   // 아무것도 안 골랐으면 에러 방지
            return;
        }
        r = m_mgr->getResidentById(id);
        existingCars = m_mgr->getCarsByResidentId(id);
    }

    // 정보 입력 및 수정 팝업창 생성
    QDialog dlg(this);
    dlg.setWindowTitle(isEdit ? "세대 수정" : "세대 등록");  // 조건문(? :) - 수정 모드면 "세대 수정", 아니면 "세대 등록"
    dlg.setMinimumWidth(360);

    // 입력 위젯 생성
    /*
     * QSpinBox - 동/호수를 숫자로 선택
     * QDateEdit - 달력으로 입주일 선택
     * QLineEdit - 이름, 연락처, 메모, 차량번호 입력
     */
    auto *dongSpin = new QSpinBox(&dlg);
    dongSpin->setRange(1, 999);
    dongSpin->setValue(isEdit ? r.dong : 101);

    auto *hoSpin = new QSpinBox(&dlg);
    hoSpin->setRange(1, 9999);
    hoSpin->setValue(isEdit ? r.ho : 101);

    auto *nameEdit  = new QLineEdit(isEdit ? r.name  : "", &dlg);
    auto *phoneEdit = new QLineEdit(isEdit ? r.phone : "", &dlg);
    phoneEdit->setPlaceholderText("010-0000-0000");

    auto *dateEdit = new QDateEdit(&dlg);
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("yyyy-MM-dd");
    dateEdit->setDate((isEdit && r.moveInDate.isValid()) ? r.moveInDate : QDate::currentDate());

    auto *memoEdit = new QLineEdit(isEdit ? r.memo : "", &dlg);

    // 차량 입력칸 생성 (최대 3대까지만 고정 배열로 처리)
    QLineEdit *carEdits[3];
    for (int i = 0; i < 3; ++i) {
        carEdits[i] = new QLineEdit(&dlg);
        carEdits[i]->setPlaceholderText("예: 12가1234");
        if (i < existingCars.size())
            carEdits[i]->setText(existingCars[i].carNumber);
    }

    // 폼 레이아웃에 위젯 배치 (QFormLayout - 왼쪽엔 제목, 오른쪽엔 입력칸 배치)
    auto *form = new QFormLayout;
    form->addRow("동:",    dongSpin);
    form->addRow("호:",    hoSpin);
    form->addRow("이름:",   nameEdit);
    form->addRow("연락처:", phoneEdit);
    form->addRow("입주일:", dateEdit);
    form->addRow("메모:",   memoEdit);
    form->addRow("차량 1:", carEdits[0]);
    form->addRow("차량 2:", carEdits[1]);
    form->addRow("차량 3:", carEdits[2]);

    // 확인 / 취소 버튼
    auto *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(btnBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    auto *vbox = new QVBoxLayout(&dlg);
    vbox->addLayout(form);
    vbox->addWidget(btnBox);

    // 팝업 띄우기 (exec). 취소(X 표시나 취소 버튼) 누르면 아래 코드는 무시하고 즉시 함수 종료
    if (dlg.exec() != QDialog::Accepted) return;

    // 이름 입력 검증 (trimmed() - 앞뒤 실수로 들어간 띄어쓰기 지워줌)
    if (nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "입력 오류", "이름을 입력하세요.");
        return;
    }

    // 입력값을 Resident 구조체에 담아 백엔드에 전달
    Resident res;
    res.id         = isEdit ? r.id : 0;
    res.dong       = dongSpin->value();
    res.ho         = hoSpin->value();
    res.name       = nameEdit->text().trimmed();
    res.phone      = phoneEdit->text().trimmed();
    res.moveInDate = dateEdit->date();
    res.memo       = memoEdit->text().trimmed();

    QStringList cars;
    for (int i = 0; i < 3; ++i) cars << carEdits[i]->text();

    // DB에 데이터 쏘기. 성공하면 ok는 true
    bool ok = isEdit ? m_mgr->updateResident(res, cars) : m_mgr->addResident(res, cars);
    if (!ok) {
        QMessageBox::warning(this, "오류",
                             isEdit ? "수정 실패\n(동/호 중복 여부 확인)" : "등록 실패\n(동/호 중복 여부 확인)");
    }
}

// 슬롯 — CRUD
void ResidentPage::onAdd()  { showResidentDialog(false); }  // 등록 팝업
void ResidentPage::onEdit() { showResidentDialog(true);  }  // 수정 팝업

void ResidentPage::onDelete() //삭제
{
    int id = selectedResidentId();
    if (id == 0) {
        QMessageBox::information(this, "알림", "삭제할 세대를 선택하세요.");
        return;
    }

    Resident r = m_mgr->getResidentById(id);

    // 삭제 확인 메시지
    // 삭제 확인 메시지 (arg() 함수 - 문자열의 %1, %2 자리에 변수를 순서대로 넣음)
    if (QMessageBox::question(this, "삭제 확인",
                              QString("%1동 %2호 (%3) 를 삭제하시겠습니까?\n등록 차량도 함께 삭제됩니다.")
                                  .arg(r.dong).arg(r.ho).arg(r.name))
        == QMessageBox::Yes)
        m_mgr->removeResident(id);
}

// 슬롯 — 검색
void ResidentPage::onSearch()
{
    int idx = m_searchField->currentIndex(); // 드롭다운에서 선택한 항목 순서 번호(0, 1, 2)
    if (idx >= 0 && idx < kSearchFields.size())
        m_mgr->setSearchFilter(kSearchFields[idx], m_searchEdit->text());
}

void ResidentPage::onClear()
{
    m_searchEdit->clear();   // 검색창 글자 지우기
    m_mgr->clearFilter();    // 전체 목록 표시
}

// 슬롯 — 행 더블클릭 -> 세대 상세정보 팝업
void ResidentPage::onRowDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    // 프록시 인덱스 -> 소스 인덱스 변환 후 거주자 ID 조회
    auto *proxy = qobject_cast<QIdentityProxyModel*>(m_table->model());
    QModelIndex src = proxy ? proxy->mapToSource(index) : index;
    int id = m_mgr->residentModel()->data(
                                       m_mgr->residentModel()->index(src.row(), 0)).toInt();
    if (id == 0) return;

    Resident   r    = m_mgr->getResidentById(id);
    QList<Car> cars = m_mgr->getCarsByResidentId(id);

    // 상세정보 다이얼로그 (보기 전용 팝업창)
    QDialog dlg(this);
    dlg.setWindowTitle(QString("%1동 %2호 — 세대 상세정보").arg(r.dong).arg(r.ho));
    dlg.setMinimumWidth(400);

    // 세대 정보 그룹 (QGroupBox - 테두리와 제목이 있는 상자)
    auto *resGroup = new QGroupBox("세대 정보", &dlg);
    auto *form = new QFormLayout(resGroup);
    form->setLabelAlignment(Qt::AlignRight);

    // 텍스트 복사 가능한 라벨 생성
    auto mkLbl = [](const QString &t) {
        auto *l = new QLabel(t);
        l->setTextInteractionFlags(Qt::TextSelectableByMouse); // 마우스 드래그/복사 허용
        return l;
    };

    // 데이터가 없으면 삼항 연산자를 써서 비워두지 않고 "-" 기호 출력
    form->addRow("동 / 호:",  mkLbl(QString("%1동  %2호").arg(r.dong).arg(r.ho)));
    form->addRow("이름:",     mkLbl(r.name));
    form->addRow("연락처:",   mkLbl(r.phone.isEmpty() ? "-" : r.phone));
    form->addRow("입주일:",   mkLbl(r.moveInDate.isValid()
                                      ? r.moveInDate.toString("yyyy년 MM월 dd일") : "-"));
    form->addRow("메모:",     mkLbl(r.memo.isEmpty() ? "-" : r.memo));

    // 차량 정보 그룹
    auto *carGroup  = new QGroupBox(QString("차량 정보  (%1대)").arg(cars.size()), &dlg);
    auto *carLayout = new QVBoxLayout(carGroup);

    if (cars.isEmpty()) {
        auto *none = new QLabel("등록된 차량 없음");
        none->setAlignment(Qt::AlignCenter);
        none->setStyleSheet("color:gray;");
        carLayout->addWidget(none);
    } else {
        auto *carList = new QListWidget(carGroup);
        carList->setFixedHeight(34 * cars.size() + 8); // 리스트 크기를 차 대수만큼만 맞춤 (스크롤 방지)
        carList->setFocusPolicy(Qt::NoFocus);          // 항목 클릭 시 점선 미출력
        for (int i = 0; i < cars.size(); ++i) {
            auto *item = new QListWidgetItem(
                QString("  %1.  %2").arg(i + 1).arg(cars[i].carNumber));
            item->setForeground(QColor("#64748B"));
            carList->addItem(item);
        }
        carLayout->addWidget(carList);
    }

    // 수정 / 닫기 버튼
    auto *btnBox  = new QDialogButtonBox(&dlg);
    auto *editBtn = btnBox->addButton("수정", QDialogButtonBox::ActionRole);
    editBtn->setObjectName("addBtn");
    btnBox->addButton("닫기", QDialogButtonBox::RejectRole);

    // 수정 버튼: 상세 팝업 닫고 수정 팝업 열기
    connect(editBtn, &QPushButton::clicked, &dlg, [&]() {
        dlg.accept();
        m_table->selectRow(index.row());
        showResidentDialog(true);
    });
    connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    auto *vbox = new QVBoxLayout(&dlg);
    vbox->addWidget(resGroup);
    vbox->addWidget(carGroup);
    vbox->addWidget(btnBox);
    dlg.exec();
}
