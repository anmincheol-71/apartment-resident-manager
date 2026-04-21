#ifndef CCTVUTILS_H
#define CCTVUTILS_H

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QString>

// 실행 파일 위치에서 위로 탐색하며 resources/sample_data/{filename} 경로 반환
// 최대 7단계까지 올라가며 탐색 - 찾지 못하면 빈 문자열 반환
inline QString findVideo(const QString &filename)
{
    QDir dir(QCoreApplication::applicationDirPath());
    for (int i = 0; i < 7; ++i) {
        QString path = dir.filePath("resources/sample_data/" + filename);
        if (QFile::exists(path))
            return QDir::cleanPath(path);
        dir.cdUp();
    }
    return {};
}

#endif // CCTVUTILS_H
