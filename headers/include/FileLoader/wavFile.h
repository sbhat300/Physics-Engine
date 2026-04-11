#ifndef WAVFILE_H
#define WAVFILE_H

#include <stdio.h>
#include <stdint.h>

class wavFile
{
    public:
        int32_t dataSize;
        int16_t audioFormat;
        int16_t numChannels;
        int32_t sampleRate;
        int32_t bytesPerSec;
        int16_t bytesPerBlock;
        int16_t bitsPerSample;
        unsigned char* bits8Data;
        int16_t* bits16Data;
        int32_t* bits32Data;

        wavFile();
        ~wavFile();
        bool read(const char* filePath);
        float getData(int frame, int channel);
        void resample(int target);
        void freeData();
    private:
        bool allocated[3];

        void read8Bit(FILE* wav);
        void read16Bit(FILE* wav);
        void read24Bit(FILE* wav);
        void read32Bit(FILE* wav);
};

#endif