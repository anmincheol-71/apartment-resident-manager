cmake_minimum_required(VERSION 3.19)
project(AptManager LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Sql Multimedia MultimediaWidgets)

qt_standard_project_setup()

qt_add_executable(AptManager
    WIN32 MACOSX_BUNDLE
    # 00 앱 진입점 / 메인
    00_main.cpp
    00_mainwindow.cpp
    00_mainwindow.h
    00_mainwindow.ui
    00_cctvutils.h
    # 01 입주민
    01_resident.h
    01_car.h
    01_residentmanager.h
    01_residentmanager.cpp
    01_residentpage.h
    01_residentpage.cpp
    # 02 보안 알림
    02_securityalert.h
    02_securityalertmanager.h
    02_securityalertmanager.cpp
    02_alerthistorypage.h
    02_alerthistorypage.cpp
    02_alertpopup.h
    02_alertpopup.cpp
    # 03 공지사항
    03_notice.h
    03_noticemanager.h
    03_noticemanager.cpp
    03_noticedialog.h
    03_noticedialog.cpp
    03_noticepage.h
    03_noticepage.cpp
)

target_link_libraries(AptManager
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Sql
        Qt6::Multimedia
        Qt6::MultimediaWidgets
)

# 빌드 후 실행파일 옆에 resources/sample_data/ 복사 -> findVideo() 탐색 성공
add_custom_command(TARGET AptManager POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/../../resources"
        "$<TARGET_FILE_DIR:AptManager>/resources"
    COMMENT "샘플 영상 리소스 복사"
)

install(TARGETS AptManager
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET AptManager
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})
