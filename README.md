# About
RedFish is an audio engine written in C++ that has been designed for use in real-time applications. As such, RedFish is intended to be used on a dedicated audio thread; and, RedFish does not allocate or use mutexes during its runtime.

Because RedFish does not allocate during it's runtime, it preallocates everything it needs during construction.
You can modify the amount of data being allocated using the defines in `src/redfish/defines.h`.

Additionally, you can provide RedFish with custom a custom allocator and dealloctor if you wish.

RedFish’s mission is to provide an easy to use audio engine that supports a variety of features to enrich your application’s audio experience while being performant enough to support real-time usage.

RedFish is licensed under the MIT license.

# Requirements
- At this time, RedFish only support stereo (2 channels) projects and does not support any form of surround sound.
- RedFish does not communicate with the hardware layer. Your project must provide an audio callback.
- C++ 17

# Features
- Supports both WAV and FLAC files.
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
- Interactive music supporting both layer mixing and musically-synced transitions.

# Integration

RedFish is simple to integrate into your project. Everything you need is in the `src` folder. Just download the latest release and put the contents of the `src` somewhere accessible to your project.

Note that RedFish does include external code which is covered in the next section.

# External Code

[concurrentqueue](https://github.com/cameron314/concurrentqueue)

[dr_libs](https://github.com/mackron/dr_libs)

[FFTConvolver](https://github.com/HiFi-LoFi/FFTConvolver)

If you wish to compile `testbench/redfishdemo.cpp`, you will need `Dear ImGui`.

[Dear ImGui](https://github.com/ocornut/imgui)

# Usage

RedFish is simple to use! To see a demonstation of it's usage, take a look at `testbench/redfishdemo.cpp`.
In the meantime, here is an example of what it is like working the RedFish.

**Create the RedFish Context and Callback**
```cpp
rf::Config config(bufferSize, numChannles, sampleRate, lockAudioDevice, unlockAudioDevice);
rf::Context* context = new rf::Context(config);
rf::AudioCallback* callback = new rf::AudioCallback(m_context);

// Update rf::Context in your application's update/ticking function.
context->Update();

// Update rf::AudioCallback in your application's audio callback
// buffer is a float* that RedFish will fill with float samples to play this callback.
// bufferSize is how many audio frames are required this callback.
callback->Update(buffer, bufferSize);
```

**Load a WAV File**

```cpp
rf::AssetSystem* assetSystem = m_context->GetAssetSystem();
const rf::AudioHandle audioHandle = assetSystem->Load("../testbench/testdata/a2-tile-land-001.wav");
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
// Create a music cue
rf::CueParameters cueParams;
cueParams.m_meter = rf::Meter(4, 4);
cueParams.m_tempo = 130.0f;
cueParams.m_name = "BasicSong_Intro";
cueParams.AddLayer(audioHandleBasicSongIntro, mixGroupMusic);
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
# Find a Bug?
Feel free to report it and/or create an issue. RedFish is being actively developed and my goal is to fix all bugs and add features that make this project more useful.
