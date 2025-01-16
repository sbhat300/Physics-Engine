#include "Sound/audioPlayer.h"
#include <iostream>
#include <Engine/setup.h>
#include <thread>
#include <atomic>
#include <FileLoader/wavFile.h>
#include <mathFuncs.h>
#include <Sound/audioClip.h>
#include <Sound/audioManager.h>

audioLinkedList audioPlayer::audios = audioLinkedList();

int audioPlayer::start()
{
    audioManager::addClip("bgm", "Gusty Garden Galaxy");

    int err = 0;
    SoundIo* soundIo = nullptr;
    int defaultOutDeviceIndex = -1;
    SoundIoDevice* outDevice = nullptr;
    SoundIoOutStream* outStream = nullptr;
    soundIo = soundio_create();
    if (!soundIo) {
        std::cout << "SoundIO out of memory" << std::endl;
        return 1;
    }
    err = soundio_connect(soundIo);
    if(err)
    {
        std::cout << "SoundIO error connecting " << soundio_strerror(err) << std::endl;
        return 1;
    }
    soundio_flush_events(soundIo);
    defaultOutDeviceIndex = soundio_default_output_device_index(soundIo);
    if(defaultOutDeviceIndex < 0)
    {
        std::cout << "SoundIO no output device found" << std::endl;
        return 1;
    }
    outDevice = soundio_get_output_device(soundIo, defaultOutDeviceIndex);
    if(!outDevice)
    {
        std::cout << "SoundIO out of memory" << std::endl;
        return 1;
    }
    std::cout << "SoundIO Audio output device: " << outDevice->name << std::endl;
    outStream = soundio_outstream_create(outDevice);
    outStream->format = SoundIoFormatFloat32LE;
    outStream->write_callback = writeCallback;

    audioClip bgm(&audioManager::clips["bgm"]);
    outStream->userdata = (void*)&bgm;

    err = soundio_outstream_open(outStream);
    if(err)
    {
        std::cout << "SoundIO unable to open device: "  << soundio_strerror(err) << std::endl;
        return 1;
    }
    err = soundio_outstream_start(outStream);
    if(err)
    {
        std::cout << "SoundIO unable to start device: " << soundio_strerror(err) << std::endl;
        return 1;
    }
    while(!setup::shouldWindowClose.load())
    {
        //Linked list contains audio clips, they are a copy of audio clip of the entity that called it, but point to the same wavFile object
        //Create a copy of the linked list that is sent to the writeCallback when it is ready, update the callback when new audio clips
        //are added to the original linked list
        //Return an audio ID, which can be used to remove audio clips from the original linked list or change its properties
        //Remove audio clips in the writeCallback if it is done playing, and then sync that with the original linked list
    }
    soundio_outstream_destroy(outStream);
    soundio_device_unref(outDevice);
    soundio_destroy(soundIo);
    audioManager::destroy();
    audios.destroy();
    return 0;
}

void audioPlayer::writeCallback(SoundIoOutStream *outStream, int frameCountMin, int frameCountMax)
{
    //Read user input
    audioClip* playData = (audioClip*)outStream->userdata;

    //Gets number of channels and the channel ids
    const SoundIoChannelLayout* layout = &outStream->layout;
    int sampleRate = outStream->sample_rate;
    float secondPerFrame = 1.0f / sampleRate;
    int framesLeft = frameCountMax;

    //Base pointer(s) of channel buffer(s) and bytes per sample
    SoundIoChannelArea* areas;

    float frameStep = playData->audio->sampleRate / (float)outStream->sample_rate;

    int err;
    while(framesLeft > 0) //You are responsible for meeting the frame count requirement
    {
        int frameCount = framesLeft;
        //Get the number of frames allowed to write and pointer to write to
        err = soundio_outstream_begin_write(outStream, &areas, &frameCount); 
        if(err)
        {
            std::cout << "SoundIO begin write error: " << soundio_strerror(err) << std::endl;
            exit(1);
        }
        if (!frameCount)
            break;

        //Write data for each frame in each channel
        for(int frame = 0; frame < frameCount; frame++)
        {
            for(int channel = 0; channel < layout->channel_count; channel++)
            {
                //Get pointer to position in buffer to write to
                //Change move char pointer certain amount of bytes, then cast it to a float pointer
                //as we are writing a float
                float* ptr = (float*)(areas[channel].ptr + areas[channel].step * frame);
                if(channel >= playData->audio->numChannels) 
                {
                    *ptr = 0.0f;
                    continue;
                }
                float v1 = playData->audio->getData(playData->framesPlayed + frame * frameStep, channel);
                float v2 = playData->audio->getData(playData->framesPlayed + 1 + frame * frameStep, channel);
                float temp = playData->framesPlayed + frame * frameStep;
                float sample = mathFuncs::interpolate(v1, v2, temp - floor(temp));
                *ptr = sample;
            }
        }
        playData->framesPlayed += frameCount * frameStep;

        //Finish writing to buffer
        err = soundio_outstream_end_write(outStream);
        if(err)
        {
            std::cout << "SoundIO end write error: " << soundio_strerror(err) << std::endl;
            exit(1);
        }
        framesLeft -= frameCount;
    }
}