#include <unordered_map>

#include "platform.hpp"
#include "redfishdemo.h"

static RedFishDemo* s_demo = nullptr;
static constexpr int s_sampleRate = 48000;
static constexpr int s_bufferSize = 1024;
static constexpr int s_channels = 2;

void AudioCallback(void*, uint8_t* stream, int)
{
    float* samples = reinterpret_cast<float*>(stream);
    s_demo->OnAudioCallback(samples, s_bufferSize);
}

void TickCallback(float dt)
{
    s_demo->OnApplicationUpdate(dt);
}

int main(int, char**)
{
    Platform::Initialize();
    s_demo = new RedFishDemo(s_bufferSize, s_channels, s_sampleRate, Platform::LockAudioDevice, Platform::UnlockAudioDevice);
    Platform::CreateAudioDevice(AudioCallback, s_sampleRate, s_bufferSize, s_channels);
    Platform::Tick(TickCallback);
    delete s_demo;
    Platform::DestroyAudioDevice();
    return 0;
}