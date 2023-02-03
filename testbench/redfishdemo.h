#pragma once
#include <redfish/redfishapi.h>

class RedFishDemo
{
public:
    RedFishDemo(int bufferSize, int numChannles, int sampleRate, void (*lockAudioDevice)(), void (*unlockAudioDevice)());
    ~RedFishDemo();

    void OnAudioCallback(float* buffer, int bufferSize);
    void OnApplicationUpdate(float dt);

    void Example1_Construction(int bufferSize, int numChannles, int sampleRate, void (*lockAudioDevice)(), void (*unlockAudioDevice)());
    void Example2_Destruction();
    void Example3_LoadingAudioAssets();
    void Example4_CreateMixGroups();
    void Example5_CreateSoundEffects();
    void Example6_PlaySoundEffects();
    void Example7_MixGroups();
    void Example8_ViewPlayingSounds();
    void Example9_Music();

    void StressTest();
    void FuzzTest();

    void Mixer();

private:
    void (*m_lockAudioDevice)() = nullptr;
    void (*m_unlockAudioDevice)() = nullptr;

    rf::Context* m_context = nullptr;
    rf::AudioCallback* m_callback = nullptr;

    rf::AudioHandle m_assetFootstep1;
    rf::AudioHandle m_assetFootstep2;
    rf::AudioHandle m_assetFootstep3;
    rf::AudioHandle m_assetFootstep4;
    rf::AudioHandle m_assetFootstep5;
    rf::AudioHandle m_assetFootstep6;

    rf::AudioHandle m_assetBirdLoop;

    rf::AudioHandle m_assetIRSmall;
    rf::AudioHandle m_assetIRMedium;
    rf::AudioHandle m_assetIRLarge;

    rf::AudioHandle m_assetBasicSongIntro;
    rf::AudioHandle m_assetBasicSongLoop;

    rf::AudioHandle m_assetSong1Cue10Bass;
    rf::AudioHandle m_assetSong1Cue10Drums;
    rf::AudioHandle m_assetSong1Cue10Lead;
    rf::AudioHandle m_assetSong1Cue11Bass;
    rf::AudioHandle m_assetSong1Cue11Drums;
    rf::AudioHandle m_assetSong1Cue11Lead;
    rf::AudioHandle m_assetSong1Cue1Drums;
    rf::AudioHandle m_assetSong1Cue2Drums;
    rf::AudioHandle m_assetSong1Cue2Lead;
    rf::AudioHandle m_assetSong1Cue3Bass;
    rf::AudioHandle m_assetSong1Cue3Drums;
    rf::AudioHandle m_assetSong1Cue3Lead;
    rf::AudioHandle m_assetSong1Cue4Bass;
    rf::AudioHandle m_assetSong1Cue4Drums;
    rf::AudioHandle m_assetSong1Cue4Lead;
    rf::AudioHandle m_assetSong1Cue5Drums;
    rf::AudioHandle m_assetSong1Cue5Lead;
    rf::AudioHandle m_assetSong1Cue6Bass;
    rf::AudioHandle m_assetSong1Cue6Drums;
    rf::AudioHandle m_assetSong1Cue6Lead;
    rf::AudioHandle m_assetSong1Cue7Bass;
    rf::AudioHandle m_assetSong1Cue7Drums;
    rf::AudioHandle m_assetSong1Cue8Bass;
    rf::AudioHandle m_assetSong1Cue8Drums;
    rf::AudioHandle m_assetSong1Cue8Lead;
    rf::AudioHandle m_assetSong1Cue9Bass;
    rf::AudioHandle m_assetSong1Cue9Drums;
    rf::AudioHandle m_assetSong1Cue9Lead;
    rf::AudioHandle m_assetSong1Stinger;

    rf::AudioHandle m_assetSong2Lead;
    rf::AudioHandle m_assetSong2Bass;
    rf::AudioHandle m_assetSong2Drums;

    rf::AudioHandle m_assetSong3Intro;
    rf::AudioHandle m_assetSong3Loop;

    rf::AudioHandle m_assetSong4Cue;
    rf::AudioHandle m_assetSong4Exit1;
    rf::AudioHandle m_assetSong4Exit2;

    rf::MixGroup* m_mixGroupEntities = nullptr;
    rf::MixGroup* m_mixGroupAmbience = nullptr;
    rf::MixGroup* m_mixGroupReverb = nullptr;
    rf::MixGroup* m_mixGroupDelay = nullptr;
    rf::MixGroup* m_mixGroupSounds = nullptr;
    rf::MixGroup* m_mixGroupLeads = nullptr;
    rf::MixGroup* m_mixGroupBass = nullptr;
    rf::MixGroup* m_mixGroupDrums = nullptr;
    rf::MixGroup* m_mixGroupMusic = nullptr;
    rf::MixGroup* m_mixGroupTestSendTo = nullptr;
    rf::MixGroup* m_mixGroupMaster = nullptr;

    rf::SoundEffect m_soundEffectFootsteps;
    rf::SoundEffect m_soundEffectBirdLoop;

    rf::Send* m_sendEntitiesToReverb = nullptr;
    rf::Send* m_sendEntitiesToDelay = nullptr;
    rf::Send* m_sendTestCreateDestroy = nullptr;

    rf::GainPlugin* m_gainAmbience = nullptr;
    rf::PanPlugin* m_panAmbience = nullptr;
    rf::ButterworthHighpassFilterPlugin* m_bwHighpassAmbience = nullptr;
    rf::ButterworthLowpassFilterPlugin* m_bwLowpassAmbience = nullptr;
    rf::IIR2HighpassFilterPlugin* m_iir2HighpassEntities = nullptr;
    rf::IIR2LowpassFilterPlugin* m_iir2LowpassEntities = nullptr;
    rf::CompressorPlugin* m_compressorEntities = nullptr;
    rf::DelayPlugin* m_delayDelay = nullptr;
    rf::LimiterPlugin* m_limiterMaster = nullptr;
    rf::ConvolverPlugin* m_convolverReverb = nullptr;
    rf::PositioningPlugin* m_positioningEntities = nullptr;

    enum class FuzzTestState
    {
        Wait,
        Sound,
        Music,
        ForceDestroy,
    } m_fuzzTestState = FuzzTestState::Wait;

    float m_stressTestAccumulator = 0.0f;
    float m_fuzzTestWaitTime = 0.0f;
    bool m_runStressTest = false;
    bool m_runFuzzTest = false;

    void RunStressTest(float dt);
    void RunFuzzTest(float dt);
};