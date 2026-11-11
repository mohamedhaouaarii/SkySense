#pragma once

#ifdef SKYSENSE_MULTIMEDIA
#include <QSoundEffect>
using ClickSound = QSoundEffect;
#else
#include <QObject>
#include <QByteArray>
#include <QUrl>

// The same click sound remains available on Windows kits without Qt Multimedia.
class ClickSound final : public QObject {
    Q_OBJECT
public:
    enum Status { Null, Loading, Ready, Error };
    explicit ClickSound(QObject *parent = nullptr) : QObject(parent) {}
    ~ClickSound() override { stop(); }
    void setSource(const QUrl &url);
    void setLoopCount(int) {}
    void setVolume(float value) { volume = value; }
    Status status() const { return currentStatus; }
    void play();
    void stop();
signals:
    void statusChanged();
private:
    QByteArray wave;
    float volume = 0.4f;
    Status currentStatus = Null;
};
#endif
