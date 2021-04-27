#ifndef RWAAUDIOMETADATA_H
#define RWAAUDIOMETADATA_H

#include <QObject>
#include <QMediaPlayer>
#include <QMediaService>

class RwaAudioMetaData : public QObject
{
    Q_OBJECT
public:
    explicit RwaAudioMetaData(QObject *parent = nullptr);

    QMediaPlayer *player;
    void setFile(QString name);
    float getDuration();
    int getChannelCount();
    int getSampleRate();
};

#endif // RWAAUDIOMETADATA_H
