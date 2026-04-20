#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QLineEdit>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QGroupBox>
#include <QPushButton>

static const QStringList kSearchFields = { "name", "dong", "ho", "phone" };

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_mgr(new ResidentManager(this))
{
    ui->setupUi(this);
    setMinimumSize(800, 560);

    if (!m_mgr->initialize()) {
        QMessageBox::critical(this, "오류", "데이터베이스 초기화에 실패했습니다.");
        return;
    }

    ui->tableView->setModel(m_mgr->residentModel());
    ui->tableView->hideColumn(0);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->verticalHeader()->setVisible(false);

    connect(ui->addBtn,     &QPushButton::clicked,      this, &MainWindow::onAdd);
    connect(ui->editBtn,    &QPushButton::clicked,      this, &MainWindow::onEdit);
    connect(ui->deleteBtn,  &QPushButton::clicked,      this, &MainWindow::onDelete);
    connect(ui->searchBtn,  &QPushButton::clicked,      this, &MainWindow::onSearch);
    connect(ui->clearBtn,   &QPushButton::clicked,      this, &MainWindow::onClear);
    connect(ui->searchEdit, &QLineEdit::returnPressed,  this, &MainWindow::onSearch);
    connect(m_mgr, &ResidentManager::dataChanged, this, &MainWindow::onDataChanged);
    connect(ui->tableView, &QTableView::doubleClicked, this, &MainWindow::onRowDoubleClicked);

    onDataChanged();
}

MainWindow::~MainWindow()
{
    delete ui;
}

int MainWindow::selectedResidentId() const
{
    QModelIndex idx = ui->tableView->currentIndex();
    if (!idx.isValid()) return 0;
    QModelIndex idIdx = m_mgr->residentModel()->index(idx.row(), 0);
    return m_mgr->residentModel()->data(idIdx).toInt();
}

void MainWindow::showResidentDialog(bool isEdit)
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
    for (int i = 0; i < 3; ++i)
        cars << carEdits[i]->text();

    bool ok = isEdit ? m_mgr->updateResident(res, cars) : m_mgr->addResident(res, cars);
    if (!ok) {
        QMessageBox::warning(this, "오류",
            isEdit ? "수정 실패\n(동/호 중복 여부 확인)" : "등록 실패\n(동/호 중복 여부 확인)");
    }
}

void MainWindow::onAdd()  { showResidentDialog(false); }
void MainWindow::onEdit() { showResidentDialog(true);  }

void MainWindow::onDelete()
{
    int id = selectedResidentId();
    if (id == 0) {
        QMessageBox::information(this, "알림", "삭제할 세대를 선택하세요.");
        return;
    }
    Resident r = m_mgr->getResidentById(id);
    auto ret = QMessageBox::question(
        this, "삭제 확인",
        QString("%1동 %2호 (%3) 를 삭제하시겠습니까?\n등록된 차량도 함께 삭제됩니다.")
            .arg(r.dong).arg(r.ho).arg(r.name)
    );
    if (ret == QMessageBox::Yes)
        m_mgr->removeResident(id);
}

void MainWindow::onSearch()
{
    int idx = ui->searchFieldCombo->currentIndex();
    if (idx < 0 || idx >= kSearchFields.size()) return;
    m_mgr->setSearchFilter(kSearchFields[idx], ui->searchEdit->text());
}

void MainWindow::onClear()
{
    ui->searchEdit->clear();
    m_mgr->clearFilter();
}

void MainWindow::onDataChanged()
{
    updateStatusBar();
}

void MainWindow::updateStatusBar()
{
    int count = m_mgr->residentModel()->rowCount();
    ui->statusbar->showMessage(QString("총 %1세대").arg(count));
}

void MainWindow::onRowDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    QModelIndex idIdx = m_mgr->residentModel()->index(index.row(), 0);
    int id = m_mgr->residentModel()->data(idIdx).toInt();
    if (id == 0) return;

    Resident r   = m_mgr->getResidentById(id);
    QList<Car> cars = m_mgr->getCarsByResidentId(id);

    QDialog dlg(this);
    dlg.setWindowTitle(QString("%1동 %2호 — 세대 상세정보").arg(r.dong).arg(r.ho));
    dlg.setMinimumWidth(380);

    // ── 세대 정보 그룹 ──
    auto *resGroup = new QGroupBox("세대 정보", &dlg);
    auto *form = new QFormLayout(resGroup);
    form->setLabelAlignment(Qt::AlignRight);

    auto makeLabel = [&](const QString &text) {
        auto *l = new QLabel(text);
        l->setTextInteractionFlags(Qt::TextSelectableByMouse);
        return l;
    };

    form->addRow("동 / 호:",  makeLabel(QString("%1동  %2호").arg(r.dong).arg(r.ho)));
    form->addRow("이름:",     makeLabel(r.name));
    form->addRow("연락처:",   makeLabel(r.phone.isEmpty() ? "-" : r.phone));
    form->addRow("입주일:",   makeLabel(r.moveInDate.isValid()
                                        ? r.moveInDate.toString("yyyy년 MM월 dd일") : "-"));
    form->addRow("메모:",     makeLabel(r.memo.isEmpty() ? "-" : r.memo));

    // ── 차량 정보 그룹 ──
    auto *carGroup = new QGroupBox(QString("차량 정보  (%1대)").arg(cars.size()), &dlg);
    auto *carLayout = new QVBoxLayout(carGroup);

    if (cars.isEmpty()) {
        auto *none = new QLabel("등록된 차량 없음");
        none->setAlignment(Qt::AlignCenter);
        none->setStyleSheet("color: gray;");
        carLayout->addWidget(none);
    } else {
        auto *carList = new QListWidget(carGroup);
        carList->setFixedHeight(28 * cars.size() + 8);
        carList->setFocusPolicy(Qt::NoFocus);
        for (int i = 0; i < cars.size(); ++i)
            carList->addItem(QString("  %1.  %2").arg(i + 1).arg(cars[i].carNumber));
        carLayout->addWidget(carList);
    }

    // ── 버튼 ──
    auto *btnBox  = new QDialogButtonBox(&dlg);
    auto *editBtn = btnBox->addButton("수정", QDialogButtonBox::ActionRole);
    editBtn->setObjectName("addBtn");   // 파란 버튼 스타일 재사용
    auto *closeBtn = btnBox->addButton("닫기", QDialogButtonBox::RejectRole);
    Q_UNUSED(closeBtn);

    connect(editBtn,  &QPushButton::clicked, &dlg, [&]() {
        dlg.accept();
        // 해당 행 선택 후 수정 다이얼로그 열기
        ui->tableView->selectRow(index.row());
        showResidentDialog(true);
    });
    connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    auto *vbox = new QVBoxLayout(&dlg);
    vbox->addWidget(resGroup);
    vbox->addWidget(carGroup);
    vbox->addWidget(btnBox);

    dlg.exec();
}
