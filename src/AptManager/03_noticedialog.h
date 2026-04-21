#ifndef NOTICEDIALOG_H
#define NOTICEDIALOG_H

#include <QDialog>
#include "03_notice.h"

class NoticeManager;
class QComboBox;
class QFormLayout;
class QLineEdit;
class QTextEdit;

// 공지 작성 또는 열람 다이얼로그
// mgr != nullptr -> 작성 모드 / mgr == nullptr -> 열람 모드 (읽기 전용)
// prefill  초기값 (작성 모드에선 보안알림 연동 시 미리채움, 열람 모드에선 표시할 공지)
class NoticeDialog : public QDialog {
    Q_OBJECT

public:
    explicit NoticeDialog(const Notice &prefill, NoticeManager *mgr, QWidget *parent = nullptr);

private slots:
    // 수신대상 변경 시 동/호 선택 행 표시 여부 갱신
    void onTargetTypeChanged(const QString &value);

    // 동 변경 시 해당 동의 호 목록 재로드
    void onDongChanged(int index);

    // 파일 선택 버튼 클릭 시 파일 탐색기 열기
    void onBrowseVideo();

    // 저장 버튼 클릭 시 유효성 검사 후 DB 등록
    void onSave();

    // 영상 재생 버튼 클릭 시 QMediaPlayer 팝업 열기
    void onPlayVideo();

private:
    NoticeManager *m_mgr;  // nullptr이면 열람 모드
    bool m_readOnly;

    QFormLayout *m_form;
    QComboBox *m_typeCombo;
    QLineEdit *m_titleEdit;
    QTextEdit *m_contentEdit;
    QLineEdit *m_videoEdit;  // 영상 경로 입력 칸

    // 수신대상 관련 (작성 모드 전용)
    QComboBox *m_targetTypeCombo = nullptr;
    QComboBox *m_dongCombo = nullptr;
    QComboBox *m_hoCombo = nullptr;

    // 입주민 DB에서 동 목록 로드
    void loadDongs();

    // 선택된 동에 속한 호 목록 로드
    void loadHos(int dong);

    // Notice의 수신대상을 읽기 편한 문자열로 변환
    static QString formatTarget(const Notice &n);
};

#endif // NOTICEDIALOG_H
