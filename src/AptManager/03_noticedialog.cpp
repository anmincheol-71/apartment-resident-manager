#include "03_noticedialog.h"
#include "03_noticemanager.h"

#include <QAudioOutput>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMediaPlayer>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlQuery>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>
#include <QVideoWidget>

// ── 생성자 ─────────────────────────────────────────────────────────────────
NoticeDialog::NoticeDialog(const Notice &prefill, NoticeManager *mgr, QWidget *parent)
    : QDialog(parent)
    , m_mgr(mgr)
    , m_readOnly(mgr == nullptr)
{
    setWindowTitle(m_readOnly ? "공지 상세 보기" : "공지 등록");
    setMinimumWidth(540);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 20, 24, 16);
    root->setSpacing(14);

    // ── 폼 ───────────────────────────────────────────────────────────────
    m_form = new QFormLayout;
    m_form->setRowWrapPolicy(QFormLayout::DontWrapRows);
    m_form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    m_form->setSpacing(10);

    // 유형 선택 콤보박스
    m_typeCombo = new QComboBox;
    m_typeCombo->addItems({"일반", "보안경고", "주의인물"});
    m_typeCombo->setCurrentText(prefill.type.isEmpty() ? "일반" : prefill.type);
    m_typeCombo->setEnabled(!m_readOnly);
    m_form->addRow("유형 :", m_typeCombo);

    // 제목 입력 칸
    m_titleEdit = new QLineEdit(prefill.title);
    m_titleEdit->setPlaceholderText("공지 제목을 입력하세요");
    m_titleEdit->setReadOnly(m_readOnly);
    m_form->addRow("제목 :", m_titleEdit);

    // 내용 입력 칸
    m_contentEdit = new QTextEdit;
    m_contentEdit->setPlainText(prefill.content);
    m_contentEdit->setPlaceholderText("공지 내용을 입력하세요");
    m_contentEdit->setReadOnly(m_readOnly);
    m_contentEdit->setFixedHeight(110);
    m_form->addRow("내용 :", m_contentEdit);

    // 열람 모드에서 layout 미포함 시 parent 렌더링으로 (0,0)에 떠서 명시적으로 숨김
    m_videoEdit = new QLineEdit(prefill.videoPath, this);
    if (m_readOnly) m_videoEdit->hide();

    if (!m_readOnly) {
        // 작성 모드: 경로 입력 칸 + 찾아보기 버튼
        m_videoEdit->setPlaceholderText("영상 파일 경로 (선택 사항)");
        auto *videoRow = new QHBoxLayout;
        videoRow->addWidget(m_videoEdit);

        auto *browseBtn = new QPushButton("찾아보기");
        connect(browseBtn, &QPushButton::clicked, this, &NoticeDialog::onBrowseVideo);
        videoRow->addWidget(browseBtn);

        auto *videoContainer = new QWidget;
        videoContainer->setLayout(videoRow);
        m_form->addRow("영상 파일 :", videoContainer);
    }

    // ── 수신대상 ──────────────────────────────────────────────────────────
    if (m_readOnly) {
        // 열람 모드: 단순 텍스트로 표시
        auto *targetEdit = new QLineEdit(formatTarget(prefill));
        targetEdit->setReadOnly(true);
        m_form->addRow("수신대상 :", targetEdit);
    } else {
        // 작성 모드: 콤보박스로 선택
        m_targetTypeCombo = new QComboBox;
        m_targetTypeCombo->addItems({"전체 세대", "특정 동", "특정 세대"});
        m_form->addRow("수신대상 :", m_targetTypeCombo);

        // 동 선택 - 특정 동 또는 특정 세대 선택 시 표시
        m_dongCombo = new QComboBox;
        m_form->addRow("동 선택 :", m_dongCombo);
        m_form->setRowVisible(m_dongCombo, false);

        // 호 선택 - 특정 세대 선택 시 표시
        m_hoCombo = new QComboBox;
        m_form->addRow("호 선택 :", m_hoCombo);
        m_form->setRowVisible(m_hoCombo, false);

        connect(m_targetTypeCombo, &QComboBox::currentTextChanged,
                this, &NoticeDialog::onTargetTypeChanged);
        connect(m_dongCombo, &QComboBox::currentIndexChanged,
                this, &NoticeDialog::onDongChanged);

        // 보안알림 연동 시 prefill 값 적용
        if (prefill.targetType == "동" || prefill.targetType == "세대") {
            m_targetTypeCombo->setCurrentText(
                prefill.targetType == "동" ? "특정 동" : "특정 세대"
            );
            if (prefill.targetDong > 0) {
                int idx = m_dongCombo->findText(QString::number(prefill.targetDong));
                if (idx >= 0) m_dongCombo->setCurrentIndex(idx);
                if (prefill.targetType == "세대" && prefill.targetHo > 0) {
                    int hidx = m_hoCombo->findText(QString::number(prefill.targetHo));
                    if (hidx >= 0) m_hoCombo->setCurrentIndex(hidx);
                }
            }
        }
    }

    root->addLayout(m_form);

    // 열람 모드 + 영상 있을 때만 재생 버튼 표시
    if (m_readOnly && !prefill.videoPath.isEmpty()) {
        auto *playBtn = new QPushButton("▶   CCTV 영상 재생");
        playBtn->setCursor(Qt::PointingHandCursor);
        playBtn->setStyleSheet(
            "QPushButton { background-color:#1565C0; color:white; border-radius:5px;"
            "              padding:8px 16px; font-size:13px; }"
            "QPushButton:hover { background-color:#0D47A1; }"
        );
        connect(playBtn, &QPushButton::clicked, this, &NoticeDialog::onPlayVideo);
        root->addWidget(playBtn);
    }

    // ── 버튼 박스 ─────────────────────────────────────────────────────────
    auto *btnBox = new QDialogButtonBox(this);
    if (m_readOnly) {
        // 열람 모드: 닫기만
        btnBox->addButton(QDialogButtonBox::Close);
        connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    } else {
        // 작성 모드: 저장 + 취소
        auto *saveBtn = btnBox->addButton("저장", QDialogButtonBox::AcceptRole);
        btnBox->addButton(QDialogButtonBox::Cancel);
        connect(saveBtn, &QPushButton::clicked,       this, &NoticeDialog::onSave);
        connect(btnBox,  &QDialogButtonBox::rejected, this, &QDialog::reject);
    }
    root->addWidget(btnBox);
}

// ── 슬롯 구현 ──────────────────────────────────────────────────────────────

// 수신대상 변경 시 동/호 행 표시 여부 갱신
void NoticeDialog::onTargetTypeChanged(const QString &value)
{
    bool showDong = (value == "특정 동" || value == "특정 세대");
    bool showHo = (value == "특정 세대");

    m_form->setRowVisible(m_dongCombo, showDong);
    m_form->setRowVisible(m_hoCombo, showHo);

    // 동 목록이 아직 비어있으면 로드
    if (showDong && m_dongCombo->count() == 0)
        loadDongs();

    adjustSize();
}

// 동 변경 시 호 목록 재로드
void NoticeDialog::onDongChanged(int index)
{
    if (!m_hoCombo || !m_form->isRowVisible(m_hoCombo)) return;
    int dong = m_dongCombo->itemText(index).toInt();
    if (dong > 0) loadHos(dong);
}

// 파일 탐색기에서 영상 파일 선택
void NoticeDialog::onBrowseVideo()
{
    QString path = QFileDialog::getOpenFileName(
        this, "영상 파일 선택", {},
        "영상 파일 (*.mp4 *.avi *.mov *.mkv)"
    );
    if (!path.isEmpty())
        m_videoEdit->setText(path);
}

// 유효성 검사 후 DB에 공지 등록
void NoticeDialog::onSave()
{
    if (m_titleEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "입력 오류", "제목을 입력해주세요.");
        return;
    }

    // 수신대상 파싱
    QString targetType = "전체";
    int targetDong = 0, targetHo = 0;

    if (m_targetTypeCombo) {
        QString tv = m_targetTypeCombo->currentText();
        if (tv == "특정 동") {
            targetType = "동";
            targetDong = m_dongCombo->currentText().toInt();
        } else if (tv == "특정 세대") {
            targetType = "세대";
            targetDong = m_dongCombo->currentText().toInt();
            targetHo = m_hoCombo->currentText().toInt();
        }
    }

    int id = m_mgr->addNotice(
        m_typeCombo->currentText(),
        m_titleEdit->text().trimmed(),
        m_contentEdit->toPlainText().trimmed(),
        m_videoEdit->text().trimmed(),
        targetType, targetDong, targetHo
    );

    if (id < 0) {
        QMessageBox::critical(this, "오류", "공지 등록에 실패했습니다.");
        return;
    }

    accept();
}

// QMediaPlayer 팝업으로 영상 재생 (열람 모드)
void NoticeDialog::onPlayVideo()
{
    QString path = m_videoEdit->text();
    if (path.isEmpty() || !QFile::exists(path)) {
        QMessageBox::warning(this, "파일 없음", "영상 파일을 찾을 수 없습니다.\n경로: " + path);
        return;
    }

    auto *dlg = new QDialog(this, Qt::Window);
    dlg->setWindowTitle("CCTV 영상 재생");
    dlg->setMinimumSize(854, 520);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    auto *layout = new QVBoxLayout(dlg);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *videoWidget = new QVideoWidget;
    videoWidget->setStyleSheet("background:black;");
    layout->addWidget(videoWidget);

    auto *player = new QMediaPlayer(dlg);
    auto *audio = new QAudioOutput(dlg);
    player->setAudioOutput(audio);
    player->setVideoOutput(videoWidget);
    player->setSource(QUrl::fromLocalFile(path));
    player->play();

    dlg->show();
}

// ── private 헬퍼 ───────────────────────────────────────────────────────────

// 입주민 DB에서 동 목록 로드
void NoticeDialog::loadDongs()
{
    m_dongCombo->clear();
    QSqlQuery q("SELECT DISTINCT dong FROM resident ORDER BY dong");
    while (q.next())
        m_dongCombo->addItem(QString::number(q.value(0).toInt()));
}

// 선택된 동에 속한 호 목록 로드
void NoticeDialog::loadHos(int dong)
{
    m_hoCombo->clear();
    QSqlQuery q;
    q.prepare("SELECT DISTINCT ho FROM resident WHERE dong = ? ORDER BY ho");
    q.addBindValue(dong);
    if (q.exec())
        while (q.next())
            m_hoCombo->addItem(QString::number(q.value(0).toInt()));
}

// Notice의 수신대상을 읽기 편한 문자열로 변환
QString NoticeDialog::formatTarget(const Notice &n)
{
    if (n.targetType == "동")   return QString("%1동").arg(n.targetDong);
    if (n.targetType == "세대") return QString("%1동 %2호").arg(n.targetDong).arg(n.targetHo);
    return "전체 세대";
}
