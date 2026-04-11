#ifndef AUDIOCLIP_H
#define AUDIOCLIP_H

#include <FileLoader/wavFile.h>
#include <glm/glm.hpp>

class audioClip
{
    public:
        wavFile* audio;
        float framesPlayed;
        glm::vec2 pos;
        float speed;
        bool reverse;
        bool loop;
        bool pause;
        int debugid;

        audioClip();
        audioClip(wavFile* a);
        void loadAudio(wavFile* a);
        void reverseState(bool state);
        void pauseState(bool state);
    private:

};

#endif