#include "FileLoader/wavFile.h"
#include <FileLoader/fileLoader.h>
#include <iostream>
#include <Engine/setup.h>
#include <Engine/engineConstants.h>
#include <cstring>

wavFile::wavFile()
{
    dataSize = 0;
    for(int i = 0; i < 3; i++) allocated[i] = false;
}

bool wavFile::read(const char* file)
{
    const int8_t RIFF[4] = {0x52, 0x49, 0x46, 0x46};
    const int8_t WAVE[4] = {0x57, 0x41, 0x56, 0x45};
    const int8_t fmt[4] = {0x66, 0x6D, 0x74, 0x20};
    const int8_t dataHeader[4] = {0x64, 0x61, 0x74, 0x61};
    const int8_t LIST[4] = {0x4C, 0x49, 0x53, 0x54};
    const int8_t bext[4] = {0x62, 0x65, 0x78, 0x74};
    const int8_t iXML[4] = {0x69, 0x58, 0x4D, 0x4C};

    
    for(int i = 0; i < 3; i++) allocated[i] = false;
    FILE* wav;
    wav = fopen(fileLoader::loadWAV(file, setup::windows).c_str(), "rb");
    if(wav == NULL)
    {
        std::cout << "WAV FILE NOT OPENED" << std::endl;
        return false;
    }
    unsigned char header[4];
    if(fread(header, 1, 4, wav) == 0)
    {
        std::cout << "ERROR READING WAV FILE" << std::endl;
        return false;
    }
    if(memcmp(header, RIFF, sizeof(RIFF)) != 0)
    {
        std::cout << "NOT A RIFF" << std::endl;
        return false;
    }
    fread(&dataSize, 4, 1, wav);
    fread(header, 1, 4, wav);
    if(memcmp(header, WAVE, sizeof(WAVE)) != 0)
    {
        std::cout << "NOT A WAVE" << std::endl;
        return false;
    }
    fread(header, 1, 4, wav);
    while(memcmp(header, fmt, sizeof(fmt)) != 0)
    {
        fread(&dataSize, 4, 1, wav);
        fseek(wav, dataSize, SEEK_CUR);
        fread(header, 1, 4, wav);
    }
    int chunkSize;
    fread(&chunkSize, 4, 1, wav);
    if(chunkSize != 16) 
    {
        std::cout << "SUPPORTED FMT CHUNK SIZE DOES NOT MATCH WAV" << std::endl;
        return false;
    }
    fread(&audioFormat, 2, 1, wav);
    if(audioFormat != 1)
    {
        std::cout << "WAV Audio format not PCM" << std::endl;
    }
    fread(&numChannels, 2, 1, wav);
    fread(&sampleRate, 4, 1, wav);
    fread(&bytesPerSec, 4, 1, wav);
    fread(&bytesPerBlock, 2, 1, wav);
    fread(&bitsPerSample, 2, 1, wav);
    fread(header, 1, 4, wav);
    if(memcmp(header, LIST, sizeof(LIST)) == 0)
    {
        fread(&dataSize, 4, 1, wav);
        fseek(wav, dataSize, SEEK_CUR);
        fread(header, 1, 4, wav);
    }
    if(memcmp(header, dataHeader, sizeof(dataHeader)) != 0)
    {
        std::cout << "WAV data chunk not found" << std::endl;
        return false;
    }
    fread(&dataSize, 4, 1, wav);
    if(bitsPerSample == 8) read8Bit(wav);
    if(bitsPerSample == 16) read16Bit(wav);
    if(bitsPerSample == 24) read24Bit(wav);
    if(bitsPerSample == 32) read32Bit(wav);
    fclose(wav);
    return true;
}

void wavFile::read8Bit(FILE* wav)
{
    allocated[0] = true;
    bits8Data = (unsigned char*)malloc(dataSize);
    fread(bits8Data, 1, dataSize, wav);
}

void wavFile::read16Bit(FILE* wav)
{
    allocated[1] = true;
    int count = dataSize / ((int32_t)bitsPerSample / 8);
    bits16Data = (int16_t*)malloc(count * sizeof(int16_t));
    fread(bits16Data, 2, count, wav);
}

void wavFile::read24Bit(FILE* wav)
{
    allocated[2] = true;
    int count = dataSize / ((int32_t)bitsPerSample / 8);
    unsigned char bytes[3];
    bits32Data = (int32_t*)malloc(count * sizeof(int32_t));
    for(int i = 0; i < count; i++)
    {
        fread(bytes, 1, 3, wav);
        if((bytes[2] >> 7) & 1)
        {
            *(bits32Data + i) = (0xFF << 24) | (bytes[2] << 16) | (bytes[1] << 8) | bytes[0]; 
            *(bits32Data + i) = -(~*(bits32Data + i) + 1);
        }
        else *(bits32Data + i) = (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];
    }
}

void wavFile::read32Bit(FILE* wav)
{
    allocated[2] = true;
    int count = dataSize / ((int32_t)bitsPerSample / 8);
    bits32Data = (int32_t*)malloc(count * sizeof(int32_t));
    fread(bits32Data, 4, count, wav);
}

void wavFile::freeData()
{
    if(allocated[0]) free(bits8Data);
    if(allocated[1]) free(bits16Data);
    if(allocated[2]) free(bits32Data);
    for(int i = 0; i < 3; i++) allocated[i] = false;
}

float wavFile::getData(int frame, int channel)
{
    if(bitsPerSample == 8) return (int)(*(bits8Data + frame * numChannels + channel)) * engineConstants::normalize8Bit;
    if(bitsPerSample == 16) return (int)(*(bits16Data + frame * numChannels + channel)) * engineConstants::normalize16Bit;
    if(bitsPerSample == 24) return *(bits32Data + frame * numChannels + channel) * engineConstants::normalize24Bit;
    if(bitsPerSample == 32) return *(bits32Data + frame * numChannels + channel) * engineConstants::normalize32Bit;
    return 0;
}

void wavFile::resample(int target)
{
    if(target == sampleRate) return;

}

wavFile::~wavFile()
{
    std::cout << "data erased" << std::endl;
    freeData();
}