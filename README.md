![alt text](https://github.com/z-e-chan/RedFish/blob/master/redfish.PNG?raw=true)

# About
RedFish is an audio engine designed for real-time applications. Written in C++, it is intended to be used on a dedicated audio thread. RedFish does not allocate or use mutexes during runtime. Instead, it preallocates everything it needs during construction.

RedFish offers the ability to modify the amount of data being allocated using the defines found in `src/redfish/defines.h`. Additionally, you can provide your own custom allocator and deallocator.

RedFish’s mission is to provide an easy to use audio engine that supports a variety of features to enrich your application’s audio experience while being performant enough to support real-time usage in applications such as game development.

RedFish is licensed under the MIT license.

# Requirements
- At this time, RedFish only supports stereo (2 channels) playback.
- RedFish does not communicate with the hardware layer. Your project must provide an audio callback.
- C++ 17

# Features
- Supports both WAV and FLAC files.
- Interactive music supporting both layer mixing and musically-synced transitions.
- Sound variation playback with Sound Effects.
- Mixing with mix groups, output routing, and sends.
- A variety of plug-ins including:
  - Gain
  - Pan
  - Positioning
  - Delay
  - Convolution
  - IIR2 HP/LP Filters
  - Butterworth HP/LP Filters

# Editor

RedFish offers an editor that allows you to mix audio in real time. You can use the editor to save a JSON representation of the mixer, which can then be deserialized. Currently, the convolver plugin does not support serialization. Therefore, any convolvers must be created on a mix group after deserialization.

# Integration

RedFish is simple to integrate into your project. Everything you need is in the `src` folder. Just download the latest release and put the contents of the `src` somewhere accessible to your project.

For simplicity, you can just include `src/redfish/redfishapi.h` where ever you need to use RedFish.

The editor's source code lives in `editor` folder.

Note that RedFish and the editor does include external code which is covered in the next section.

# External Code

[concurrentqueue](https://github.com/cameron314/concurrentqueue)

[dr_libs](https://github.com/mackron/dr_libs)

[FFTConvolver](https://github.com/HiFi-LoFi/FFTConvolver)

[JSON for Modern C++](https://github.com/nlohmann/json)

Editor:

[nativefiledialog](https://github.com/mlabbe/nativefiledialog)

[Dear ImGui](https://github.com/ocornut/imgui)

# Usage

RedFish is simple to use! To see a demonstation of it's usage, take a look at `testbench/redfishdemo.cpp`.
In the meantime, here is an example of what it is like working the RedFish.

**Create the RedFish Context and AudioCallback**
```cpp

void LockAudioDevice()
{
    // Implementation specific way of locking your audio device.
}

void UnlockAudioDevice()
{
    // Implementation specific way of unlocking your audio device.
}

// The buffer size of the callback. This is the number of audio frames to be processed in an audio callback.
const int bufferSize = 1024;
// Currently, RedFish only offically supports 2 channels.
const int numChannels = 2;
// The sample rate of your project.
const int sampleRate = 48000;

rf::Config config(bufferSize, numChannles, sampleRate, LockAudioDevice, UnlockAudioDevice);

// If you want, you can provide a custom allocator and deallocator.
config.m_onAllocate = [](size_t numBytes, const char* name, int alignment) {
    void* data = _aligned_malloc(numBytes, alignment);
    return data;
};
config.m_onDeallocate = [](void* data) {
    _aligned_free(data);
};

// Now that the config has been made, create the rf::Context and rf::AudioCallback.
rf::Context* context = new rf::Context(config);
rf::AudioCallback* callback = new rf::AudioCallback(m_context);

// If you want to use the editor, allocate one!
rf::Editor* editor = new rf::Editor(m_context);

// Update rf::Context in your application's update/ticking function.
context->Update();

// Have an editor? Update it after the context.
editor->Update();

// Update rf::AudioCallback in your application's audio callback
// buffer is a float* that RedFish will fill with float samples to play this callback.
// bufferSize is how many audio frames are required this callback.
callback->Update(buffer, bufferSize);
```

**Load and Unload a Audio Files**

```cpp
// Load
rf::AssetSystem* assetSystem = m_context->GetAssetSystem();
const rf::AudioHandle audioHandle = assetSystem->Load("../testbench/testdata/a2-tile-land-001.wav");

// Unload
assetSystem->Unload(audioHandle);
```

**Create a Mix Group**

```cpp
rf::System* mixerSystem = context->GetMixerSystem();
rf::MixGroup* mixGroupSFX = mixerSystem->CreateMixGroup();
```

**Working With Sound Effects**

```cpp
rf::SoundEffect sfx = rf::SoundEffect(context);
sfx.SetMixGroup(mixGroupSFX);
sfx.AddVariation(audioHandle);
sfx.Play();
```

**Working With Music**
```cpp
// Load some audio
rf::AssetSystem* assetSystem = m_context->GetAssetSystem();
const rf::AudioHandle audioHandle = assetSystem->Load("BasicSong_Intro.flac");

// Create a music cue
rf::CueParameters cueParams;
cueParams.m_meter = rf::Meter(4, 4);
cueParams.m_tempo = 130.0f;
cueParams.m_name = "BasicSong_Intro";
cueParams.AddLayer(audioHandle, mixGroupMusic);
rf::Cue* cue = musicSytem->CreateCue(cueParams);

// Create a transition to play the cue
rf::TransitionParameters transParams;
transParams.m_cue = cue;
transParams.m_name = "To_BasicSong_Intro";
rf::Transition* transition = musicSytem->CreateTransition(transParams);

// Play the transistion
rf::MusicSystem* musicSystem = context->GetMusicSystem();
musicSystem->Play(transition);
```

**After Deserialization**
```cpp
m_context->Deserialize("redfish.json");

// After deserialization, associate mix groupss with mix group pointers.
rf::MixerSystem* mixerSystem = context->GetMixerSystem();
rf::MixGroup* mixGroupEntities = mixerSystem->GetMixGroup("Entities");
rf::MixGroup* mixGroupAmbience = mixerSystem->GetMixGroup("Ambience");
rf::MixGroup* mixGroupReverb = mixerSystem->GetMixGroup("Reverb");
rf::MixGroup* mixGroupDelay = mixerSystem->GetMixGroup("Delay");
rf::MixGroup* mixGroupLeads = mixerSystem->GetMixGroup("Leads");
rf::MixGroup* mixGroupBass = mixerSystem->GetMixGroup("Bass");
rf::MixGroup* mixGroupDrums = mixerSystem->GetMixGroup("Drums");
rf::MixGroup* mixGroupMusic = mixerSystem->GetMixGroup("Music");
rf::MixGroup* mixGroupSounds = mixerSystem->GetMixGroup("Sounds");
rf::MixGroup* mixGroupMaster = mixerSystem->GetMasterMixGroup();

// After deserialization, associate plugins with plugin pointers.
rf::IIR2HighpassFilterPlugin* iir2HighpassEntities = mixGroupEntities->GetPlugin<rf::IIR2HighpassFilterPlugin>(0);
rf::IIR2LowpassFilterPlugin* iir2LowpassEntities = mixGroupEntities->GetPlugin<rf::IIR2LowpassFilterPlugin>(1);
rf::CompressorPlugin* compressorEntities = mixGroupEntities->GetPlugin<rf::CompressorPlugin>(2);
rf::PositioningPlugin* positioningEntities = mixGroupEntities->GetPlugin<rf::PositioningPlugin>(3);
rf::GainPlugin* = gainAmbience = mixGroupAmbience->GetPlugin<rf::GainPlugin>(0);
rf::PanPlugin* panAmbience = mixGroupAmbience->GetPlugin<rf::PanPlugin>(1);
rf::ButterworthHighpassFilterPlugin* bwHighpassAmbience = mixGroupAmbience->GetPlugin<rf::ButterworthHighpassFilterPlugin>(2);
rf::ButterworthLowpassFilterPlugin* bwLowpassAmbience = mixGroupAmbience->GetPlugin<rf::ButterworthLowpassFilterPlugin>(3);
rf::DelayPlugin* = delayDelay = mixGroupDelay->GetPlugin<rf::DelayPlugin>(0);
rf::LimiterPlugin* limiterMaster = mixGroupMaster->GetPlugin<rf::LimiterPlugin>(0);

// After deserialization, create a convolver. At this time, convolvers do not support serialization.
rf::ConvolverPlugin* convolverReverb = m_mixGroupReverb->CreatePlugin<rf::ConvolverPlugin>();
```

# Find a Bug?
Feel free to report it and/or create an issue. RedFish is being actively developed and my goal is to fix all bugs and add features that make this project more useful.
