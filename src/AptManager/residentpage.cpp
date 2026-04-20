#include "residentpage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QHeaderView>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QSpinBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QColor>

static const QStringList kSearchFields = {"name", "dong", "ho", "phone"};

ResidentPage::ResidentPage(ResidentManager *mgr, QWidget *parent)
    : QWidget(parent), m_mgr(mgr)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── 툴바 ──
    auto *toolbar = new QWidget(this);
    toolbar->setObjectName("toolbarWidget");
    toolbar->setFixedHeight(56);

    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(16, 10, 16, 10);
    tbLayout->setSpacing(8);

    m_searchField = new QComboBox(toolbar);
    m_searchField->addItems({"이름", "동", "호", "연락처"});
    m_searchField->setFixedWidth(90);

    m_searchEdit = new QLineEdit(toolbar);
    m_searchEdit->setPlaceholderText("검색어를 입력하세요");
    m_searchEdit->setMinimumWidth(200);

    auto *searchBtn = new QPushButton("검색",    toolbar);
    auto *clearBtn  = new QPushButton("전체보기", toolbar);
    searchBtn->setObjectName("searchBtn");
    clearBtn->setObjectName("clearBtn");

    auto *addBtn    = new QPushButton("+ 등록", toolbar);
    auto *editBtn   = new QPushButton("수정",   toolbar);
    auto *deleteBtn = new QPushButton("삭제",   toolbar);
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

    // ── 테이블 ──
    auto *tableContainer = new QWidget(this);
    tableContainer->setObjectName("tableContainer");

    auto *tcLayout = new QVBoxLayout(tableContainer);
    tcLayout->setContentsMargins(16, 16, 16, 16);

    m_table = new QTableView(tableContainer);
    m_table->setModel(m_mgr->residentModel());
    m_table->hideColumn(0);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setShowGrid(false);
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    tcLayout->addWidget(m_table);

    root->addWidget(toolbar);
    root->addWidget(tableContainer, 1);

    // ── 시그널 연결 ──
    connect(addBtn,    &QPushButton::clicked,     this, &ResidentPage::onAdd);
    connect(editBtn,   &QPushButton::clicked,     this, &ResidentPage::onEdit);
    connect(deleteBtn, &QPushButton::clicked,     this, &ResidentPage::onDelete);
    connect(searchBtn, &QPushButton::clicked,     this, &ResidentPage::onSearch);
    connect(clearBtn,  &QPushButton::clicked,     this, &ResidentPage::onClear);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &ResidentPage::onSearch);
    connect(m_table, &QTableView::doubleClicked,  this, &ResidentPage::onRowDoubleClicked);
}

int ResidentPage::selectedResidentId() const
{
    QModelIndex idx = m_table->currentIndex();
    if (!idx.isValid()) return 0;
    return m_mgr->residentModel()->data(
        m_mgr->residentModel()->index(idx.row(), 0)
    ).toInt();
}

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

    auto *nameEdit  = new QLineEdit(isEdit ? r.name  : "", &dlg);
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
    form->addRow("동:",    dongSpin);
    form->addRow("호:",    hoSpin);
    form->addRow("이름:",   nameEdit);
    form->addRow("연락처:", phoneEdit);
    form->addRow("입주일:", dateEdit);
    form->addRow("메모:",   memoEdit);
    form->addRow("차량 1:", carEdits[0]);
    form->addRow("차량 2:", carEdits[1]);
    form->addRow("차량 3:", carEdits[2]);

    auto *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
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
    res.id         = isEdit ? r.id : 0;
    res.dong       = dongSpin->value();
    res.ho         = hoSpin->value();
    res.name       = nameEdit->text().trimmed();
    res.phone      = phoneEdit->text().trimmed();
    res.moveInDate = dateEdit->date();
    res.memo       = memoEdit->text().trimmed();

    QStringList cars;
    for (int i = 0; i < 3; ++i) cars << carEdits[i]->text();

    bool ok = isEdit ? m_mgr->updateResident(res, cars) : m_mgr->addResident(res, cars);
    if (!ok) {
        QMessageBox::warning(this, "오류",
            isEdit ? "수정 실패\n(동/호 중복 여부 확인)" : "등록 실패\n(동/호 중복 여부 확인)");
    }
}

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

void ResidentPage::onRowDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;
    int id = m_mgr->residentModel()->data(
        m_mgr->residentModel()->index(index.row(), 0)).toInt();
    if (id == 0) return;

    Resident r    = m_mgr->getResidentById(id);
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
    form->addRow("연락처:",   mkLbl(r.phone.isEmpty() ? "-" : r.phone));
    form->addRow("입주일:",   mkLbl(r.moveInDate.isValid()
        ? r.moveInDate.toString("yyyy년 MM월 dd일") : "-"));
    form->addRow("메모:",     mkLbl(r.memo.isEmpty() ? "-" : r.memo));

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
            const Car &c = cars[i];
            QString status = c.isParked
                ? QString("  [%1층 %2번 주차 중]")
                    .arg(c.parkingSpot.section('-', 0, 0))
                    .arg(c.parkingSpot.section('-', 1))
                : "  [미주차]";
            auto *item = new QListWidgetItem(
                QString("  %1.  %2%3").arg(i + 1).arg(c.carNumber).arg(status));
            item->setForeground(c.isParked ? QColor("#EF4444") : QColor("#64748B"));
            carList->addItem(item);
        }
        carLayout->addWidget(carList);
    }

    auto *btnBox  = new QDialogButtonBox(&dlg);
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
