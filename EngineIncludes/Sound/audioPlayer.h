#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <soundio/soundio.h>
#include <Sound/audioLinkedList.h>

namespace audioPlayer
{

    extern audioLinkedList audios;

    int start();
    void writeCallback(SoundIoOutStream *outStream, int frameCountMin, int frameCountMax);
};

#endif