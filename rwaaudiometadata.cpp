#include "rwaaudiometadata.h"

RwaAudioMetaData::RwaAudioMetaData(QObject *parent) : QObject(parent)
{
     player = new QMediaPlayer(this);
}

void RwaAudioMetaData::setFile(QString name)
{
    player->setMedia(QUrl::fromLocalFile(name));
    player->setVolume(0);
}

float RwaAudioMetaData::getDuration()
{
    float duration = static_cast<float>(player->duration());
    return (duration);
}

int RwaAudioMetaData::getChannelCount()
{
    int channelCount  = player->metaData("ChannelCount").toInt();
    return (channelCount);
}

int RwaAudioMetaData::getSampleRate()
{
    int sampleRate  = player->metaData("ChannelCount").toInt();
    return (sampleRate);
}
