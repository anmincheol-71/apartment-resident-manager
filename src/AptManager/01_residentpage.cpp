#include "01_residentpage.h"

#include <QAbstractProxyModel>
#include <QColor>
#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QSqlQuery>
#include <QTableView>
#include <QVBoxLayout>

// 콤보박스 검색 기준 목록 (인덱스 순서 = m_searchField 순서와 일치)
static const QStringList kSearchFields = {"name", "phone", "car_number"};

// ── ResidentCarProxy ────────────────────────────────────────────────────────
// QSqlTableModel을 감싸서 차량번호 열을 동적으로 추가하는 프록시 모델
// - GROUP_CONCAT 쿼리로 여러 차량번호를 쉼표로 묶어 표시
// - CenterAlignProxy를 대체하며 가운데 정렬 기능도 내장
class ResidentCarProxy : public QAbstractProxyModel {
    Q_OBJECT
public:
    using QAbstractProxyModel::QAbstractProxyModel;

    int rowCount(const QModelIndex &parent = {}) const override {
        return (!parent.isValid() && sourceModel()) ? sourceModel()->rowCount() : 0;
    }

    // 소스 컬럼 수 + 1 (마지막 위치에 차량번호 열 추가)
    int columnCount(const QModelIndex &parent = {}) const override {
        return (!parent.isValid() && sourceModel()) ? sourceModel()->columnCount() + 1 : 0;
    }

    QModelIndex index(int row, int col, const QModelIndex &parent = {}) const override {
        return parent.isValid() ? QModelIndex{} : createIndex(row, col);
    }

    QModelIndex parent(const QModelIndex &) const override { return {}; }

    // 차량번호 열(마지막)은 소스 인덱스 없음 -> 빈 인덱스 반환
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override {
        if (!proxyIndex.isValid() || !sourceModel()) return {};
        if (proxyIndex.column() >= sourceModel()->columnCount()) return {};
        return sourceModel()->index(proxyIndex.row(), proxyIndex.column());
    }

    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override {
        if (!sourceIndex.isValid()) return {};
        return createIndex(sourceIndex.row(), sourceIndex.column());
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        // 마지막 열 헤더 = "차량번호"
        if (orientation == Qt::Horizontal && sourceModel()
                && section == sourceModel()->columnCount())
            return role == Qt::DisplayRole ? QString("차량번호") : QVariant{};
        return sourceModel() ? sourceModel()->headerData(section, orientation, role) : QVariant{};
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || !sourceModel()) return {};

        // 모든 셀 가운데 정렬 (CenterAlignProxy 역할 포함)
        if (role == Qt::TextAlignmentRole) return Qt::AlignCenter;

        // 차량번호 열 - DisplayRole만 처리, 나머지는 기본값
        if (index.column() == sourceModel()->columnCount()) {
            if (role != Qt::DisplayRole) return {};
            // ID 컬럼(0)으로 residentId 조회 후 car 테이블에서 GROUP_CONCAT
            int residentId = sourceModel()->data(
                sourceModel()->index(index.row(), 0)).toInt();
            QSqlQuery q;
            q.prepare("SELECT GROUP_CONCAT(carNumber, ', ') FROM car WHERE residentId = ?");
            q.addBindValue(residentId);
            if (q.exec() && q.next()) {
                QString cars = q.value(0).toString();
                return cars.isEmpty() ? QString("-") : cars;
            }
            return QString("-");
        }

        return sourceModel()->data(mapToSource(index), role);
    }
};

// moc가 ResidentCarProxy의 Q_OBJECT를 처리할 수 있도록 포함
#include "01_residentpage.moc"

// ── ResidentPage 생성자 ─────────────────────────────────────────────────────
ResidentPage::ResidentPage(ResidentManager *mgr, QWidget *parent)
    : QWidget(parent), m_mgr(mgr)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── 툴바 ─────────────────────────────────────────────
    auto *toolbar = new QWidget(this);
    toolbar->setObjectName("toolbarWidget");
    toolbar->setFixedHeight(56);

    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(16, 10, 16, 10);
    tbLayout->setSpacing(8);

    m_searchField = new QComboBox(toolbar);
    m_searchField->addItems({"이름", "연락처", "차량번호"});
    m_searchField->setFixedWidth(90);

    m_searchEdit = new QLineEdit(toolbar);
    m_searchEdit->setPlaceholderText("검색어를 입력하세요");
    m_searchEdit->setMinimumWidth(200);

    auto *searchBtn = new QPushButton("검색", toolbar);
    auto *clearBtn = new QPushButton("전체보기", toolbar);
    searchBtn->setObjectName("searchBtn");
    clearBtn->setObjectName("clearBtn");

    auto *addBtn = new QPushButton("+ 등록", toolbar);
    auto *editBtn = new QPushButton("수정", toolbar);
    auto *deleteBtn = new QPushButton("삭제", toolbar);
    addBtn->setObjectName("addBtn");
    editBtn->setObjectName("editBtn");
    deleteBtn->setObjectName("deleteBtn");

    tbLayout->addWidget(m_searchField);
    tbLayout->addWidget(m_searchEdit);
    tbLayout->addWidget(searchBtn);
    tbLayout->addWidget(clearBtn);
    tbLayout->addStretch();
    tbLayout->addWidget(addBtn);
    tbLayout->addWidget(editBtn);
    tbLayout->addWidget(deleteBtn);

    // ── 테이블 ─────────────────────────────────────────────
    auto *tableContainer = new QWidget(this);
    tableContainer->setObjectName("tableContainer");

    auto *tcLayout = new QVBoxLayout(tableContainer);
    tcLayout->setContentsMargins(16, 16, 16, 16);

    m_table = new QTableView(tableContainer);

    // ResidentCarProxy: 가운데 정렬 + 차량번호 열 추가
    auto *proxy = new ResidentCarProxy(m_table);
    proxy->setSourceModel(m_mgr->residentModel());
    m_table->setModel(proxy);

    m_table->hideColumn(0);                              // ID 숨김
    m_table->verticalHeader()->setVisible(false);
    m_table->setShowGrid(false);
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSortingEnabled(true);

    auto *hdr = m_table->horizontalHeader();
    hdr->setSectionResizeMode(QHeaderView::Interactive);
    hdr->setStretchLastSection(true);   // 메모(logical 6, 시각적 마지막)이 나머지 공간 차지
    m_table->setColumnWidth(1, 60);     // 동
    m_table->setColumnWidth(2, 60);     // 호
    m_table->setColumnWidth(3, 100);    // 이름
    m_table->setColumnWidth(4, 130);    // 연락처
    m_table->setColumnWidth(5, 100);    // 입주일
    m_table->setColumnWidth(7, 180);    // 차량번호 (logical 7, visual 6)
    hdr->moveSection(6, 7);             // 메모를 시각적 맨 끝으로 -> 차량번호가 입주일 뒤로

    tcLayout->addWidget(m_table);

    root->addWidget(toolbar);
    root->addWidget(tableContainer, 1);

    // ── 시그널 연결 ─────────────────────────────────────────
    connect(addBtn,      &QPushButton::clicked,      this, &ResidentPage::onAdd);
    connect(editBtn,     &QPushButton::clicked,      this, &ResidentPage::onEdit);
    connect(deleteBtn,   &QPushButton::clicked,      this, &ResidentPage::onDelete);
    connect(searchBtn,   &QPushButton::clicked,      this, &ResidentPage::onSearch);
    connect(clearBtn,    &QPushButton::clicked,      this, &ResidentPage::onClear);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &ResidentPage::onSearch);
    connect(m_table,     &QTableView::doubleClicked, this, &ResidentPage::onRowDoubleClicked);
}

// ── private 헬퍼 ───────────────────────────────────────────────────────────

// 현재 선택된 행의 입주민 ID 반환 - 미선택 시 0
int ResidentPage::selectedResidentId() const
{
    QModelIndex idx = m_table->currentIndex();
    if (!idx.isValid()) return 0;

    auto *proxy = qobject_cast<QAbstractProxyModel*>(m_table->model());
    // 클릭한 열과 무관하게 항상 column 0 (ID)로 안전하게 조회
    QModelIndex srcId = proxy
        ? proxy->mapToSource(proxy->index(idx.row(), 0))
        : m_mgr->residentModel()->index(idx.row(), 0);

    return m_mgr->residentModel()->data(srcId).toInt();
}

// 등록/수정 다이얼로그 표시
void ResidentPage::showResidentDialog(bool isEdit)
{
    Resident r;
    QList<Car> existingCars;

    if (isEdit) {
        int id = selectedResidentId();
        if (id == 0) {
            QMessageBox::information(this, "알림", "수정할 세대를 선택하세요.");
            return;
        }
        r = m_mgr->getResidentById(id);
        existingCars = m_mgr->getCarsByResidentId(id);
    }

    QDialog dlg(this);
    dlg.setWindowTitle(isEdit ? "세대 수정" : "세대 등록");
    dlg.setMinimumWidth(360);

    auto *dongSpin = new QSpinBox(&dlg);
    dongSpin->setRange(1, 999);
    dongSpin->setValue(isEdit ? r.dong : 101);

    auto *hoSpin = new QSpinBox(&dlg);
    hoSpin->setRange(1, 9999);
    hoSpin->setValue(isEdit ? r.ho : 101);

    auto *nameEdit = new QLineEdit(isEdit ? r.name : "", &dlg);
    auto *phoneEdit = new QLineEdit(isEdit ? r.phone : "", &dlg);
    phoneEdit->setPlaceholderText("010-0000-0000");

    auto *dateEdit = new QDateEdit(&dlg);
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("yyyy-MM-dd");
    dateEdit->setDate((isEdit && r.moveInDate.isValid()) ? r.moveInDate : QDate::currentDate());

    auto *memoEdit = new QLineEdit(isEdit ? r.memo : "", &dlg);

    QLineEdit *carEdits[3];
    for (int i = 0; i < 3; ++i) {
        carEdits[i] = new QLineEdit(&dlg);
        carEdits[i]->setPlaceholderText("예: 12가1234");
        if (i < existingCars.size())
            carEdits[i]->setText(existingCars[i].carNumber);
    }

    auto *form = new QFormLayout;
    form->addRow("동:", dongSpin);
    form->addRow("호:", hoSpin);
    form->addRow("이름:", nameEdit);
    form->addRow("연락처:", phoneEdit);
    form->addRow("입주일:", dateEdit);
    form->addRow("메모:", memoEdit);
    form->addRow("차량 1:", carEdits[0]);
    form->addRow("차량 2:", carEdits[1]);
    form->addRow("차량 3:", carEdits[2]);

    auto *btnBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(btnBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    auto *vbox = new QVBoxLayout(&dlg);
    vbox->addLayout(form);
    vbox->addWidget(btnBox);

    if (dlg.exec() != QDialog::Accepted) return;

    if (nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "입력 오류", "이름을 입력하세요.");
        return;
    }

    Resident res;
    res.id = isEdit ? r.id : 0;
    res.dong = dongSpin->value();
    res.ho = hoSpin->value();
    res.name = nameEdit->text().trimmed();
    res.phone = phoneEdit->text().trimmed();
    res.moveInDate = dateEdit->date();
    res.memo = memoEdit->text().trimmed();

    QStringList cars;
    for (int i = 0; i < 3; ++i) cars << carEdits[i]->text();

    bool ok = isEdit ? m_mgr->updateResident(res, cars) : m_mgr->addResident(res, cars);
    if (!ok) {
        QMessageBox::warning(this, "오류",
            isEdit ? "수정 실패\n(동/호 중복 여부 확인)" : "등록 실패\n(동/호 중복 여부 확인)");
    }
}

// ── 슬롯 구현 ──────────────────────────────────────────────────────────────

void ResidentPage::onAdd()  { showResidentDialog(false); }
void ResidentPage::onEdit() { showResidentDialog(true);  }

void ResidentPage::onDelete()
{
    int id = selectedResidentId();
    if (id == 0) {
        QMessageBox::information(this, "알림", "삭제할 세대를 선택하세요.");
        return;
    }
    Resident r = m_mgr->getResidentById(id);
    if (QMessageBox::question(this, "삭제 확인",
            QString("%1동 %2호 (%3) 를 삭제하시겠습니까?\n등록 차량도 함께 삭제됩니다.")
                .arg(r.dong).arg(r.ho).arg(r.name))
        == QMessageBox::Yes)
        m_mgr->removeResident(id);
}

void ResidentPage::onSearch()
{
    int idx = m_searchField->currentIndex();
    if (idx >= 0 && idx < kSearchFields.size())
        m_mgr->setSearchFilter(kSearchFields[idx], m_searchEdit->text());
}

void ResidentPage::onClear()
{
    m_searchEdit->clear();
    m_mgr->clearFilter();
}

// 행 더블클릭 -> 세대 상세정보 다이얼로그 표시
void ResidentPage::onRowDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    auto *proxy = qobject_cast<QAbstractProxyModel*>(m_table->model());
    // 차량번호 열을 클릭해도 항상 column 0 (ID)로 안전하게 조회
    QModelIndex srcId = proxy
        ? proxy->mapToSource(proxy->index(index.row(), 0))
        : m_mgr->residentModel()->index(index.row(), 0);
    int id = m_mgr->residentModel()->data(srcId).toInt();
    if (id == 0) return;

    Resident r = m_mgr->getResidentById(id);
    QList<Car> cars = m_mgr->getCarsByResidentId(id);

    QDialog dlg(this);
    dlg.setWindowTitle(QString("%1동 %2호 — 세대 상세정보").arg(r.dong).arg(r.ho));
    dlg.setMinimumWidth(400);

    auto *resGroup = new QGroupBox("세대 정보", &dlg);
    auto *form = new QFormLayout(resGroup);
    form->setLabelAlignment(Qt::AlignRight);

    auto mkLbl = [](const QString &t) {
        auto *l = new QLabel(t);
        l->setTextInteractionFlags(Qt::TextSelectableByMouse);
        return l;
    };
    form->addRow("동 / 호:",  mkLbl(QString("%1동  %2호").arg(r.dong).arg(r.ho)));
    form->addRow("이름:",     mkLbl(r.name));
    form->addRow("연락처:",   mkLbl(r.phone.isEmpty()    ? "-" : r.phone));
    form->addRow("입주일:",   mkLbl(r.moveInDate.isValid()
        ? r.moveInDate.toString("yyyy년 MM월 dd일") : "-"));
    form->addRow("메모:",     mkLbl(r.memo.isEmpty()     ? "-" : r.memo));

    auto *carGroup = new QGroupBox(
        QString("차량 정보  (%1대)").arg(cars.size()), &dlg);
    auto *carLayout = new QVBoxLayout(carGroup);

    if (cars.isEmpty()) {
        auto *none = new QLabel("등록된 차량 없음");
        none->setAlignment(Qt::AlignCenter);
        none->setStyleSheet("color:gray;");
        carLayout->addWidget(none);
    } else {
        auto *carList = new QListWidget(carGroup);
        carList->setFixedHeight(34 * cars.size() + 8);
        carList->setFocusPolicy(Qt::NoFocus);
        for (int i = 0; i < cars.size(); ++i) {
            auto *item = new QListWidgetItem(
                QString("  %1.  %2").arg(i + 1).arg(cars[i].carNumber));
            item->setForeground(QColor("#64748B"));
            carList->addItem(item);
        }
        carLayout->addWidget(carList);
    }

    auto *btnBox = new QDialogButtonBox(&dlg);
    auto *editBtn = btnBox->addButton("수정", QDialogButtonBox::ActionRole);
    editBtn->setObjectName("addBtn");
    btnBox->addButton("닫기", QDialogButtonBox::RejectRole);

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
