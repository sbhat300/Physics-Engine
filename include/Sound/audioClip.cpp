#include "Sound/audioClip.h"

audioClip::audioClip(){}

audioClip::audioClip(wavFile* a)
{
    audio = a;
    pos = glm::vec2(0, 0);
    speed = 1.0f;
    reverse = false;
    pause = false;
    loop = false;
    framesPlayed = 0;
}

void audioClip::loadAudio(wavFile* a)
{
    audio = a;
    framesPlayed = 0;
}

void audioClip::reverseState(bool state)
{
    reverse = state;
}

void audioClip::pauseState(bool state)
{
    pause = state;
}