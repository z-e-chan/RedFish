#pragma once

#include <functional>

namespace Platform
{
void Initialize();
void CreateAudioDevice(void (*callback)(void* userdata, uint8_t* stream, int len), int sampleRate, int bufferSize, int channels);
void DestroyAudioDevice();
void LockAudioDevice();
void UnlockAudioDevice();
void Tick(const std::function<void(float dt)>& callback);

}  // namespace Platform