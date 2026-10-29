#include "clicksound.h"
#ifndef SKYSENSE_MULTIMEDIA
#include <QFile>
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mmsystem.h>
#endif

void ClickSound::setSource(const QUrl &url)
{
    QFile file(url.scheme() == "qrc" ? ":" + url.path() : url.toLocalFile());
    currentStatus = file.open(QIODevice::ReadOnly) ? Ready : Error;
    if (currentStatus == Ready) wave = file.readAll();
    emit statusChanged();
}

void ClickSound::play()
{
#ifdef Q_OS_WIN
    if (currentStatus == Ready)
        PlaySoundW(reinterpret_cast<LPCWSTR>(wave.constData()), nullptr,
                   SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
#endif
}

void ClickSound::stop()
{
#ifdef Q_OS_WIN
    PlaySoundW(nullptr, nullptr, 0);
#endif
}
#endif
