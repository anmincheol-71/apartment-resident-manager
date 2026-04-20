# 🏢 아파트 입주민 관리 시스템

> Qt 기반 아파트 입주민 정보 관리 프로그램  
> 한화비전 VEDA 심화 실습 평가 1 제출 프로젝트

---

## 👥 팀 구성

| 이름 | 역할 | 담당 영역 |
|---|---|---|
| 정석 | UI / 프레젠테이션 | MainWindow, AddEditDialog, 스타일/리소스 |
| 민철 | 데이터 / DB | DatabaseManager, ResidentManager, 스키마 |

---

## ✨ 주요 기능

### 필수 기능 (평가기준 필수 포함)
- 입주민 정보 **등록 / 수정 / 삭제** (CRUD)
- 세대주 및 차량 정보 관리
- 이름 / 동 / 호 / 전화번호 **검색 및 필터링**
- SQLite DB 저장

### 부가 기능 (외부리소스 활용)
- 커스텀 QSS 테마 (다크/라이트)
- 아이콘 세트 적용 (Bootstrap Icons / Feather Icons)
- 통계 차트 (QtCharts) — 주차 등록 비율
- CSV 가져오기 / 내보내기

---

## 🛠 기술 스택

| 구분 | 사용 기술 |
|---|---|
| 언어 | C++17 |
| 프레임워크 | Qt 6.11.0 (MinGW 64-bit) |
| 데이터베이스 | SQLite (QSqlDatabase) |
| 빌드 시스템 | qmake |
| IDE | Qt Creator |
| 버전 관리 | Git / GitHub |

---

## 📁 프로젝트 구조

```
apartment-resident-manager/
├── src/                      # 소스 코드
│   ├── main.cpp
│   ├── mainwindow.{h,cpp,ui}       # 메인 윈도우 [UI]
│   ├── addeditdialog.{h,cpp,ui}    # 등록/수정 다이얼로그 [UI]
│   ├── resident.h                  # 데이터 모델 (공통)
│   ├── databasemanager.{h,cpp}     # DB 연결 관리 [DB]
│   └── residentmanager.{h,cpp}     # CRUD 비즈니스 로직 [DB]
├── resources/                # 리소스
│   ├── icons/                # 툴바/메뉴 아이콘
│   ├── style/                # QSS 스타일시트
│   └── sample_data/          # seed.sql, 샘플 CSV
├── docs/                     # 문서
│   ├── 기획서.docx
│   └── screenshots/          # UI 스크린샷
├── .gitignore
├── README.md
└── apartment.pro             # Qt 프로젝트 파일
```

---

## 🚀 빌드 및 실행

### 요구사항
- Qt 6.11.0 이상
- MinGW 64-bit 컴파일러
- Qt Creator (권장)

### 방법
1. Qt Creator에서 `apartment.pro` 열기
2. Configure Project → MinGW 64-bit 킷 선택
3. 좌측 하단 ▶️ 버튼으로 빌드 & 실행
4. 첫 실행 시 자동으로 DB 파일 생성 및 샘플 데이터 삽입

---

## 🤝 협업 규칙

### 브랜치 전략
```
main                   ← 안정 버전 (제출 대상)
├── feat/ui            ← 정석 (UI)
└── feat/db            ← [파트너] (DB)
```

- 각자 자기 브랜치에서 작업
- 하루 **2회 이상** `main`에 병합 (점심·저녁 권장)
- 인터페이스 헤더(`resident.h`, `residentmanager.h`) 변경은 **반드시 상대에게 공유**

### 커밋 메시지 접두어
| 접두어 | 용도 |
|---|---|
| `[UI]` | UI 관련 변경 |
| `[DB]` | DB / 비즈니스 로직 관련 변경 |
| `[공통]` | 인터페이스 헤더, 설정 파일 |
| `[문서]` | README, 기획서 등 |
| `[리소스]` | 아이콘, QSS, 샘플 데이터 |

예: `[UI] 툴바 아이콘 세트 적용`

---

## 📅 개발 일정

| 날짜 | 주요 목표 |
|---|---|
| **4/20 (월)** | 프로젝트 세팅, 인터페이스 합의, UI 뼈대 + DB 스키마 |
| **4/21 (화)** | 기능 구현 완료, 스타일링, 통합 테스트 |
| **4/22 (수)** | 기획서 작성, 최종 점검, **17시 제출** |

---

## 📚 외부 리소스 출처

> 개발 진행하면서 추가/업데이트 예정

| 리소스 | 출처 | 라이선스 | 사용 위치 |
|---|---|---|---|
| (아이콘) | (TBD) | | |
| (QSS) | (TBD) | | |

---

## 📝 제출물 구성

| 항목 | 위치 |
|---|---|
| 소스 코드 | `src/`, `resources/`, `apartment.pro` |
| 기획서 | `docs/기획서.docx` |
| 스크린샷 | `docs/screenshots/` |
| 샘플 데이터 | `resources/sample_data/seed.sql` |

---

## 📄 라이선스

한화비전 VEDA 교육 과정 학습 목적 프로젝트
