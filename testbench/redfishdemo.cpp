#include "redfishdemo.h"

#include <imgui.h>

#include <unordered_map>

static std::unordered_map<void*, const char*>* s_allocationMap = nullptr;

RedFishDemo::RedFishDemo(int bufferSize, int numChannles, int sampleRate, void (*lockAudioDevice)(), void (*unlockAudioDevice)())
    : m_lockAudioDevice(lockAudioDevice)
    , m_unlockAudioDevice(unlockAudioDevice)
{
    s_allocationMap = new std::unordered_map<void*, const char*>;
    Example1_Construction(bufferSize, numChannles, sampleRate, lockAudioDevice, unlockAudioDevice);
    Example3_LoadingAudioAssets();
    Example4_CreateMixGroups();
    Example5_CreateSoundEffects();
    Example10_Events();
}

RedFishDemo::~RedFishDemo()
{
    Example2_Destruction();

    if (!s_allocationMap->empty())
    {
        // RedFish memory leak detected!
        assert(false);
    }

    delete s_allocationMap;
}

void RedFishDemo::OnAudioCallback(float* buffer, int bufferSize)
{
    // Call rf::AudioCallback::Update on the audio thread, or where ever your audio callback is.
    if (m_callback)
    {
        m_callback->Update(buffer, bufferSize);
    }
}

void RedFishDemo::OnApplicationUpdate(float dt)
{
    m_context->Update();
    m_editor->Update();

    const rf::Version& version = rf::GetVersion();
    char buffer[64];
    sprintf_s(buffer, "RedFish Demo v%i.%i.%i", version.m_major, version.m_minor, version.m_patch);
    ImGui::Begin(buffer);

    Example6_PlaySoundEffects();
    Example7_MixGroups();
    Example9_Music();
    Mixer();
    StressTest();
    FuzzTest();

    if (ImGui::CollapsingHeader("Events"))
    {
        ImGui::Checkbox("Print Events to Console", &m_printEventsToConsole);
    }

    Example8_ViewPlayingSounds();
    ImGui::End();

    RunStressTest(dt);
    RunFuzzTest(dt);
}

void RedFishDemo::Example1_Construction(int bufferSize, int numChannles, int sampleRate, void (*lockAudioDevice)(), void (*unlockAudioDevice)())
{
    // Fill out a rf::Config with the appropriate data and pass it into the construct of rf::Context.
    // rf::Config requires the following data:
    // - bufferSize: the buffer size of the your audio callback. That is, how many audio frames need to be processed each callback.
    // - numChannels: the number of audio channels your project supports. RedFish only currently supports 2 channels (stereo).
    // - sampleRate: the sample rate of the project.
    // - lockAudioDevice: a callback to lock your audio callback.
    // - unlockAudioDevice: a callback to unlock your audio callback.
    // Optionally, you can provide custom allocation/deallocation callbacks.
    // You may want to do this if you wish to track RedFish memory allocations.
    // An example is shown below of providing custom allocation/deallocation callbacks.

    rf::Config config(bufferSize, numChannles, sampleRate, lockAudioDevice, unlockAudioDevice);
    config.m_onAllocate = [](size_t numBytes, const char* name, int alignment) {
        void* data = _aligned_malloc(numBytes, alignment);
        (*s_allocationMap)[data] = name;
        return data;
    };
    config.m_onDeallocate = [](void* data) {
        (*s_allocationMap).erase(data);
        _aligned_free(data);
    };

    m_context = new rf::Context(config);

    // Create a rf::AudioCallback using the context we just made.

    m_callback = new rf::AudioCallback(m_context);

    m_editor = new rf::Editor(m_context);
}

void RedFishDemo::Example2_Destruction()
{
    // Delete the the rf::Context and rf::AudioCallback when you are done with it.
    delete m_context;
    // You may wish to lock the audio device before destroying the callback. This will prevent the audio thread from trying to call into the callback
    // which in the process of being destroyed.
    m_lockAudioDevice();
    delete m_callback;
    m_callback = nullptr;
    m_unlockAudioDevice();

    delete m_editor;
}

void RedFishDemo::Example3_LoadingAudioAssets()
{
    // You can load audio files directly by calling rf::Context::Load

    rf::AssetSystem* assetSystem = m_context->GetAssetSystem();

    m_assetFootstep1 = assetSystem->Load("../testbench/testdata/a2-tile-land-001.wav");
    m_assetFootstep2 = assetSystem->Load("../testbench/testdata/a2-tile-land-002.wav");
    m_assetFootstep3 = assetSystem->Load("../testbench/testdata/a2-tile-land-003.wav");
    m_assetFootstep4 = assetSystem->Load("../testbench/testdata/a2-tile-land-004.wav");
    m_assetFootstep5 = assetSystem->Load("../testbench/testdata/a2-tile-land-005.wav");
    m_assetFootstep6 = assetSystem->Load("../testbench/testdata/a2-tile-land-006.wav");

    m_assetBirdLoop = assetSystem->Load("../testbench/testdata/bird_loop.wav");
    const rf::AudioHandle duplicateTest = assetSystem->Load("../testbench/testdata/bird_loop.wav");
    // Loading an audio file with the same path will return the same audio handle
    assert(m_assetBirdLoop == duplicateTest);

    m_assetIRSmall = assetSystem->Load("../testbench/testdata/ir/reverb_small.wav");
    m_assetIRMedium = assetSystem->Load("../testbench/testdata/ir/reverb_medium.wav");
    m_assetIRLarge = assetSystem->Load("../testbench/testdata/ir/reverb_large.wav");

    m_assetBasicSongIntro = assetSystem->Load("../testbench/testdata/basic_song/intro.wav");
    m_assetBasicSongLoop = assetSystem->Load("../testbench/testdata/basic_song/loop.wav");

    m_assetSong1Cue10Bass = assetSystem->Load("../testbench/testdata/song1/song1_cue10_bass.wav");
    m_assetSong1Cue10Drums = assetSystem->Load("../testbench/testdata/song1/song1_cue10_drums.wav");
    m_assetSong1Cue10Lead = assetSystem->Load("../testbench/testdata/song1/song1_cue10_lead.wav");
    m_assetSong1Cue11Bass = assetSystem->Load("../testbench/testdata/song1/song1_cue11_bass.wav");
    m_assetSong1Cue11Drums = assetSystem->Load("../testbench/testdata/song1/song1_cue11_drums.wav");
    m_assetSong1Cue11Lead = assetSystem->Load("../testbench/testdata/song1/song1_cue11_lead.wav");
    m_assetSong1Cue1Drums = assetSystem->Load("../testbench/testdata/song1/song1_cue1_drums.wav");
    m_assetSong1Cue2Drums = assetSystem->Load("../testbench/testdata/song1/song1_cue2_drums.wav");
    m_assetSong1Cue2Lead = assetSystem->Load("../testbench/testdata/song1/song1_cue2_lead.wav");
    m_assetSong1Cue3Bass = assetSystem->Load("../testbench/testdata/song1/song1_cue3_bass.wav");
    m_assetSong1Cue3Drums = assetSystem->Load("../testbench/testdata/song1/song1_cue3_drums.wav");
    m_assetSong1Cue3Lead = assetSystem->Load("../testbench/testdata/song1/song1_cue3_lead.wav");
    m_assetSong1Cue4Bass = assetSystem->Load("../testbench/testdata/song1/song1_cue4_bass.wav");
    m_assetSong1Cue4Drums = assetSystem->Load("../testbench/testdata/song1/song1_cue4_drums.wav");
    m_assetSong1Cue4Lead = assetSystem->Load("../testbench/testdata/song1/song1_cue4_lead.wav");
    m_assetSong1Cue5Drums = assetSystem->Load("../testbench/testdata/song1/song1_cue5_drums.wav");
    m_assetSong1Cue5Lead = assetSystem->Load("../testbench/testdata/song1/song1_cue5_lead.wav");
    m_assetSong1Cue6Bass = assetSystem->Load("../testbench/testdata/song1/song1_cue6_bass.wav");
    m_assetSong1Cue6Drums = assetSystem->Load("../testbench/testdata/song1/song1_cue6_drums.wav");
    m_assetSong1Cue6Lead = assetSystem->Load("../testbench/testdata/song1/song1_cue6_lead.wav");
    m_assetSong1Cue7Bass = assetSystem->Load("../testbench/testdata/song1/song1_cue7_bass.wav");
    m_assetSong1Cue7Drums = assetSystem->Load("../testbench/testdata/song1/song1_cue7_drums.wav");
    m_assetSong1Cue8Bass = assetSystem->Load("../testbench/testdata/song1/song1_cue8_bass.wav");
    m_assetSong1Cue8Drums = assetSystem->Load("../testbench/testdata/song1/song1_cue8_drums.wav");
    m_assetSong1Cue8Lead = assetSystem->Load("../testbench/testdata/song1/song1_cue8_lead.wav");
    m_assetSong1Cue9Bass = assetSystem->Load("../testbench/testdata/song1/song1_cue9_bass.wav");
    m_assetSong1Cue9Drums = assetSystem->Load("../testbench/testdata/song1/song1_cue9_drums.wav");
    m_assetSong1Cue9Lead = assetSystem->Load("../testbench/testdata/song1/song1_cue9_lead.wav");
    m_assetSong1Stinger = assetSystem->Load("../testbench/testdata/song1/song1_stinger.wav");

    m_assetSong2Lead = assetSystem->Load("../testbench/testdata/song2/song2_lead.wav");
    m_assetSong2Bass = assetSystem->Load("../testbench/testdata/song2/song2_bass.wav");
    m_assetSong2Drums = assetSystem->Load("../testbench/testdata/song2/song2_drums.wav");

    m_assetSong4Cue = assetSystem->Load("../testbench/testdata/song4/song4_cue.flac");
    m_assetSong4Exit1 = assetSystem->Load("../testbench/testdata/song4/song4_exit1.flac");
    m_assetSong4Exit2 = assetSystem->Load("../testbench/testdata/song4/song4_exit2.flac");
}

void RedFishDemo::Example4_CreateMixGroups()
{
    // MixGroups allow you to route audio playback into groups which you can then mix.
    // RedFish makes a master mix group automatically.
    // If you do not assign a mix group to a sound, it will be sent to the master mix group.

    rf::MixerSystem* mixerSystem = m_context->GetMixerSystem();

    m_mixGroupEntities = mixerSystem->CreateMixGroup("Entities");
    m_mixGroupAmbience = mixerSystem->CreateMixGroup("Ambience");
    m_mixGroupReverb = mixerSystem->CreateMixGroup("Reverb");
    m_mixGroupDelay = mixerSystem->CreateMixGroup("Delay");
    m_mixGroupLeads = mixerSystem->CreateMixGroup("Leads");
    m_mixGroupBass = mixerSystem->CreateMixGroup("Bass");
    m_mixGroupDrums = mixerSystem->CreateMixGroup("Drums");
    m_mixGroupMusic = mixerSystem->CreateMixGroup("Music");
    m_mixGroupSounds = mixerSystem->CreateMixGroup("Sounds");
    m_mixGroupTestSendTo = mixerSystem->CreateMixGroup("Send To Test");
    m_mixGroupMaster = mixerSystem->GetMasterMixGroup();

    // Use rf::MixGroup::SetOutputMixGroup to route the signal from a mix group through another one.
    // If not set, mix groups route their signal through the master mix group.

    m_mixGroupEntities->SetOutputMixGroup(m_mixGroupSounds);
    m_mixGroupAmbience->SetOutputMixGroup(m_mixGroupSounds);
    m_mixGroupDelay->SetOutputMixGroup(m_mixGroupSounds);
    m_mixGroupReverb->SetOutputMixGroup(m_mixGroupSounds);
    m_mixGroupLeads->SetOutputMixGroup(m_mixGroupMusic);
    m_mixGroupBass->SetOutputMixGroup(m_mixGroupMusic);
    m_mixGroupDrums->SetOutputMixGroup(m_mixGroupMusic);

    // Sends allow you to route route signal(s) to a mix group.
    // Use rf::MixGroup::CreateSend to create a send from one mix group to another.
    // This can be useful if you have a reverb effect on one mix group and you want many mix groups to have some reverb.

    m_sendEntitiesToReverb = m_mixGroupEntities->CreateSend(m_mixGroupReverb);
    m_sendEntitiesToDelay = m_mixGroupEntities->CreateSend(m_mixGroupDelay);

    // Plug-ins offer you away to add some digital signal process to mix groups.
    // Use rf::MixGroup::CreatePlugin<T> to create a plug-in.

    m_gainAmbience = m_mixGroupAmbience->CreatePlugin<rf::GainPlugin>();
    m_panAmbience = m_mixGroupAmbience->CreatePlugin<rf::PanPlugin>();
    m_bwHighpassAmbience = m_mixGroupAmbience->CreatePlugin<rf::ButterworthHighpassFilterPlugin>();
    m_bwLowpassAmbience = m_mixGroupAmbience->CreatePlugin<rf::ButterworthLowpassFilterPlugin>();
    m_iir2HighpassEntities = m_mixGroupEntities->CreatePlugin<rf::IIR2HighpassFilterPlugin>();
    m_iir2LowpassEntities = m_mixGroupEntities->CreatePlugin<rf::IIR2LowpassFilterPlugin>();
    m_compressorEntities = m_mixGroupEntities->CreatePlugin<rf::CompressorPlugin>();
    m_delayDelay = m_mixGroupDelay->CreatePlugin<rf::DelayPlugin>();
    m_limiterMaster = m_mixGroupMaster->CreatePlugin<rf::LimiterPlugin>();
    m_convolverReverb = m_mixGroupReverb->CreatePlugin<rf::ConvolverPlugin>();
    m_positioningEntities = m_mixGroupEntities->CreatePlugin<rf::PositioningPlugin>();

    m_context->Deserialize("redfish.json");

    mixerSystem = m_context->GetMixerSystem();
    m_mixGroupEntities = mixerSystem->GetMixGroup("Entities");
    m_mixGroupAmbience = mixerSystem->GetMixGroup("Ambience");
    m_mixGroupReverb = mixerSystem->GetMixGroup("Reverb");
    m_mixGroupDelay = mixerSystem->GetMixGroup("Delay");
    m_mixGroupLeads = mixerSystem->GetMixGroup("Leads");
    m_mixGroupBass = mixerSystem->GetMixGroup("Bass");
    m_mixGroupDrums = mixerSystem->GetMixGroup("Drums");
    m_mixGroupMusic = mixerSystem->GetMixGroup("Music");
    m_mixGroupSounds = mixerSystem->GetMixGroup("Sounds");
    m_mixGroupTestSendTo = mixerSystem->GetMixGroup("Send To Test");
    m_mixGroupMaster = mixerSystem->GetMasterMixGroup();
}

void RedFishDemo::Example5_CreateSoundEffects()
{
    // A rf::SoundEffect contains many rf::AudioHandle assets as variations.
    // Then, when a sound effect is played, a variation is chosen.
    // Use rf::SoundEffect::SetPlaybackRule to control how variations are chosen.
    // - PlaybackRule::SmartShuffle: the least commonly played variation is chosen.
    // - PlaybackRule::RoundRobin: variations are played in order.
    // - PlaybackRule::Random: variations played at random.
    // You can use the setters on a variation to control volume/pitch randomization.
    // Each time a variation is selected a volume/pitch will be selected that is between the respective min and max values.
    // Use rf::SoundEffect::SetMixGroup to route the sound effect's playback through a mix group.

    m_soundEffectFootsteps = rf::SoundEffect(m_context);
    m_soundEffectFootsteps.SetMixGroup(m_mixGroupEntities);
    m_soundEffectFootsteps.AddVariation(m_assetFootstep1).SetMinPitch(0.75f).SetMaxPitch(1.5f).SetMinVolumeDb(-12.0f).SetMaxVolumeDb(6.0f);
    m_soundEffectFootsteps.AddVariation(m_assetFootstep2).SetMinPitch(0.75f).SetMaxPitch(1.5f).SetMinVolumeDb(-12.0f).SetMaxVolumeDb(6.0f);
    m_soundEffectFootsteps.AddVariation(m_assetFootstep3).SetMinPitch(0.75f).SetMaxPitch(1.5f).SetMinVolumeDb(-12.0f).SetMaxVolumeDb(6.0f);
    m_soundEffectFootsteps.AddVariation(m_assetFootstep4).SetMinPitch(0.75f).SetMaxPitch(1.5f).SetMinVolumeDb(-12.0f).SetMaxVolumeDb(6.0f);
    m_soundEffectFootsteps.AddVariation(m_assetFootstep5).SetMinPitch(0.75f).SetMaxPitch(1.5f).SetMinVolumeDb(-12.0f).SetMaxVolumeDb(6.0f);
    m_soundEffectFootsteps.AddVariation(m_assetFootstep6).SetMinPitch(0.75f).SetMaxPitch(1.5f).SetMinVolumeDb(-12.0f).SetMaxVolumeDb(6.0f);

    m_soundEffectBirdLoop = rf::SoundEffect(m_context);
    m_soundEffectBirdLoop.SetMixGroup(m_mixGroupAmbience);
    m_soundEffectBirdLoop.SetIsLooping(true);
    m_soundEffectBirdLoop.AddVariation(m_assetBirdLoop);
}

void RedFishDemo::Example6_PlaySoundEffects()
{
    if (ImGui::CollapsingHeader("Play Sound Effects"))
    {
        if (ImGui::TreeNode("Basic Playback"))
        {
            // Call use rf::SoundEffect::Play and rf::SoundEffect::Stop to play/stop respectively.
            const auto Demo = [](const char* name, rf::SoundEffect& sound) {
                ImGui::Text(name);
                ImGui::PushID(name);
                ImGui::SameLine();
                if (ImGui::Button("Play"))
                {
                    sound.Play();
                }
                ImGui::SameLine();
                if (ImGui::Button("Stop"))
                {
                    sound.Stop();
                }
                ImGui::PopID();
            };

            Demo("Footsteps", m_soundEffectFootsteps);
            Demo("Bird Loop", m_soundEffectBirdLoop);

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Synced Playback (Play some music first!)"))
        {
            const auto Demo = [](const char* name, rf::SoundEffect& sound) {
                ImGui::Text(name);
                ImGui::PushID(name);
                ImGui::SameLine();
                if (ImGui::Button("Play"))
                {
                    sound.Play(rf::Sync(rf::Sync::Value::Quarter));
                }
                ImGui::SameLine();
                if (ImGui::Button("Stop"))
                {
                    sound.Stop();
                }
                ImGui::PopID();
            };

            Demo("Footsteps", m_soundEffectFootsteps);

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Fading"))
        {
            ImGui::PushID("SFX Fading");
            static rf::SoundEffect s_fadingSound = m_soundEffectBirdLoop;

            // Have the sound start fade to silent and fade it in to simulate a situation where you might want to fade in a sound effect.
            if (ImGui::Button("Play (Silenced, use 'Fade To dB' and 'Fade' to control volume)"))
            {
                s_fadingSound.Play();
                s_fadingSound.Fade(-120.0f, rf::Sync(0.0f), rf::Sync(0.0f));
            }

            static float s_fadeToDb = 0.0f;
            ImGui::DragFloat("Fade To dB", &s_fadeToDb, 0.01f, -120.0f, 12.0f);
            if (ImGui::Button("Fade"))
            {
                s_fadingSound.Fade(s_fadeToDb, rf::Sync(0.0f), rf::Sync(5.0f));
            }

            if (ImGui::Button("Fade Out And Stop"))
            {
                s_fadingSound.FadeOutAndStop(rf::Sync(0.0f), rf::Sync(5.0f));
            }

            if (ImGui::Button("Stop"))
            {
                s_fadingSound.Stop();
            }

            ImGui::PopID();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Playback with Automation"))
        {
            // Copy sound effects to create instances that you can automate independently of each other.
            static rf::SoundEffect s_footstepInstance1 = m_soundEffectFootsteps;
            static rf::SoundEffect s_footstepInstance2 = m_soundEffectFootsteps;
            static rf::SoundEffect m_birdLoopInstance1 = m_soundEffectBirdLoop;

            const auto Demo = [](const char* name, rf::SoundEffect& sound) {
                ImGui::Text(name);
                ImGui::PushID(name);
                ImGui::SameLine();
                if (ImGui::Button("Play"))
                {
                    sound.Play();
                }
                ImGui::SameLine();
                if (ImGui::Button("Stop"))
                {
                    sound.Stop();
                }

                bool loop = sound.GetIsLooping();
                ImGui::Checkbox("Loop", &loop);
                sound.SetIsLooping(loop);

                float volumeDb = sound.GetVolumeDb();
                ImGui::DragFloat("Volume dB", &volumeDb, 0.01f, -120.0f, 12.0f);
                sound.SetVolumeDb(volumeDb);

                float pitch = sound.GetPitch();
                ImGui::DragFloat("Pitch", &pitch, 0.01f, 0.0f, 2.0f);
                sound.SetPitch(pitch);

                rf::PositioningParameters params = sound.GetPositioningParameters();
                if (ImGui::TreeNode("Positioning"))
                {
                    ImGui::Checkbox("Enable", &params.m_enable);
                    ImGui::DragFloat("Pan", &params.m_panAngle, 0.001f, -1.0f, 1.0f);
                    ImGui::DragFloat("Current Distance", &params.m_currentDistance, 1.0f, 0.0f, 1000.0f);
                    ImGui::DragFloat("Min Distance", &params.m_minDistance, 1.0f, 0.0f, 1000.0f);
                    ImGui::DragFloat("Max Distance", &params.m_maxDistance, 1.0f, 0.0f, 1000.0f);
                    ImGui::DragFloat("Max Attenuation dB", &params.m_maxAttenuationDb, 1.0f, -60.0f, 12.0f);
                    ImGui::DragFloat("Max HPF Cutoff", &params.m_maxHpfCutoff, 1.0f, 20.0f, 20000.0f);
                    ImGui::DragFloat("Max LPF Cutoff", &params.m_maxLpfCutoff, 1.0f, 20.0f, 20000.0f);
                    ImGui::TreePop();
                }
                sound.SetPositioningParameters(params);

                ImGui::PopID();
            };

            Demo("Footsteps 1", s_footstepInstance1);
            Demo("Footsteps 2", s_footstepInstance2);
            Demo("Bird Loop", m_birdLoopInstance1);

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Unloading During Playback"))
        {
            // It is safe to unload a rf::AudioHandle while it is playing.
            // RedFish will stop the sound before it is unloaded.

            static bool s_hasBeenLoaded = false;
            static rf::AudioHandle s_assetSineLoop;
            static rf::SoundEffect s_soundEffectSineLoop;
            rf::AssetSystem* assetSystem = m_context->GetAssetSystem();

            if (!s_hasBeenLoaded)
            {
                if (ImGui::Button("Load"))
                {
                    s_hasBeenLoaded = true;
                    s_assetSineLoop = assetSystem->Load("../testbench/testdata/sineloop.wav");
                    s_soundEffectSineLoop = rf::SoundEffect(m_context);
                    s_soundEffectSineLoop.SetIsLooping(true);
                    s_soundEffectSineLoop.AddVariation(s_assetSineLoop);
                }
            }
            else
            {
                if (ImGui::Button("Play"))
                {
                    s_soundEffectSineLoop.Play();
                }

                if (ImGui::Button("Unload"))
                {
                    assetSystem->Unload(s_assetSineLoop);
                    s_hasBeenLoaded = false;
                }
            }

            ImGui::TreePop();
        }
    }
}

void RedFishDemo::Example7_MixGroups()
{
    if (ImGui::CollapsingHeader("Mix Groups"))
    {
        if (ImGui::TreeNode("Mix Groups: Create and Destroy"))
        {
            static rf::SoundEffect s_testSound = m_soundEffectFootsteps;
            static rf::Send* s_testSend1 = nullptr;
            static rf::Send* s_testSend2 = nullptr;
            static rf::GainPlugin* s_testGain = nullptr;
            static rf::PanPlugin* s_testDelay = nullptr;

            if (m_mixGroupTestCreateDestroy)
            {
                if (ImGui::Button("Play"))
                {
                    s_testSound.SetMixGroup(m_mixGroupTestCreateDestroy);
                    s_testSound.Play();
                }

                if (ImGui::Button("Destroy"))
                {
                    m_context->GetMixerSystem()->DestroyMixGroup(&m_mixGroupTestCreateDestroy);
                }
            }
            else
            {
                if (ImGui::Button("Create"))
                {
                    m_mixGroupTestCreateDestroy = m_context->GetMixerSystem()->CreateMixGroup("Create Destroy Test");
                    s_testSend1 = m_mixGroupTestCreateDestroy->CreateSend(m_mixGroupReverb);
                    s_testSend2 = m_mixGroupTestCreateDestroy->CreateSend(m_mixGroupDelay);
                    s_testGain = m_mixGroupTestCreateDestroy->CreatePlugin<rf::GainPlugin>();
                    s_testDelay = m_mixGroupTestCreateDestroy->CreatePlugin<rf::PanPlugin>();
                }
            }

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Sends"))
        {
            if (m_sendTestCreateDestroy)
            {
                if (ImGui::Button("Destroy Send"))
                {
                    m_mixGroupEntities->DestroySend(&m_sendTestCreateDestroy);
                }
            }
            else
            {
                if (ImGui::Button("Create Send"))
                {
                    m_sendTestCreateDestroy = m_mixGroupEntities->CreateSend(m_mixGroupTestSendTo);
                }
            }

            {
                float db = m_sendEntitiesToReverb->GetVolumeDb();
                ImGui::DragFloat("Entities-To-Reverb", &db, 0.01f, -120.0f, 12.0f);
                m_sendEntitiesToReverb->SetVolumeDb(db);
            }

            {
                float db = m_sendEntitiesToDelay->GetVolumeDb();
                ImGui::DragFloat("Entities-To-Delay", &db, 0.01f, -120.0f, 12.0f);
                m_sendEntitiesToDelay->SetVolumeDb(db);
            }

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Plug-ins: Create and Destroy"))
        {
            static rf::GainPlugin* s_gain = nullptr;

            if (!s_gain)
            {
                if (ImGui::Button("Create Gain"))
                {
                    s_gain = m_mixGroupEntities->CreatePlugin<rf::GainPlugin>();
                    s_gain->SetGainDb(6.0f);
                }
            }
            else
            {
                if (ImGui::Button("Destroy Gain"))
                {
                    m_mixGroupEntities->DestroyPlugin<rf::GainPlugin>(&s_gain);
                }
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Plug-ins: Demonstrate All"))
        {
            ImGui::Text("Delay: Delay");
            ImGui::PushID("Delay: Delay");
            {
                float value1 = m_delayDelay->GetDelay();
                ImGui::DragFloat("Delay", &value1, 1.0f, 0.1f, 1000.0f);
                m_delayDelay->SetDelay(value1);

                float value2 = m_delayDelay->GetFeedback();
                ImGui::DragFloat("Feedback", &value2, 0.01f, 0.0f, 1.0f);
                m_delayDelay->SetFeedback(value2);

                bool bypass = m_delayDelay->GetBypass();
                ImGui::Checkbox("Bypass", &bypass);
                m_delayDelay->SetBypass(bypass);
            }
            ImGui::PopID();

            ImGui::Text("Reverb: Convolver");
            ImGui::PushID("Reverb: Convolver");
            {
                static bool s_loadOnBoot = true;
                if (s_loadOnBoot)
                {
                    s_loadOnBoot = false;
                    m_convolverReverb->LoadIR(m_assetIRSmall, 0);
                    m_convolverReverb->LoadIR(m_assetIRMedium, 1);
                    m_convolverReverb->LoadIR(m_assetIRLarge, 2);
                }

                {
                    static bool s_isSmallLoaded = true;
                    if (s_isSmallLoaded)
                    {
                        if (ImGui::Button("Unload Small IR"))
                        {
                            s_isSmallLoaded = false;
                            m_convolverReverb->UnloadIR(0);
                        }
                    }
                    else
                    {
                        if (ImGui::Button("Load Small IR"))
                        {
                            s_isSmallLoaded = true;
                            m_convolverReverb->LoadIR(m_assetIRSmall, 0);
                        }
                    }
                }

                {
                    static bool s_isMediumLoaded = true;
                    if (s_isMediumLoaded)
                    {
                        if (ImGui::Button("Unload Medium IR"))
                        {
                            s_isMediumLoaded = false;
                            m_convolverReverb->UnloadIR(1);
                        }
                    }
                    else
                    {
                        if (ImGui::Button("Load Medium IR"))
                        {
                            s_isMediumLoaded = true;
                            m_convolverReverb->LoadIR(m_assetIRMedium, 1);
                        }
                    }
                }

                {
                    static bool s_isLargeLoaded = true;
                    if (s_isLargeLoaded)
                    {
                        if (ImGui::Button("Unload Large IR"))
                        {
                            s_isLargeLoaded = false;
                            m_convolverReverb->UnloadIR(2);
                        }
                    }
                    else
                    {
                        if (ImGui::Button("Load Large IR"))
                        {
                            s_isLargeLoaded = true;
                            m_convolverReverb->LoadIR(m_assetIRLarge, 2);
                        }
                    }
                }

                float value1 = m_convolverReverb->GetWetPercentage();
                ImGui::DragFloat("Wet Percentage", &value1, 0.01f, 0.0f, 1.0f);
                m_convolverReverb->SetWetPercentage(value1);

                float dB1 = m_convolverReverb->GetIRVolumeDb(0);
                ImGui::DragFloat("Small Volume dB", &dB1, 0.01f, -60.0f, 12.0f);
                m_convolverReverb->SetIRVolumeDb(dB1, 0);

                float dB2 = m_convolverReverb->GetIRVolumeDb(1);
                ImGui::DragFloat("Medium Volume dB", &dB2, 0.01f, -60.0f, 12.0f);
                m_convolverReverb->SetIRVolumeDb(dB2, 1);

                float dB3 = m_convolverReverb->GetIRVolumeDb(2);
                ImGui::DragFloat("Large Volume dB", &dB3, 0.01f, -60.0f, 12.0f);
                m_convolverReverb->SetIRVolumeDb(dB3, 2);

                bool bypass = m_convolverReverb->GetBypass();
                ImGui::Checkbox("Bypass", &bypass);
                m_convolverReverb->SetBypass(bypass);
            }
            ImGui::PopID();

            ImGui::Text("Entities: Positioning");
            ImGui::PushID("Entities: Positioning");
            {
                rf::PositioningParameters params = m_positioningEntities->GetPositioningParameters();
                ImGui::Checkbox("Enable", &params.m_enable);

                static bool s_useLinear = true;
                static bool s_useEqualPower = false;
                static bool s_useQuadratic = false;

                const bool linear = ImGui::Checkbox("Linear", &s_useLinear);
                const bool equalPower = ImGui::Checkbox("Equal Power", &s_useEqualPower);
                const bool quadratic = ImGui::Checkbox("Quadratic", &s_useQuadratic);
                const bool changed = linear || equalPower || quadratic;

                if (changed)
                {
                    s_useLinear = false;
                    s_useEqualPower = false;
                    s_useQuadratic = false;

                    if (linear)
                    {
                        params.m_distanceCurveType = rf::PositioningParameters::DistanceCurve::Linear;
                        s_useLinear = true;
                    }
                    else if (equalPower)
                    {
                        params.m_distanceCurveType = rf::PositioningParameters::DistanceCurve::EqualPower;
                        s_useEqualPower = true;
                    }
                    else if (quadratic)
                    {
                        params.m_distanceCurveType = rf::PositioningParameters::DistanceCurve::Quadratic;
                        s_useQuadratic = true;
                    }
                }

                ImGui::DragFloat("Pan", &params.m_panAngle, 0.001f, -1.0f, 1.0f);
                ImGui::DragFloat("Current Distance", &params.m_currentDistance, 1.0f, 0.0f, 1000.0f);
                ImGui::DragFloat("Min Distance", &params.m_minDistance, 1.0f, 0.0f, 1000.0f);
                ImGui::DragFloat("Max Distance", &params.m_maxDistance, 1.0f, 0.0f, 1000.0f);
                ImGui::DragFloat("Max Attenuation dB", &params.m_maxAttenuationDb, 1.0f, -60.0f, 12.0f);
                ImGui::DragFloat("Max HPF Cutoff", &params.m_maxHpfCutoff, 1.0f, 20.0f, 20000.0f);
                ImGui::DragFloat("Max LPF Cutoff", &params.m_maxLpfCutoff, 1.0f, 20.0f, 20000.0f);
                m_positioningEntities->SetPositioningParameters(params);

                bool bypass = m_positioningEntities->GetBypass();
                ImGui::Checkbox("Bypass", &bypass);
                m_positioningEntities->SetBypass(bypass);
            }
            ImGui::PopID();

            ImGui::Text("Entities: IIR2 Highpass Filter");
            ImGui::PushID("Entities: IIR2 Highpass Filter");
            {
                float value1 = m_iir2HighpassEntities->GetQ();
                ImGui::DragFloat("Q", &value1, 0.1f, 0.1f, 1000.0f);
                m_iir2HighpassEntities->SetQ(value1);

                float value2 = m_iir2HighpassEntities->GetCutoff();
                ImGui::DragFloat("Cutoff", &value2, 1.0f, 20.0f, 20000.0f);
                m_iir2HighpassEntities->SetCutoff(value2);

                bool bypass = m_iir2HighpassEntities->GetBypass();
                ImGui::Checkbox("Bypass", &bypass);
                m_iir2HighpassEntities->SetBypass(bypass);
            }
            ImGui::PopID();

            ImGui::Text("Entities: IIR2 Lowpass Filter");
            ImGui::PushID("Entities: IIR2 Lowpass Filter");
            {
                float value1 = m_iir2LowpassEntities->GetQ();
                ImGui::DragFloat("Q", &value1, 0.1f, 0.1f, 1000.0f);
                m_iir2LowpassEntities->SetQ(value1);

                float value2 = m_iir2LowpassEntities->GetCutoff();
                ImGui::DragFloat("Cutoff", &value2, 1.0f, 20.0f, 20000.0f);
                m_iir2LowpassEntities->SetCutoff(value2);

                bool bypass = m_iir2LowpassEntities->GetBypass();
                ImGui::Checkbox("Bypass", &bypass);
                m_iir2LowpassEntities->SetBypass(bypass);
            }
            ImGui::PopID();

            ImGui::Text("Entities: Compressor");
            ImGui::PushID("Entities: Compressor");
            {
                float value1 = m_compressorEntities->GetThreshold();
                ImGui::DragFloat("Threshold", &value1, 0.1f, -200.0f, 200.0f);
                m_compressorEntities->SetThreshold(value1);

                float value2 = m_compressorEntities->GetRatio();
                ImGui::DragFloat("Ratio", &value2, 0.1f, -200.0f, 200.0f);
                m_compressorEntities->SetRatio(value2);

                float value3 = m_compressorEntities->GetMakeUpGainDb();
                ImGui::DragFloat("Make Up Gain", &value3, 0.1f, -200.0f, 200.0f);
                m_compressorEntities->SetMakeUpGainDb(value3);

                float value4 = m_compressorEntities->GetAttack();
                ImGui::DragFloat("Attack", &value4, 0.1f, -200.0f, 200.0f);
                m_compressorEntities->SetAttack(value4);

                float value5 = m_compressorEntities->GetRelease();
                ImGui::DragFloat("Release", &value5, 0.1f, -200.0f, 200.0f);
                m_compressorEntities->SetRelease(value5);

                bool bypass = m_compressorEntities->GetBypass();
                ImGui::Checkbox("Bypass", &bypass);
                m_compressorEntities->SetBypass(bypass);
            }
            ImGui::PopID();

            ImGui::Text("Ambience: Gain");
            ImGui::PushID("Ambience: Gain");
            {
                float value = m_gainAmbience->GetGainDb();
                ImGui::DragFloat("Gain", &value, 0.01f, -120.0f, 12.0f);
                m_gainAmbience->SetGainDb(value);

                bool bypass = m_gainAmbience->GetBypass();
                ImGui::Checkbox("Bypass", &bypass);
                m_gainAmbience->SetBypass(bypass);
            }
            ImGui::PopID();

            ImGui::Text("Ambience: Pan");
            ImGui::PushID("Ambience: Pan");
            {
                float value = m_panAmbience->GetAngle();
                ImGui::DragFloat("Pan", &value, 0.001f, -1.0f, 1.0f);
                m_panAmbience->SetAngle(value);

                bool bypass = m_panAmbience->GetBypass();
                ImGui::Checkbox("Bypass", &bypass);
                m_panAmbience->SetBypass(bypass);
            }
            ImGui::PopID();

            ImGui::Text("Ambience: Butterworth Highpass Filter");
            ImGui::PushID("Ambience: Butterworth Highpass Filter");
            {
                int value1 = m_bwHighpassAmbience->GetOrder();
                ImGui::DragInt("Order", &value1, 1.0f, 1, 2);
                m_bwHighpassAmbience->SetOrder(value1);

                float value2 = m_bwHighpassAmbience->GetCutoff();
                ImGui::DragFloat("Cutoff", &value2, 1.0f, 20.0f, 20000.0f);
                m_bwHighpassAmbience->SetCutoff(value2);

                bool bypass = m_bwHighpassAmbience->GetBypass();
                ImGui::Checkbox("Bypass", &bypass);
                m_bwHighpassAmbience->SetBypass(bypass);
            }
            ImGui::PopID();

            ImGui::Text("Ambience: Butterworth Lowpass Filter");
            ImGui::PushID("Ambience: Butterworth Lowpass Filter");
            {
                int value1 = m_bwLowpassAmbience->GetOrder();
                ImGui::DragInt("Order", &value1, 1.0f, 1, 2);
                m_bwLowpassAmbience->SetOrder(value1);

                float value2 = m_bwLowpassAmbience->GetCutoff();
                ImGui::DragFloat("Cutoff", &value2, 1.0f, 20.0f, 20000.0f);
                m_bwLowpassAmbience->SetCutoff(value2);

                bool bypass = m_bwLowpassAmbience->GetBypass();
                ImGui::Checkbox("Bypass", &bypass);
                m_bwLowpassAmbience->SetBypass(bypass);
            }
            ImGui::PopID();

            ImGui::Text("Master: Limiter");
            ImGui::PushID("Master: Limiter");
            {
                float value1 = m_limiterMaster->GetThreshold();
                ImGui::DragFloat("Threshold", &value1, 0.1f, -120.0f, 12.0f);
                m_limiterMaster->SetThreshold(value1);

                bool bypass = m_limiterMaster->GetBypass();
                ImGui::Checkbox("Bypass", &bypass);
                m_limiterMaster->SetBypass(bypass);
            }
            ImGui::PopID();

            ImGui::TreePop();
        }
    }
}

void RedFishDemo::Example8_ViewPlayingSounds()
{
    const std::vector<rf::PlayingSoundInfo>& sounds = m_context->GetPlayingSoundInfo();
    if (ImGui::CollapsingHeader("Playing Sound Info"))
    {
        for (const rf::PlayingSoundInfo& sound : sounds)
        {
            ImGui::Separator();
            ImGui::Text("Name: %s", sound.m_name);
            ImGui::Text("Audio Handle: %i", sound.m_audioHandle.m_id);
            ImGui::Separator();
        }
    }
}

void RedFishDemo::Example9_Music()
{
    rf::MusicSystem* musicSystem = m_context->GetMusicSystem();

    if (ImGui::CollapsingHeader("Music"))
    {
        ImGui::Text("Current Cue: %s", musicSystem->GetCurrentCueName());
        ImGui::Text("Meter      : %i / %i", musicSystem->GetCurrentMeter().m_top, musicSystem->GetCurrentMeter().m_bottom);
        ImGui::Text("Tempo      : %f", musicSystem->GetCurrentTempo());
        ImGui::Text("Bar.Beat   : %i.%i", musicSystem->GetCurrentBar(), musicSystem->GetCurrentBeat());
        ImGui::Text("");

        ImGui::Text("Cues define what is played.");
        ImGui::Text("Transitions define when and how cues are played.");
        ImGui::Text("Stingers are played over top of cues during transitions to help the transition sound more musical.");
        ImGui::Text("");

        if (ImGui::TreeNode("Create and Destroy Cues, Transitions, and Stingers"))
        {
            ImGui::Text("Creating does not allocate any memory.");
            ImGui::Text("Therefore, it is not required to destroy this pointers to avoid a memory leak.");
            ImGui::Text("You may want to destroy cues / transitions / stingers as a part of the runtime of your application though.");

            static bool s_hasBeenCreated = false;
            static rf::Cue* s_cueCreateTest = nullptr;
            static rf::Transition* s_transitionCreateTest = nullptr;
            static rf::Stinger* s_stingerCreateTest = nullptr;

            const auto Create = [this, musicSystem]() {
                rf::CueParameters cueParams;
                cueParams.m_meter = rf::Meter(4, 4);
                cueParams.m_tempo = 130.0f;
                cueParams.m_name = "CreateCueTest";
                cueParams.AddLayer(m_assetBasicSongIntro, m_mixGroupMusic);
                s_cueCreateTest = musicSystem->CreateCue(cueParams);

                rf::StingerParameters stingerParameters;
                stingerParameters.m_cue = s_cueCreateTest;
                stingerParameters.m_name = "CreateTransitionTest";
                s_stingerCreateTest = musicSystem->CreateStinger(stingerParameters);

                rf::TransitionParameters transParams;
                transParams.m_cue = s_cueCreateTest;
                transParams.m_name = "CreateTransitionTest";
                s_transitionCreateTest = musicSystem->CreateTransition(transParams);

                s_hasBeenCreated = true;
            };

            const auto Destroy = [this, musicSystem]() {
                musicSystem->DestroyCue(&s_cueCreateTest);
                musicSystem->DestroyTransition(&s_transitionCreateTest);
                musicSystem->DestroyStinger(&s_stingerCreateTest);

                s_hasBeenCreated = false;
            };

            if (!s_hasBeenCreated)
            {
                if (ImGui::Button("Create"))
                {
                    Create();
                }
            }
            else
            {
                if (ImGui::Button("Destroy"))
                {
                    Destroy();
                }
            }

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Basics"))
        {
            ImGui::Text("You can play a transition directly.");

            static bool s_hasBeenCreated = false;
            static rf::Cue* s_cueBasicSongIntro = nullptr;
            static rf::Cue* s_cueBasicSongLoop = nullptr;
            static rf::Transition* s_toBasicLoop = nullptr;
            static rf::Transition* s_toBasicIntro = nullptr;

            const auto Create = [this, musicSystem]() {
                if (s_hasBeenCreated)
                {
                    return;
                }

                // Cues
                {
                    rf::CueParameters params;
                    params.m_meter = rf::Meter(4, 4);
                    params.m_tempo = 130.0f;

                    params.m_name = "BasicSong_Intro";
                    params.AddLayer(m_assetBasicSongIntro, m_mixGroupMusic);
                    s_cueBasicSongIntro = musicSystem->CreateCue(params);
                    params.ClearLayers();

                    params.m_name = "BasicSong_Loop";
                    params.AddLayer(m_assetBasicSongLoop, m_mixGroupMusic);
                    s_cueBasicSongLoop = musicSystem->CreateCue(params);
                }

                // Transitions
                {
                    rf::TransitionParameters loop;
                    loop.m_cue = s_cueBasicSongLoop;
                    loop.m_name = "To_BasicSong_Loop";
                    s_toBasicLoop = musicSystem->CreateTransition(loop);

                    rf::TransitionParameters intro;
                    intro.m_cue = s_cueBasicSongIntro;
                    intro.m_name = "To_BasicSong_Intro";
                    intro.m_playCount = 1;
                    s_toBasicIntro = musicSystem->CreateTransition(intro);
                }

                s_hasBeenCreated = true;
            };

            const auto Destroy = [this, musicSystem]() {
                if (!s_hasBeenCreated)
                {
                    return;
                }

                musicSystem->DestroyCue(&s_cueBasicSongIntro);
                musicSystem->DestroyCue(&s_cueBasicSongLoop);
                musicSystem->DestroyTransition(&s_toBasicLoop);
                musicSystem->DestroyTransition(&s_toBasicIntro);

                s_hasBeenCreated = false;
            };

            Create();

            if (ImGui::Button("Play Intro"))
            {
                musicSystem->Play(s_toBasicIntro);
            }

            if (ImGui::Button("Play Loop"))
            {
                musicSystem->Play(s_toBasicLoop);
            }

            ImGui::SameLine();

            if (ImGui::Button("Stop"))
            {
                musicSystem->Stop();
            }

            if (ImGui::Button("Destroy"))
            {
                Destroy();
            }

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Follow Up Transitions"))
        {
            ImGui::Text("Follow Up Transitions allow you to set a cue to play as soon as the current cue has finished playing.");
            ImGui::Text("A use might be setting up a looping song. You may want an intro cue that leads into the main loop.");

            static bool s_hasBeenCreated = false;
            static rf::Cue* s_cueBasicSongIntro = nullptr;
            static rf::Cue* s_cueBasicSongLoop = nullptr;
            static rf::Transition* s_toBasicLoop = nullptr;
            static rf::Transition* s_toBasicIntro = nullptr;

            const auto Create = [this, musicSystem]() {
                if (s_hasBeenCreated)
                {
                    return;
                }

                // Cues
                {
                    rf::CueParameters params;
                    params.m_meter = rf::Meter(4, 4);
                    params.m_tempo = 130.0f;

                    params.m_name = "BasicSong_Intro";
                    params.AddLayer(m_assetBasicSongIntro, m_mixGroupMusic);
                    s_cueBasicSongIntro = musicSystem->CreateCue(params);
                    params.ClearLayers();

                    params.m_name = "BasicSong_Loop";
                    params.AddLayer(m_assetBasicSongLoop, m_mixGroupMusic);
                    s_cueBasicSongLoop = musicSystem->CreateCue(params);
                }

                // Transitions
                {
                    rf::TransitionParameters loop;
                    loop.m_cue = s_cueBasicSongLoop;
                    loop.m_name = "To_BasicSong_Loop";
                    s_toBasicLoop = musicSystem->CreateTransition(loop);

                    rf::TransitionParameters intro;
                    intro.m_cue = s_cueBasicSongIntro;
                    intro.m_name = "To_BasicSong_Intro";
                    intro.m_playCount = 1;
                    intro.m_followUpTransition = s_toBasicLoop;
                    s_toBasicIntro = musicSystem->CreateTransition(intro);
                }

                s_hasBeenCreated = true;
            };

            const auto Destroy = [this, musicSystem]() {
                if (!s_hasBeenCreated)
                {
                    return;
                }

                musicSystem->DestroyCue(&s_cueBasicSongIntro);
                musicSystem->DestroyCue(&s_cueBasicSongLoop);
                musicSystem->DestroyTransition(&s_toBasicLoop);
                musicSystem->DestroyTransition(&s_toBasicIntro);

                s_hasBeenCreated = false;
            };

            Create();

            if (ImGui::Button("Play"))
            {
                musicSystem->Play(s_toBasicIntro);
            }

            ImGui::SameLine();

            if (ImGui::Button("Stop"))
            {
                musicSystem->Stop();
            }

            if (ImGui::Button("Destroy"))
            {
                Destroy();
            }

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Fade Out and Stop"))
        {
            static bool s_hasBeenCreated = false;
            static rf::Cue* s_cueBasicSongLoop = nullptr;
            static rf::Transition* s_toBasicLoop = nullptr;

            const auto Create = [this, musicSystem]() {
                if (s_hasBeenCreated)
                {
                    return;
                }

                // Cues
                {
                    rf::CueParameters params;
                    params.m_meter = rf::Meter(4, 4);
                    params.m_tempo = 130.0f;

                    params.m_name = "BasicSong_Loop";
                    params.AddLayer(m_assetBasicSongLoop, m_mixGroupMusic);
                    s_cueBasicSongLoop = musicSystem->CreateCue(params);
                }

                // Transitions
                {
                    rf::TransitionParameters loop;
                    loop.m_cue = s_cueBasicSongLoop;
                    loop.m_name = "To_BasicSong_Loop";
                    s_toBasicLoop = musicSystem->CreateTransition(loop);
                }

                s_hasBeenCreated = true;
            };

            const auto Destroy = [this, musicSystem]() {
                if (!s_hasBeenCreated)
                {
                    return;
                }

                musicSystem->DestroyCue(&s_cueBasicSongLoop);
                musicSystem->DestroyTransition(&s_toBasicLoop);

                s_hasBeenCreated = false;
            };

            Create();

            if (ImGui::Button("Play"))
            {
                musicSystem->Play(s_toBasicLoop);
            }

            ImGui::SameLine();

            if (ImGui::Button("Stop"))
            {
                musicSystem->Stop();
            }

            if (ImGui::Button("Musical Fade Out and Stop"))
            {
                musicSystem->FadeOutAndStop(rf::Sync(rf::Sync::Value::Bar), rf::Sync(rf::Sync::Value::Bar, 2));
            }

            if (ImGui::Button("Time Fade Out and Stop"))
            {
                musicSystem->FadeOutAndStop(rf::Sync(0.0f), rf::Sync(5.0f));
            }

            if (ImGui::Button("Destroy"))
            {
                Destroy();
            }

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Layers"))
        {
            ImGui::Text("Cues can be layered meaning multiple audio assets are played at the same time.");
            ImGui::Text("Each audio asset can be routed through a different mix group meaning you can dynamically mix music layers");

            static bool s_hasBeenCreated = false;
            static rf::Cue* s_cueSong2 = nullptr;
            static rf::Transition* s_toSong2 = nullptr;

            const auto Create = [this, musicSystem]() {
                if (s_hasBeenCreated)
                {
                    return;
                }

                // Cues
                {
                    rf::CueParameters params;
                    params.m_meter = rf::Meter(3, 4);
                    params.m_tempo = 135.0f;
                    params.m_gainDb = -3.0f;
                    params.m_name = "Song2";
                    params.AddLayer(m_assetSong2Lead, m_mixGroupLeads);
                    params.AddLayer(m_assetSong2Bass, m_mixGroupBass, -1.0f);
                    params.AddLayer(m_assetSong2Drums, m_mixGroupDrums);
                    s_cueSong2 = musicSystem->CreateCue(params);
                }

                // Transitions
                {
                    rf::TransitionParameters params;
                    params.m_cue = s_cueSong2;
                    params.m_sync = rf::Sync(rf::Sync::Value::Bar);
                    params.m_name = "To_Song2";
                    s_toSong2 = musicSystem->CreateTransition(params);
                }

                s_hasBeenCreated = true;
            };

            const auto Destroy = [this, musicSystem]() {
                if (!s_hasBeenCreated)
                {
                    return;
                }

                musicSystem->DestroyCue(&s_cueSong2);
                musicSystem->DestroyTransition(&s_toSong2);

                s_hasBeenCreated = false;
            };

            Create();

            if (ImGui::Button("Play"))
            {
                musicSystem->Play(s_toSong2);
            }

            ImGui::SameLine();

            if (ImGui::Button("Stop"))
            {
                musicSystem->Stop();
            }

            static float s_fadeDrumsToDb = 0.0f;
            ImGui::DragFloat("Fade To dB", &s_fadeDrumsToDb, 0.01f, -120.0f, 12.0f);

            if (ImGui::Button("Fade Drums"))
            {
                rf::MixerSystem* mixerSystem = m_context->GetMixerSystem();
                const rf::MixGroup* layers[] = {m_mixGroupDrums};
                mixerSystem->FadeMixGroups(layers, 1, s_fadeDrumsToDb, rf::Sync(rf::Sync::Value::Bar), rf::Sync(rf::Sync::Value::Bar));
            }

            if (ImGui::Button("Destroy"))
            {
                Destroy();
            }

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Musical Transitions"))
        {
            ImGui::Text("RedFish support a variety of musical transition via rf::Sync");
            ImGui::Text("Cut mean an immediate transition.");
            ImGui::Text("Queue means wait until the current cue is finish playing.");

            static bool s_hasBeenCreated = false;
            static rf::Cue* s_cueSong1Cue1 = nullptr;
            static rf::Cue* s_cueSong1Cue2 = nullptr;
            static rf::Cue* s_cueSong1Cue3 = nullptr;
            static rf::Cue* s_cueSong1Cue4 = nullptr;
            static rf::Cue* s_cueSong1Cue5 = nullptr;
            static rf::Cue* s_cueSong1Cue6 = nullptr;
            static rf::Cue* s_cueSong1Cue7 = nullptr;
            static rf::Cue* s_cueSong1Cue8 = nullptr;
            static rf::Cue* s_cueSong1Cue9 = nullptr;
            static rf::Cue* s_cueSong1Cue10 = nullptr;
            static rf::Cue* s_cueSong1Cue11 = nullptr;
            static rf::Cue* s_cueSong1Stinger = nullptr;
            static rf::Stinger* s_song1Stinger = nullptr;
            static rf::Transition* s_toSong1Cue1 = nullptr;
            static rf::Transition* s_toSong1Cue2 = nullptr;
            static rf::Transition* s_toSong1Cue3 = nullptr;
            static rf::Transition* s_toSong1Cue4 = nullptr;
            static rf::Transition* s_toSong1Cue5 = nullptr;
            static rf::Transition* s_toSong1Cue6 = nullptr;
            static rf::Transition* s_toSong1Cue7 = nullptr;
            static rf::Transition* s_toSong1Cue8 = nullptr;
            static rf::Transition* s_toSong1Cue9 = nullptr;
            static rf::Transition* s_toSong1Cue10 = nullptr;
            static rf::Transition* s_toSong1Cue11 = nullptr;

            const auto Create = [this, musicSystem]() {
                if (s_hasBeenCreated)
                {
                    return;
                }

                // Cues
                {
                    rf::CueParameters params;
                    params.m_meter = rf::Meter(4, 4);
                    params.m_tempo = 110.0f;

                    params.m_name = "Song1_Cue1";
                    params.AddLayer(m_assetSong1Cue1Drums, m_mixGroupDrums);
                    s_cueSong1Cue1 = musicSystem->CreateCue(params);
                    params.ClearLayers();

                    params.m_name = "Song1_Cue2";
                    params.AddLayer(m_assetSong1Cue2Lead, m_mixGroupLeads);
                    params.AddLayer(m_assetSong1Cue2Drums, m_mixGroupDrums);
                    s_cueSong1Cue2 = musicSystem->CreateCue(params);
                    params.ClearLayers();

                    params.m_name = "Song1_Cue3";
                    params.AddLayer(m_assetSong1Cue3Lead, m_mixGroupLeads);
                    params.AddLayer(m_assetSong1Cue3Bass, m_mixGroupBass);
                    params.AddLayer(m_assetSong1Cue3Drums, m_mixGroupDrums);
                    s_cueSong1Cue3 = musicSystem->CreateCue(params);
                    params.ClearLayers();

                    params.m_name = "Song1_Cue4";
                    params.AddLayer(m_assetSong1Cue4Lead, m_mixGroupLeads);
                    params.AddLayer(m_assetSong1Cue4Bass, m_mixGroupBass);
                    params.AddLayer(m_assetSong1Cue4Drums, m_mixGroupDrums);
                    s_cueSong1Cue4 = musicSystem->CreateCue(params);
                    params.ClearLayers();

                    params.m_name = "Song1_Cue5";
                    params.AddLayer(m_assetSong1Cue5Lead, m_mixGroupLeads);
                    params.AddLayer(m_assetSong1Cue5Drums, m_mixGroupDrums);
                    s_cueSong1Cue5 = musicSystem->CreateCue(params);
                    params.ClearLayers();

                    params.m_name = "Song1_Cue6";
                    params.AddLayer(m_assetSong1Cue6Lead, m_mixGroupLeads);
                    params.AddLayer(m_assetSong1Cue6Bass, m_mixGroupBass);
                    params.AddLayer(m_assetSong1Cue6Drums, m_mixGroupDrums);
                    s_cueSong1Cue6 = musicSystem->CreateCue(params);
                    params.ClearLayers();

                    params.m_name = "Song1_Cue7";
                    params.AddLayer(m_assetSong1Cue7Bass, m_mixGroupBass);
                    params.AddLayer(m_assetSong1Cue7Drums, m_mixGroupDrums);
                    s_cueSong1Cue7 = musicSystem->CreateCue(params);
                    params.ClearLayers();

                    params.m_name = "Song1_Cue8";
                    params.AddLayer(m_assetSong1Cue8Lead, m_mixGroupLeads);
                    params.AddLayer(m_assetSong1Cue8Bass, m_mixGroupBass);
                    params.AddLayer(m_assetSong1Cue8Drums, m_mixGroupDrums);
                    s_cueSong1Cue8 = musicSystem->CreateCue(params);
                    params.ClearLayers();

                    params.m_name = "Song1_Cue9";
                    params.AddLayer(m_assetSong1Cue9Lead, m_mixGroupLeads);
                    params.AddLayer(m_assetSong1Cue9Bass, m_mixGroupBass);
                    params.AddLayer(m_assetSong1Cue9Drums, m_mixGroupDrums);
                    s_cueSong1Cue9 = musicSystem->CreateCue(params);
                    params.ClearLayers();

                    params.m_name = "Song1_Cue10";
                    params.AddLayer(m_assetSong1Cue10Lead, m_mixGroupLeads);
                    params.AddLayer(m_assetSong1Cue10Bass, m_mixGroupBass);
                    params.AddLayer(m_assetSong1Cue10Drums, m_mixGroupDrums);
                    s_cueSong1Cue10 = musicSystem->CreateCue(params);
                    params.ClearLayers();

                    params.m_name = "Song1_Cue11";
                    params.AddLayer(m_assetSong1Cue11Lead, m_mixGroupLeads);
                    params.AddLayer(m_assetSong1Cue11Bass, m_mixGroupBass);
                    params.AddLayer(m_assetSong1Cue11Drums, m_mixGroupDrums);
                    s_cueSong1Cue11 = musicSystem->CreateCue(params);
                    params.ClearLayers();

                    params.m_meter = rf::Meter(4, 4);
                    params.m_tempo = 110.0f;
                    params.m_name = "Song1_Stinger";
                    params.AddLayer(m_assetSong1Stinger, m_mixGroupDrums);
                    s_cueSong1Stinger = musicSystem->CreateCue(params);
                }

                // Stingers
                {
                    rf::StingerParameters params;
                    params.m_cue = s_cueSong1Stinger;
                    params.m_sync = rf::Sync(rf::Sync::Value::Bar);
                    params.m_gainDb = -3.0f;
                    params.m_name = "Stinger_Song1";
                    s_song1Stinger = musicSystem->CreateStinger(params);
                }

                // Transitions
                {
                    rf::TransitionParameters params;

                    params.m_cue = s_cueSong1Cue1;
                    params.m_sync = rf::Sync(rf::Sync::Value::Bar);
                    params.m_name = "To_Song1_Cue1";
                    s_toSong1Cue1 = musicSystem->CreateTransition(params);

                    params.m_cue = s_cueSong1Cue2;
                    params.m_name = "To_Song1_Cue2";
                    s_toSong1Cue2 = musicSystem->CreateTransition(params);

                    params.m_cue = s_cueSong1Cue3;
                    params.m_name = "To_Song1_Cue3";
                    s_toSong1Cue3 = musicSystem->CreateTransition(params);

                    params.m_cue = s_cueSong1Cue4;
                    params.m_sync = rf::Sync(rf::Sync::Value::Quarter);
                    params.m_name = "To_Song1_Cue4";
                    s_toSong1Cue4 = musicSystem->CreateTransition(params);

                    params.m_cue = s_cueSong1Cue5;
                    params.m_sync = rf::Sync(rf::Sync::Value::Queue);
                    params.m_name = "To_Song1_Cue5";
                    s_toSong1Cue5 = musicSystem->CreateTransition(params);

                    params.m_cue = s_cueSong1Cue6;
                    params.m_sync = rf::Sync(rf::Sync::Value::Bar);
                    params.m_name = "To_Song1_Cue6";
                    s_toSong1Cue6 = musicSystem->CreateTransition(params);

                    params.m_cue = s_cueSong1Cue7;
                    params.m_name = "To_Song1_Cue7";
                    s_toSong1Cue7 = musicSystem->CreateTransition(params);

                    params.m_cue = s_cueSong1Cue8;
                    params.m_name = "To_Song1_Cue8";
                    s_toSong1Cue8 = musicSystem->CreateTransition(params);

                    params.m_cue = s_cueSong1Cue9;
                    params.m_name = "To_Song1_Cue9";
                    s_toSong1Cue9 = musicSystem->CreateTransition(params);

                    params.m_cue = s_cueSong1Cue10;
                    params.m_name = "To_Song1_Cue10";
                    s_toSong1Cue10 = musicSystem->CreateTransition(params);

                    params.m_cue = s_cueSong1Cue11;
                    params.m_playCount = 1;
                    params.m_stinger = s_song1Stinger;
                    params.m_name = "To_Song1_Cue11";
                    s_toSong1Cue11 = musicSystem->CreateTransition(params);
                }

                s_hasBeenCreated = true;
            };

            const auto Destroy = [this, musicSystem]() {
                if (!s_hasBeenCreated)
                {
                    return;
                }

                musicSystem->DestroyCue(&s_cueSong1Cue1);
                musicSystem->DestroyCue(&s_cueSong1Cue2);
                musicSystem->DestroyCue(&s_cueSong1Cue3);
                musicSystem->DestroyCue(&s_cueSong1Cue4);
                musicSystem->DestroyCue(&s_cueSong1Cue5);
                musicSystem->DestroyCue(&s_cueSong1Cue6);
                musicSystem->DestroyCue(&s_cueSong1Cue7);
                musicSystem->DestroyCue(&s_cueSong1Cue8);
                musicSystem->DestroyCue(&s_cueSong1Cue9);
                musicSystem->DestroyCue(&s_cueSong1Cue10);
                musicSystem->DestroyCue(&s_cueSong1Cue11);
                musicSystem->DestroyCue(&s_cueSong1Stinger);
                musicSystem->DestroyStinger(&s_song1Stinger);
                musicSystem->DestroyTransition(&s_toSong1Cue1);
                musicSystem->DestroyTransition(&s_toSong1Cue2);
                musicSystem->DestroyTransition(&s_toSong1Cue3);
                musicSystem->DestroyTransition(&s_toSong1Cue4);
                musicSystem->DestroyTransition(&s_toSong1Cue5);
                musicSystem->DestroyTransition(&s_toSong1Cue6);
                musicSystem->DestroyTransition(&s_toSong1Cue7);
                musicSystem->DestroyTransition(&s_toSong1Cue8);
                musicSystem->DestroyTransition(&s_toSong1Cue9);
                musicSystem->DestroyTransition(&s_toSong1Cue10);
                musicSystem->DestroyTransition(&s_toSong1Cue11);

                s_hasBeenCreated = false;
            };

            Create();

            if (ImGui::Button("Cue 1"))
            {
                musicSystem->Play(s_toSong1Cue1);
            }

            ImGui::SameLine();

            if (ImGui::Button("Cue 2"))
            {
                musicSystem->Play(s_toSong1Cue2);
            }

            ImGui::SameLine();

            if (ImGui::Button("Cue 3"))
            {
                musicSystem->Play(s_toSong1Cue3);
            }

            ImGui::SameLine();

            if (ImGui::Button("Cue 4"))
            {
                musicSystem->Play(s_toSong1Cue4);
            }

            ImGui::SameLine();

            if (ImGui::Button("Cue 5"))
            {
                musicSystem->Play(s_toSong1Cue5);
            }

            if (ImGui::Button("Cue 6"))
            {
                musicSystem->Play(s_toSong1Cue6);
            }

            ImGui::SameLine();

            if (ImGui::Button("Cue 7"))
            {
                musicSystem->Play(s_toSong1Cue7);
            }

            ImGui::SameLine();

            if (ImGui::Button("Cue 8"))
            {
                musicSystem->Play(s_toSong1Cue8);
            }

            ImGui::SameLine();

            if (ImGui::Button("Cue 9"))
            {
                musicSystem->Play(s_toSong1Cue9);
            }

            ImGui::SameLine();

            if (ImGui::Button("Cue 10"))
            {
                musicSystem->Play(s_toSong1Cue10);
            }

            ImGui::SameLine();

            if (ImGui::Button("Cue 11"))
            {
                musicSystem->Play(s_toSong1Cue11);
            }

            static float s_fadeLayersToDb = 0.0f;
            ImGui::DragFloat("Fade To dB", &s_fadeLayersToDb, 0.01f, -120.0f, 12.0f);
            if (ImGui::Button("Fade Leads & Bass with Stinger"))
            {
                rf::MixerSystem* mixerSystem = m_context->GetMixerSystem();
                const rf::MixGroup* layers[] = {m_mixGroupLeads, m_mixGroupBass};
                mixerSystem->FadeMixGroups(layers, 2, s_fadeLayersToDb, rf::Sync(rf::Sync::Value::Bar), rf::Sync(0.1f), s_song1Stinger);
            }

            if (ImGui::Button("Stop"))
            {
                musicSystem->Stop();
            }

            if (ImGui::Button("Destroy"))
            {
                Destroy();
            }

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Play Stinger"))
        {
            ImGui::Text("Stingers can be played by themselves.");
            ImGui::Text("Play a music cue first and see how stingers are synced to the beat");

            static bool s_hasBeenCreated = false;
            static rf::Cue* s_cueSong1Stinger = nullptr;
            static rf::Stinger* s_song1Stinger = nullptr;

            const auto Create = [this, musicSystem]() {
                if (s_hasBeenCreated)
                {
                    return;
                }

                // Cues
                {
                    rf::CueParameters params;
                    params.m_meter = rf::Meter(4, 4);
                    params.m_tempo = 110.0f;
                    params.m_name = "Song1_Stinger";
                    params.AddLayer(m_assetSong1Stinger, m_mixGroupDrums);
                    s_cueSong1Stinger = musicSystem->CreateCue(params);
                    params.ClearLayers();
                }

                // Stingers
                {
                    rf::StingerParameters params;
                    params.m_cue = s_cueSong1Stinger;
                    params.m_sync = rf::Sync(rf::Sync::Value::Bar);
                    params.m_gainDb = -3.0f;
                    params.m_name = "Stinger_Song1";
                    s_song1Stinger = musicSystem->CreateStinger(params);
                }

                s_hasBeenCreated = true;
            };

            const auto Destroy = [this, musicSystem]() {
                if (!s_hasBeenCreated)
                {
                    return;
                }

                musicSystem->DestroyCue(&s_cueSong1Stinger);
                musicSystem->DestroyStinger(&s_song1Stinger);

                s_hasBeenCreated = false;
            };

            Create();

            if (ImGui::Button("Play"))
            {
                musicSystem->Play(s_song1Stinger);
            }

            ImGui::SameLine();

            if (ImGui::Button("Stop"))
            {
                musicSystem->Stop();
            }

            if (ImGui::Button("Destroy"))
            {
                Destroy();
            }

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Unload Assets While Playing Music"))
        {
            static bool s_isLoaded = false;
            static bool s_hasUnloadMusicTestBeenCreated = false;
            static rf::Cue* s_cueSong3Intro = nullptr;
            static rf::Cue* s_cueSong3Loop = nullptr;
            static rf::Transition* s_toSong3Loop = nullptr;
            static rf::Transition* s_toSong3Intro = nullptr;
            rf::AssetSystem* assetSystem = m_context->GetAssetSystem();

            const auto Destroy = [this, musicSystem]() {
                if (!s_hasUnloadMusicTestBeenCreated)
                {
                    return;
                }

                musicSystem->DestroyCue(&s_cueSong3Intro);
                musicSystem->DestroyCue(&s_cueSong3Loop);
                musicSystem->DestroyTransition(&s_toSong3Loop);
                musicSystem->DestroyTransition(&s_toSong3Intro);

                s_hasUnloadMusicTestBeenCreated = false;
            };

            const auto Create = [this, musicSystem, &Destroy]() {
                if (s_hasUnloadMusicTestBeenCreated)
                {
                    return;
                }

                Destroy();

                // Cues
                {
                    rf::CueParameters params;
                    params.m_meter = rf::Meter(4, 4);
                    params.m_tempo = 150.0f;

                    params.m_name = "Song3_Intro";
                    params.AddLayer(m_assetSong3Intro, m_mixGroupMusic);
                    s_cueSong3Intro = musicSystem->CreateCue(params);
                    params.ClearLayers();

                    params.m_name = "Song3_Loop";
                    params.AddLayer(m_assetSong3Loop, m_mixGroupMusic);
                    s_cueSong3Loop = musicSystem->CreateCue(params);
                }

                // Transitions
                {
                    rf::TransitionParameters loop;
                    loop.m_cue = s_cueSong3Loop;
                    loop.m_name = "To_Song3_Loop";
                    s_toSong3Loop = musicSystem->CreateTransition(loop);

                    rf::TransitionParameters intro;
                    intro.m_cue = s_cueSong3Intro;
                    intro.m_name = "To_Song3_Intro";
                    intro.m_playCount = 1;
                    intro.m_followUpTransition = s_toSong3Loop;
                    s_toSong3Intro = musicSystem->CreateTransition(intro);
                }

                s_hasUnloadMusicTestBeenCreated = true;
            };

            if (!s_isLoaded)
            {
                if (ImGui::Button("Load"))
                {
                    m_assetSong3Intro = assetSystem->Load("../testbench/testdata/song3/song3_intro.flac");
                    m_assetSong3Loop = assetSystem->Load("../testbench/testdata/song3/song3_loop.flac");
                    s_isLoaded = true;
                }
            }
            else
            {
                if (ImGui::Button("Unload"))
                {
                    assetSystem->Unload(m_assetSong3Intro);
                    assetSystem->Unload(m_assetSong3Loop);
                    s_isLoaded = false;
                    s_hasUnloadMusicTestBeenCreated = false;
                }
            }

            if (s_isLoaded)
            {
                Create();

                if (ImGui::Button("Play"))
                {
                    musicSystem->Play(s_toSong3Intro);
                }

                ImGui::SameLine();

                if (ImGui::Button("Stop"))
                {
                    musicSystem->Stop();
                }
            }

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Sync Reference Points"))
        {
            ImGui::Text("Sync reference points gives you options on how to time your transitions.");
            ImGui::Text("There are 2 reference points: Bar, and CueStart.");
            ImGui::Text("Bar: the transition timing is calculated with respect to the start of the current bar.");
            ImGui::Text("     Therefore a rf::Sync(rf::Sync::Value::Bar) will occur on the next bar.");
            ImGui::Text("CueStart: the transition timing is calculated with respect to the start of the current cue.");
            ImGui::Text("          Therefore a rf::Sync(rf::Sync::Value::Bar) will occur after the first bar of the cue (bar 2).");
            ImGui::Text("          A rf::Sync(rf::Sync::Value::Bar, 8) will occur after the bar 8 of the cue (bar 9).");

            static bool s_hasBeenCreated = false;
            static rf::Cue* s_cueSong4Cue = nullptr;
            static rf::Cue* s_cueSong4Exit1 = nullptr;
            static rf::Cue* s_cueSong4Exit2 = nullptr;
            static rf::Transition* s_toCueSong4Cue = nullptr;
            static rf::Transition* s_toCueSong4Exit1 = nullptr;
            static rf::Transition* s_toCueSong4Exit2 = nullptr;

            const auto Create = [this, musicSystem]() {
                if (s_hasBeenCreated)
                {
                    return;
                }

                // Cues
                {
                    rf::CueParameters params;
                    params.m_meter = rf::Meter(4, 4);
                    params.m_tempo = 120.0f;

                    params.m_name = "Song4_Cue";
                    params.AddLayer(m_assetSong4Cue, m_mixGroupMusic);
                    s_cueSong4Cue = musicSystem->CreateCue(params);
                    params.ClearLayers();

                    params.m_name = "Song4_Exit1";
                    params.AddLayer(m_assetSong4Exit1, m_mixGroupMusic);
                    s_cueSong4Exit1 = musicSystem->CreateCue(params);
                    params.ClearLayers();

                    params.m_name = "Song4_Exit2";
                    params.AddLayer(m_assetSong4Exit2, m_mixGroupMusic);
                    s_cueSong4Exit2 = musicSystem->CreateCue(params);
                }

                // Transitions
                {
                    rf::TransitionParameters cue;
                    cue.m_cue = s_cueSong4Cue;
                    cue.m_name = "To_Song4_Cue";
                    cue.m_playCount = 1;
                    s_toCueSong4Cue = musicSystem->CreateTransition(cue);

                    rf::TransitionParameters exit1;
                    exit1.m_cue = s_cueSong4Exit1;
                    exit1.m_playCount = 1;
                    exit1.m_sync = rf::Sync(rf::Sync::Value::Bar);
                    exit1.m_name = "To_Song4_Exit1";
                    s_toCueSong4Exit1 = musicSystem->CreateTransition(exit1);

                    rf::TransitionParameters exit2;
                    exit2.m_cue = s_cueSong4Exit2;
                    exit2.m_playCount = 1;
                    exit2.m_sync = rf::Sync(rf::Sync::Value::Bar, 8);
                    exit2.m_sync.m_referencePoint = rf::Sync::ReferencePoint::CueStart;
                    exit2.m_name = "To_BasicSong_Intro";
                    s_toCueSong4Exit2 = musicSystem->CreateTransition(exit2);
                }

                s_hasBeenCreated = true;
            };

            const auto Destroy = [this, musicSystem]() {
                if (!s_hasBeenCreated)
                {
                    return;
                }

                musicSystem->DestroyCue(&s_cueSong4Cue);
                musicSystem->DestroyCue(&s_cueSong4Exit1);
                musicSystem->DestroyCue(&s_cueSong4Exit2);
                musicSystem->DestroyTransition(&s_toCueSong4Cue);
                musicSystem->DestroyTransition(&s_toCueSong4Exit1);
                musicSystem->DestroyTransition(&s_toCueSong4Exit2);
                s_hasBeenCreated = false;
            };

            Create();

            if (ImGui::Button("Play Cue"))
            {
                musicSystem->Play(s_toCueSong4Cue);
            }

            if (ImGui::Button("Play Exit 1 (Bar Reference, Sync = 1 Bar)"))
            {
                musicSystem->Play(s_toCueSong4Exit1);
            }

            if (ImGui::Button("Play Exit 2 (Cue Start Reference, Sync = 8 Bars)"))
            {
                musicSystem->Play(s_toCueSong4Exit2);
            }

            ImGui::SameLine();

            if (ImGui::Button("Stop"))
            {
                musicSystem->Stop();
            }

            if (ImGui::Button("Destroy"))
            {
                Destroy();
            }

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Transition Conditions: User Data"))
        {
            ImGui::Text("Transitions Conditions allow you to author logic associated with your transitions.");
            ImGui::Text("Then, you can ask RedFish to automatically pick a transition that meets the current application / music state.");
            ImGui::Text("You inform RedFish about your application's state by filling out a rf::MusicSystem::UserData struct.");

            static bool s_hasBeenCreated = false;
            static rf::Cue* s_cueSong1Cue1 = nullptr;
            static rf::Cue* s_cueSong1Cue2 = nullptr;
            static rf::Cue* s_cueSong1Cue3 = nullptr;
            static rf::Transition* s_toSong1Cue1 = nullptr;
            static rf::Transition* s_toSong1Cue2 = nullptr;
            static rf::Transition* s_toSong1Cue3 = nullptr;

            enum DemoState1 : int
            {
                None = 10,
                State1,
                State2,
                State3
            };

            const auto Create = [this, musicSystem]() {
                if (s_hasBeenCreated)
                {
                    return;
                }

                // Cues
                {
                    rf::CueParameters params;
                    params.m_meter = rf::Meter(4, 4);
                    params.m_tempo = 110.0f;

                    params.m_name = "Song1_Cue1";
                    params.AddLayer(m_assetSong1Cue1Drums, m_mixGroupDrums);
                    s_cueSong1Cue1 = musicSystem->CreateCue(params);
                    params.ClearLayers();

                    params.m_name = "Song1_Cue2";
                    params.AddLayer(m_assetSong1Cue2Lead, m_mixGroupLeads);
                    params.AddLayer(m_assetSong1Cue2Drums, m_mixGroupDrums);
                    s_cueSong1Cue2 = musicSystem->CreateCue(params);
                    params.ClearLayers();

                    params.m_name = "Song1_Cue3";
                    params.AddLayer(m_assetSong1Cue3Lead, m_mixGroupLeads);
                    params.AddLayer(m_assetSong1Cue3Bass, m_mixGroupBass);
                    params.AddLayer(m_assetSong1Cue3Drums, m_mixGroupDrums);
                    s_cueSong1Cue3 = musicSystem->CreateCue(params);
                }

                // Transitions
                {
                    rf::TransitionParameters params;

                    rf::TransitionCondition condition1;
                    {
                        DemoState1* data = reinterpret_cast<DemoState1*>(condition1.m_userData);
                        *data = DemoState1::State1;
                        params.m_condition = condition1;
                        params.m_cue = s_cueSong1Cue1;
                        params.m_sync = rf::Sync(rf::Sync::Value::Bar);
                        params.m_name = "To_Song1_Cue1";
                        s_toSong1Cue1 = musicSystem->CreateTransition(params);
                    }

                    rf::TransitionCondition condition2;
                    {
                        DemoState1* data = reinterpret_cast<DemoState1*>(condition2.m_userData);
                        *data = DemoState1::State2;
                        params.m_condition = condition2;
                        params.m_cue = s_cueSong1Cue2;
                        params.m_name = "To_Song1_Cue2";
                        s_toSong1Cue2 = musicSystem->CreateTransition(params);
                    }

                    rf::TransitionCondition condition3;
                    {
                        DemoState1* data = reinterpret_cast<DemoState1*>(condition3.m_userData);
                        *data = DemoState1::State3;
                        params.m_condition = condition3;
                        params.m_cue = s_cueSong1Cue3;
                        params.m_name = "To_Song1_Cue3";
                        s_toSong1Cue3 = musicSystem->CreateTransition(params);
                    }
                }

                s_hasBeenCreated = true;
            };

            const auto Destroy = [this, musicSystem]() {
                if (!s_hasBeenCreated)
                {
                    return;
                }

                musicSystem->DestroyCue(&s_cueSong1Cue1);
                musicSystem->DestroyCue(&s_cueSong1Cue2);
                musicSystem->DestroyCue(&s_cueSong1Cue3);
                musicSystem->DestroyTransition(&s_toSong1Cue1);
                musicSystem->DestroyTransition(&s_toSong1Cue2);
                musicSystem->DestroyTransition(&s_toSong1Cue3);

                s_hasBeenCreated = false;
            };

            Create();

            const DemoState1 states[] = {DemoState1::None, DemoState1::State1, DemoState1::State2, DemoState1::State3};
            const char* stateNames[] = {"None", "State1", "State2", "State3"};
            static int s_index = 0;

            if (ImGui::Combo("State", &s_index, stateNames, 4))
            {
                rf::MusicSystem::UserData userData;
                DemoState1* data = reinterpret_cast<DemoState1*>(userData.m_data);
                *data = states[s_index];
                musicSystem->Play(userData);
            }

            if (ImGui::Button("Stop"))
            {
                s_index = 0;
                musicSystem->Stop();
            }

            if (ImGui::Button("Destroy"))
            {
                s_index = 0;
                Destroy();
            }

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Transition Conditions: Cues"))
        {
            ImGui::Text("You can set a transition condition such that it will only get picked if a current cue is playing.");
            ImGui::Text("In this case, 'Song1_Cue2' (State2) will only play if 'Song1_Cue1' (State1) is playing.");

            static bool s_hasBeenCreated = false;
            static rf::Cue* s_cueSong1Cue1 = nullptr;
            static rf::Cue* s_cueSong1Cue2 = nullptr;
            static rf::Cue* s_cueSong1Cue3 = nullptr;
            static rf::Transition* s_toSong1Cue1 = nullptr;
            static rf::Transition* s_toSong1Cue2 = nullptr;
            static rf::Transition* s_toSong1Cue3 = nullptr;

            enum DemoState2 : int
            {
                None = 20,
                State1,
                State2,
                State3
            };

            const auto Create = [this, musicSystem]() {
                if (s_hasBeenCreated)
                {
                    return;
                }

                // Cues
                {
                    rf::CueParameters params;
                    params.m_meter = rf::Meter(4, 4);
                    params.m_tempo = 110.0f;

                    params.m_name = "Song1_Cue1";
                    params.AddLayer(m_assetSong1Cue1Drums, m_mixGroupDrums);
                    s_cueSong1Cue1 = musicSystem->CreateCue(params);
                    params.ClearLayers();

                    params.m_name = "Song1_Cue2";
                    params.AddLayer(m_assetSong1Cue2Lead, m_mixGroupLeads);
                    params.AddLayer(m_assetSong1Cue2Drums, m_mixGroupDrums);
                    s_cueSong1Cue2 = musicSystem->CreateCue(params);
                    params.ClearLayers();

                    params.m_name = "Song1_Cue3";
                    params.AddLayer(m_assetSong1Cue3Lead, m_mixGroupLeads);
                    params.AddLayer(m_assetSong1Cue3Bass, m_mixGroupBass);
                    params.AddLayer(m_assetSong1Cue3Drums, m_mixGroupDrums);
                    s_cueSong1Cue3 = musicSystem->CreateCue(params);
                }

                // Transitions
                {
                    rf::TransitionParameters params;

                    rf::TransitionCondition condition1;
                    {
                        DemoState2* data = reinterpret_cast<DemoState2*>(condition1.m_userData);
                        *data = DemoState2::State1;
                        params.m_condition = condition1;
                        params.m_cue = s_cueSong1Cue1;
                        params.m_sync = rf::Sync(rf::Sync::Value::Bar);
                        params.m_name = "To_Song1_Cue1";
                        s_toSong1Cue1 = musicSystem->CreateTransition(params);
                    }

                    rf::TransitionCondition condition2;
                    {
                        DemoState2* data = reinterpret_cast<DemoState2*>(condition2.m_userData);
                        *data = DemoState2::State2;
                        condition2.m_cueHandle = s_cueSong1Cue1->GetCueHandle();
                        params.m_condition = condition2;
                        params.m_cue = s_cueSong1Cue2;
                        params.m_name = "To_Song1_Cue2";
                        s_toSong1Cue2 = musicSystem->CreateTransition(params);
                    }

                    rf::TransitionCondition condition3;
                    {
                        DemoState2* data = reinterpret_cast<DemoState2*>(condition3.m_userData);
                        *data = DemoState2::State3;
                        params.m_condition = condition3;
                        params.m_cue = s_cueSong1Cue3;
                        params.m_name = "To_Song1_Cue3";
                        s_toSong1Cue3 = musicSystem->CreateTransition(params);
                    }
                }

                s_hasBeenCreated = true;
            };

            const auto Destroy = [this, musicSystem]() {
                if (!s_hasBeenCreated)
                {
                    return;
                }

                musicSystem->DestroyCue(&s_cueSong1Cue1);
                musicSystem->DestroyCue(&s_cueSong1Cue2);
                musicSystem->DestroyCue(&s_cueSong1Cue3);
                musicSystem->DestroyTransition(&s_toSong1Cue1);
                musicSystem->DestroyTransition(&s_toSong1Cue2);
                musicSystem->DestroyTransition(&s_toSong1Cue3);

                s_hasBeenCreated = false;
            };

            Create();

            const DemoState2 states[] = {DemoState2::None, DemoState2::State1, DemoState2::State2, DemoState2::State3};
            const char* stateNames[] = {"None", "State1", "State2", "State3"};
            static int s_index = 0;

            if (ImGui::Combo("State", &s_index, stateNames, 4))
            {
                rf::MusicSystem::UserData userData;
                DemoState2* data = reinterpret_cast<DemoState2*>(userData.m_data);
                *data = states[s_index];
                musicSystem->Play(userData);
            }

            if (ImGui::Button("Stop"))
            {
                s_index = 0;
                musicSystem->Stop();
            }

            if (ImGui::Button("Destroy"))
            {
                s_index = 0;
                Destroy();
            }

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Transition Conditions: Bars and Beats"))
        {
            ImGui::Text("You can set a transition condition such that it is picked based on the current bar and/or beat.");
            ImGui::Text("Use flags to describe the sort of transition you want with respect to the bars and beats.");
            ImGui::Text("You can also select even or odd bars only.");

            static bool s_hasBeenCreated = false;
            static rf::Cue* s_cueSong1Cue1 = nullptr;
            static rf::Cue* s_cueSong1Cue2 = nullptr;
            static rf::Cue* s_cueSong1Cue3 = nullptr;
            static rf::Cue* s_cueSong1Cue4 = nullptr;
            static rf::Transition* s_toSong1Cue1 = nullptr;
            static rf::Transition* s_toSong1Cue2 = nullptr;
            static rf::Transition* s_toSong1Cue3 = nullptr;
            static rf::Transition* s_toSong1Cue4 = nullptr;

            enum DemoState3 : int
            {
                None = 30,
                State1,
                State2,
                State3,
                State4,
            };

            const auto Create = [this, musicSystem]() {
                if (s_hasBeenCreated)
                {
                    return;
                }

                // Cues
                {
                    rf::CueParameters params;
                    params.m_meter = rf::Meter(4, 4);
                    params.m_tempo = 110.0f;

                    params.m_name = "Song1_Cue1";
                    params.AddLayer(m_assetSong1Cue1Drums, m_mixGroupDrums);
                    s_cueSong1Cue1 = musicSystem->CreateCue(params);
                    params.ClearLayers();

                    params.m_name = "Song1_Cue2";
                    params.AddLayer(m_assetSong1Cue2Lead, m_mixGroupLeads);
                    params.AddLayer(m_assetSong1Cue2Drums, m_mixGroupDrums);
                    s_cueSong1Cue2 = musicSystem->CreateCue(params);
                    params.ClearLayers();

                    params.m_name = "Song1_Cue3";
                    params.AddLayer(m_assetSong1Cue3Lead, m_mixGroupLeads);
                    params.AddLayer(m_assetSong1Cue3Bass, m_mixGroupBass);
                    params.AddLayer(m_assetSong1Cue3Drums, m_mixGroupDrums);
                    s_cueSong1Cue3 = musicSystem->CreateCue(params);
                    params.ClearLayers();

                    params.m_name = "Song1_Cue4";
                    params.AddLayer(m_assetSong1Cue4Lead, m_mixGroupLeads);
                    params.AddLayer(m_assetSong1Cue4Bass, m_mixGroupBass);
                    params.AddLayer(m_assetSong1Cue4Drums, m_mixGroupDrums);
                    s_cueSong1Cue4 = musicSystem->CreateCue(params);
                }

                // Transitions
                {
                    rf::TransitionParameters params;

                    rf::TransitionCondition condition1;
                    {
                        DemoState3* data = reinterpret_cast<DemoState3*>(condition1.m_userData);
                        *data = DemoState3::State1;
                        condition1.m_bar = 2;
                        condition1.m_flags = rf::TransitionCondition::Flag::EqualBars;

                        params.m_condition = condition1;
                        params.m_cue = s_cueSong1Cue1;
                        params.m_sync = rf::Sync(rf::Sync::Value::Bar);
                        params.m_name = "To_Song1_Cue1";
                        s_toSong1Cue1 = musicSystem->CreateTransition(params);
                    }

                    rf::TransitionCondition condition2;
                    {
                        DemoState3* data = reinterpret_cast<DemoState3*>(condition2.m_userData);
                        *data = DemoState3::State2;
                        condition2.m_beat = 3;
                        condition2.m_flags = rf::TransitionCondition::Flag::GreaterThanEqualBeats;

                        params.m_condition = condition2;
                        params.m_cue = s_cueSong1Cue2;
                        params.m_name = "To_Song1_Cue2";
                        s_toSong1Cue2 = musicSystem->CreateTransition(params);
                    }

                    rf::TransitionCondition condition3;
                    {
                        DemoState3* data = reinterpret_cast<DemoState3*>(condition3.m_userData);
                        *data = DemoState3::State3;
                        condition3.m_flags = rf::TransitionCondition::Flag::EvenBars;

                        params.m_condition = condition3;
                        params.m_cue = s_cueSong1Cue3;
                        params.m_name = "To_Song1_Cue3";
                        s_toSong1Cue3 = musicSystem->CreateTransition(params);
                    }

                    rf::TransitionCondition condition4;
                    {
                        DemoState3* data = reinterpret_cast<DemoState3*>(condition4.m_userData);
                        *data = DemoState3::State4;
                        condition4.m_flags = rf::TransitionCondition::Flag::OddBars;

                        params.m_condition = condition4;
                        params.m_cue = s_cueSong1Cue4;
                        params.m_name = "To_Song1_Cue4";
                        s_toSong1Cue4 = musicSystem->CreateTransition(params);
                    }
                }

                s_hasBeenCreated = true;
            };

            const auto Destroy = [this, musicSystem]() {
                if (!s_hasBeenCreated)
                {
                    return;
                }

                musicSystem->DestroyCue(&s_cueSong1Cue1);
                musicSystem->DestroyCue(&s_cueSong1Cue2);
                musicSystem->DestroyCue(&s_cueSong1Cue3);
                musicSystem->DestroyCue(&s_cueSong1Cue4);
                musicSystem->DestroyTransition(&s_toSong1Cue1);
                musicSystem->DestroyTransition(&s_toSong1Cue2);
                musicSystem->DestroyTransition(&s_toSong1Cue3);
                musicSystem->DestroyTransition(&s_toSong1Cue4);

                s_hasBeenCreated = false;
            };

            Create();

            if (ImGui::Button("Play State1"))
            {
                musicSystem->Play(s_toSong1Cue1);
            }

            const DemoState3 states[] = {DemoState3::None, DemoState3::State1, DemoState3::State2, DemoState3::State3, DemoState3::State4};
            const char* stateNames[] = {
                "None", "State1: Valid if Bar == 2", "State2: Valid if Beat >= 3", "State3: Valid if Bar is Even", "State4: Valid if Bar is Odd"};
            static int s_index = 0;

            if (ImGui::Combo("State", &s_index, stateNames, 5))
            {
                rf::MusicSystem::UserData userData;
                DemoState3* data = reinterpret_cast<DemoState3*>(userData.m_data);
                *data = states[s_index];
                musicSystem->Play(userData);
            }

            if (ImGui::Button("Stop"))
            {
                s_index = 0;
                musicSystem->Stop();
            }

            if (ImGui::Button("Destroy"))
            {
                s_index = 0;
                Destroy();
            }

            ImGui::TreePop();
        }
    }
}

void RedFishDemo::Example10_Events()
{
    // You can register callbacks which are called when certain RedFish events happen.

    rf::EventSystem* eventSystem = m_context->GetEventSystem();
    eventSystem->SetUserData(&m_printEventsToConsole);

    eventSystem->RegisterOnBar([](int bar, int beat, void* userData) {
        const bool printToConsole = *static_cast<bool*>(userData);
        if (printToConsole)
        {
            printf("[EVENT] OnBar: %i, %i\n", bar, beat);
        }
    });

    eventSystem->RegisterOnBeat([](int bar, int beat, void* userData) {
        const bool printToConsole = *static_cast<bool*>(userData);
        if (printToConsole)
        {
            printf("[EVENT] OnBeat: %i, %i\n", bar, beat);
        }
    });

    eventSystem->RegisterOnMusicFinished([](void* userData) {
        const bool printToConsole = *static_cast<bool*>(userData);
        if (printToConsole)
        {
            printf("[EVENT] OnMusicFinished\n");
        }
    });
}

void RedFishDemo::StressTest()
{
    ImGui::PushID("StressTest");

    if (ImGui::CollapsingHeader("Stress Test"))
    {
        if (!m_runStressTest)
        {
            if (ImGui::Button("Start"))
            {
                m_runStressTest = true;
                m_stressTestAccumulator = 0.0f;
            }
        }
        else
        {
            if (ImGui::Button("End"))
            {
                m_runStressTest = false;
            }
        }
    }

    ImGui::PopID();
}

void RedFishDemo::FuzzTest()
{
    ImGui::PushID("FuzzTest");

    if (ImGui::CollapsingHeader("Fuzz Test"))
    {
        if (!m_runFuzzTest)
        {
            if (ImGui::Button("Start"))
            {
                m_runFuzzTest = true;
                m_fuzzTestState = FuzzTestState::Wait;
                m_fuzzTestWaitTime = 1.0f;
                printf("[FUZZ TEST] Start\n");
            }
        }
        else
        {
            if (ImGui::Button("End"))
            {
                m_runFuzzTest = false;
                printf("[FUZZ TEST] End\n");
            }
        }
    }

    ImGui::PopID();
}

void RedFishDemo::Mixer()
{
    if (ImGui::CollapsingHeader("Mixer"))
    {
        const auto EditMixGroup = [](const char* name, rf::MixGroup* mixGroup) {
            ImGui::Text(name);
            ImGui::PushID(name);
            const float amplitude = mixGroup->GetCurrentAmplitude();
            ImGui::ProgressBar(amplitude, ImVec2(-1, 0), "");
            float volumeDb = mixGroup->GetVolumeDb();
            ImGui::DragFloat("Volume dB", &volumeDb, 0.01f, -120.0f, 12.0f);
            mixGroup->SetVolumeDb(volumeDb);
            ImGui::PopID();
        };

        if (m_mixGroupTestCreateDestroy)
        {
            EditMixGroup("TestCreateDestroy", m_mixGroupTestCreateDestroy);
        }

        EditMixGroup("Entities", m_mixGroupEntities);
        EditMixGroup("Ambience", m_mixGroupAmbience);
        EditMixGroup("Reverb", m_mixGroupReverb);
        EditMixGroup("Delay", m_mixGroupDelay);
        EditMixGroup("Sounds", m_mixGroupSounds);
        EditMixGroup("Leads", m_mixGroupLeads);
        EditMixGroup("Bass", m_mixGroupBass);
        EditMixGroup("Drums", m_mixGroupDrums);
        EditMixGroup("Music", m_mixGroupMusic);
        EditMixGroup("Test Send To", m_mixGroupTestSendTo);
        EditMixGroup("Master", m_mixGroupMaster);
    }
}

void RedFishDemo::RunStressTest(float dt)
{
    if (!m_runStressTest)
    {
        return;
    }

    static constexpr int k_maxSound = 200;
    static rf::SoundEffect s_sounds[k_maxSound];

    static bool s_createdSounds = false;
    if (!s_createdSounds)
    {
        for (int i = 0; i < k_maxSound; ++i)
        {
            s_sounds[i] = m_soundEffectFootsteps;
        }

        s_createdSounds = true;
    }

    static constexpr float k_piOverTwo = 1.57079632679f;
    const float x = 0.5f * sin(m_stressTestAccumulator - k_piOverTwo) + 0.5f;

    const auto Play = []() {
        const int randNum = rand() % k_maxSound;
        s_sounds[randNum].Play();
    };

    const auto Lerp = [](float a, float b, float t) -> float { return a + t * (b - a); };

    Play();
    Play();

    static constexpr float k_minDb = -30.0f;
    static constexpr float k_maxDb = -10.0f;
    const float db = Lerp(k_minDb, k_maxDb, x);
    const float pitch = Lerp(0.75f, 1.75f, x);
    const float pan = Lerp(-1.0f, 1.0f, x);

    for (int i = 0; i < k_maxSound; ++i)
    {
        rf::PositioningParameters params;
        params.m_enable = true;
        params.m_panAngle = pan;
        s_sounds[i].SetPositioningParameters(params);
        s_sounds[i].SetVolumeDb(db);
        s_sounds[i].SetPitch(pitch);
    }

    m_stressTestAccumulator += dt;
}

void RedFishDemo::RunFuzzTest(float dt)
{
    static rf::SoundEffect s_fuzzSound = m_soundEffectFootsteps;

    static rf::Cue* s_cueSong1Cue1 = nullptr;
    static rf::Cue* s_cueSong1Cue2 = nullptr;
    static rf::Cue* s_cueSong1Cue3 = nullptr;
    static rf::Cue* s_cueSong1Cue4 = nullptr;
    static rf::Cue* s_cueSong1Cue5 = nullptr;
    static rf::Cue* s_cueSong1Cue6 = nullptr;
    static rf::Cue* s_cueSong1Cue7 = nullptr;
    static rf::Cue* s_cueSong1Cue8 = nullptr;
    static rf::Cue* s_cueSong1Cue9 = nullptr;
    static rf::Cue* s_cueSong1Cue10 = nullptr;
    static rf::Cue* s_cueSong1Cue11 = nullptr;
    static rf::Cue* s_cueSong1Stinger = nullptr;
    static rf::Stinger* s_song1Stinger = nullptr;
    static rf::Transition* s_toSong1Cue1 = nullptr;
    static rf::Transition* s_toSong1Cue2 = nullptr;
    static rf::Transition* s_toSong1Cue3 = nullptr;
    static rf::Transition* s_toSong1Cue4 = nullptr;
    static rf::Transition* s_toSong1Cue5 = nullptr;
    static rf::Transition* s_toSong1Cue6 = nullptr;
    static rf::Transition* s_toSong1Cue7 = nullptr;
    static rf::Transition* s_toSong1Cue8 = nullptr;
    static rf::Transition* s_toSong1Cue9 = nullptr;
    static rf::Transition* s_toSong1Cue10 = nullptr;
    static rf::Transition* s_toSong1Cue11 = nullptr;

    rf::MusicSystem* musicSystem = m_context->GetMusicSystem();

    const auto Create = [this, musicSystem]() {
        if (s_cueSong1Cue1)
        {
            return;
        }

        // Cues
        {
            rf::CueParameters params;
            params.m_meter = rf::Meter(4, 4);
            params.m_tempo = 110.0f;

            params.m_name = "Song1_Cue1";
            params.AddLayer(m_assetSong1Cue1Drums, m_mixGroupDrums);
            s_cueSong1Cue1 = musicSystem->CreateCue(params);
            params.ClearLayers();

            params.m_name = "Song1_Cue2";
            params.AddLayer(m_assetSong1Cue2Lead, m_mixGroupLeads);
            params.AddLayer(m_assetSong1Cue2Drums, m_mixGroupDrums);
            s_cueSong1Cue2 = musicSystem->CreateCue(params);
            params.ClearLayers();

            params.m_name = "Song1_Cue3";
            params.AddLayer(m_assetSong1Cue3Lead, m_mixGroupLeads);
            params.AddLayer(m_assetSong1Cue3Bass, m_mixGroupBass);
            params.AddLayer(m_assetSong1Cue3Drums, m_mixGroupDrums);
            s_cueSong1Cue3 = musicSystem->CreateCue(params);
            params.ClearLayers();

            params.m_name = "Song1_Cue4";
            params.AddLayer(m_assetSong1Cue4Lead, m_mixGroupLeads);
            params.AddLayer(m_assetSong1Cue4Bass, m_mixGroupBass);
            params.AddLayer(m_assetSong1Cue4Drums, m_mixGroupDrums);
            s_cueSong1Cue4 = musicSystem->CreateCue(params);
            params.ClearLayers();

            params.m_name = "Song1_Cue5";
            params.AddLayer(m_assetSong1Cue5Lead, m_mixGroupLeads);
            params.AddLayer(m_assetSong1Cue5Drums, m_mixGroupDrums);
            s_cueSong1Cue5 = musicSystem->CreateCue(params);
            params.ClearLayers();

            params.m_name = "Song1_Cue6";
            params.AddLayer(m_assetSong1Cue6Lead, m_mixGroupLeads);
            params.AddLayer(m_assetSong1Cue6Bass, m_mixGroupBass);
            params.AddLayer(m_assetSong1Cue6Drums, m_mixGroupDrums);
            s_cueSong1Cue6 = musicSystem->CreateCue(params);
            params.ClearLayers();

            params.m_name = "Song1_Cue7";
            params.AddLayer(m_assetSong1Cue7Bass, m_mixGroupBass);
            params.AddLayer(m_assetSong1Cue7Drums, m_mixGroupDrums);
            s_cueSong1Cue7 = musicSystem->CreateCue(params);
            params.ClearLayers();

            params.m_name = "Song1_Cue8";
            params.AddLayer(m_assetSong1Cue8Lead, m_mixGroupLeads);
            params.AddLayer(m_assetSong1Cue8Bass, m_mixGroupBass);
            params.AddLayer(m_assetSong1Cue8Drums, m_mixGroupDrums);
            s_cueSong1Cue8 = musicSystem->CreateCue(params);
            params.ClearLayers();

            params.m_name = "Song1_Cue9";
            params.AddLayer(m_assetSong1Cue9Lead, m_mixGroupLeads);
            params.AddLayer(m_assetSong1Cue9Bass, m_mixGroupBass);
            params.AddLayer(m_assetSong1Cue9Drums, m_mixGroupDrums);
            s_cueSong1Cue9 = musicSystem->CreateCue(params);
            params.ClearLayers();

            params.m_name = "Song1_Cue10";
            params.AddLayer(m_assetSong1Cue10Lead, m_mixGroupLeads);
            params.AddLayer(m_assetSong1Cue10Bass, m_mixGroupBass);
            params.AddLayer(m_assetSong1Cue10Drums, m_mixGroupDrums);
            s_cueSong1Cue10 = musicSystem->CreateCue(params);
            params.ClearLayers();

            params.m_name = "Song1_Cue11";
            params.AddLayer(m_assetSong1Cue11Lead, m_mixGroupLeads);
            params.AddLayer(m_assetSong1Cue11Bass, m_mixGroupBass);
            params.AddLayer(m_assetSong1Cue11Drums, m_mixGroupDrums);
            s_cueSong1Cue11 = musicSystem->CreateCue(params);
            params.ClearLayers();

            params.m_meter = rf::Meter(4, 4);
            params.m_tempo = 110.0f;
            params.m_name = "Song1_Stinger";
            params.AddLayer(m_assetSong1Stinger, m_mixGroupDrums);
            s_cueSong1Stinger = musicSystem->CreateCue(params);
        }

        // Stingers
        {
            rf::StingerParameters params;
            params.m_cue = s_cueSong1Stinger;
            params.m_sync = rf::Sync(rf::Sync::Value::Bar);
            params.m_gainDb = -3.0f;
            params.m_name = "Stinger_Song1";
            s_song1Stinger = musicSystem->CreateStinger(params);
        }

        // Transitions
        {
            rf::TransitionParameters params;

            params.m_cue = s_cueSong1Cue1;
            params.m_sync = rf::Sync(rf::Sync::Value::Bar);
            params.m_name = "To_Song1_Cue1";
            s_toSong1Cue1 = musicSystem->CreateTransition(params);

            params.m_cue = s_cueSong1Cue2;
            params.m_name = "To_Song1_Cue2";
            s_toSong1Cue2 = musicSystem->CreateTransition(params);

            params.m_cue = s_cueSong1Cue3;
            params.m_name = "To_Song1_Cue3";
            s_toSong1Cue3 = musicSystem->CreateTransition(params);

            params.m_cue = s_cueSong1Cue4;
            params.m_sync = rf::Sync(rf::Sync::Value::Quarter);
            params.m_name = "To_Song1_Cue4";
            s_toSong1Cue4 = musicSystem->CreateTransition(params);

            params.m_cue = s_cueSong1Cue5;
            params.m_sync = rf::Sync(rf::Sync::Value::Queue);
            params.m_name = "To_Song1_Cue5";
            s_toSong1Cue5 = musicSystem->CreateTransition(params);

            params.m_cue = s_cueSong1Cue6;
            params.m_sync = rf::Sync(rf::Sync::Value::Bar);
            params.m_name = "To_Song1_Cue6";
            s_toSong1Cue6 = musicSystem->CreateTransition(params);

            params.m_cue = s_cueSong1Cue7;
            params.m_name = "To_Song1_Cue7";
            s_toSong1Cue7 = musicSystem->CreateTransition(params);

            params.m_cue = s_cueSong1Cue8;
            params.m_name = "To_Song1_Cue8";
            s_toSong1Cue8 = musicSystem->CreateTransition(params);

            params.m_cue = s_cueSong1Cue9;
            params.m_name = "To_Song1_Cue9";
            s_toSong1Cue9 = musicSystem->CreateTransition(params);

            params.m_cue = s_cueSong1Cue10;
            params.m_name = "To_Song1_Cue10";
            s_toSong1Cue10 = musicSystem->CreateTransition(params);

            params.m_cue = s_cueSong1Cue11;
            params.m_playCount = 1;
            params.m_stinger = s_song1Stinger;
            params.m_name = "To_Song1_Cue11";
            s_toSong1Cue11 = musicSystem->CreateTransition(params);
        }
    };

    const auto Destroy = [this, musicSystem]() {
        musicSystem->DestroyCue(&s_cueSong1Cue1);
        musicSystem->DestroyCue(&s_cueSong1Cue2);
        musicSystem->DestroyCue(&s_cueSong1Cue3);
        musicSystem->DestroyCue(&s_cueSong1Cue4);
        musicSystem->DestroyCue(&s_cueSong1Cue5);
        musicSystem->DestroyCue(&s_cueSong1Cue6);
        musicSystem->DestroyCue(&s_cueSong1Cue7);
        musicSystem->DestroyCue(&s_cueSong1Cue8);
        musicSystem->DestroyCue(&s_cueSong1Cue9);
        musicSystem->DestroyCue(&s_cueSong1Cue10);
        musicSystem->DestroyCue(&s_cueSong1Cue11);
        musicSystem->DestroyCue(&s_cueSong1Stinger);
        musicSystem->DestroyStinger(&s_song1Stinger);
        musicSystem->DestroyTransition(&s_toSong1Cue1);
        musicSystem->DestroyTransition(&s_toSong1Cue2);
        musicSystem->DestroyTransition(&s_toSong1Cue3);
        musicSystem->DestroyTransition(&s_toSong1Cue4);
        musicSystem->DestroyTransition(&s_toSong1Cue5);
        musicSystem->DestroyTransition(&s_toSong1Cue6);
        musicSystem->DestroyTransition(&s_toSong1Cue7);
        musicSystem->DestroyTransition(&s_toSong1Cue8);
        musicSystem->DestroyTransition(&s_toSong1Cue9);
        musicSystem->DestroyTransition(&s_toSong1Cue10);
        musicSystem->DestroyTransition(&s_toSong1Cue11);
    };

    if (!m_runFuzzTest)
    {
        Destroy();
        return;
    }

    Create();

    const auto PickTimeAndWait = [this]() {
        const int randNum = rand() % 3;
        m_fuzzTestWaitTime = static_cast<float>(randNum);
        m_fuzzTestState = FuzzTestState::Wait;
    };

    switch (m_fuzzTestState)
    {
        case FuzzTestState::Wait:
        {
            if (m_fuzzTestWaitTime <= 0.0f)
            {
                const int randNum = rand() % 100;
                if (randNum >= 50)
                {
                    m_fuzzTestState = FuzzTestState::Music;
                }
                else if (randNum <= 10)
                {
                    m_fuzzTestState = FuzzTestState::ForceDestroy;
                }
                else
                {
                    m_fuzzTestState = FuzzTestState::Sound;
                }
            }
            else
            {
                m_fuzzTestWaitTime -= dt;
            }
            break;
        }
        case FuzzTestState::ForceDestroy:
        {
            printf("[FUZZ TEST] Destroy\n");
            Destroy();
            PickTimeAndWait();
            break;
        }
        case FuzzTestState::Sound:
        {
            printf("[FUZZ TEST] Play Sound\n");
            s_fuzzSound.Play();
            PickTimeAndWait();
            break;
        }
        case FuzzTestState::Music:
        {
            const int randNum = rand() % 100;
            const bool playStinger = randNum >= 75;

            if (playStinger)
            {
                printf("[FUZZ TEST] Play Stinger\n");
                musicSystem->Play(s_song1Stinger);
            }
            else
            {
                if (randNum < 10)
                {
                    const float randPercent = (rand() % 100) / 100.0f;
                    const auto Lerp = [](float a, float b, float t) -> float { return a + t * (b - a); };
                    const float db = Lerp(-20.0f, 0.0f, randPercent);
                    rf::MixerSystem* mixerSystem = m_context->GetMixerSystem();
                    const rf::MixGroup* layers[] = {m_mixGroupLeads, m_mixGroupBass};
                    mixerSystem->FadeMixGroups(layers, 2, db, rf::Sync(rf::Sync::Value::Bar), rf::Sync(0.1f), s_song1Stinger);
                    printf("[FUZZ TEST] Layer Fading Music To %fdB\n", db);
                }
                else
                {
                    if (randNum < 25)
                    {
                        printf("[FUZZ TEST] Stop Music\n");
                        musicSystem->Stop();
                    }
                    else
                    {
                        const rf::Transition* k_transitions[] = {
                            s_toSong1Cue1,
                            s_toSong1Cue2,
                            s_toSong1Cue3,
                            s_toSong1Cue4,
                            s_toSong1Cue5,
                            s_toSong1Cue6,
                            s_toSong1Cue7,
                            s_toSong1Cue8,
                            s_toSong1Cue9,
                            s_toSong1Cue10,
                            s_toSong1Cue11,
                        };

                        const int randomIndex = rand() % 11;
                        printf("[FUZZ TEST] Play Music Transition %i\n", randomIndex + 1);
                        musicSystem->Play(k_transitions[randomIndex]);
                    }
                }
            }

            PickTimeAndWait();
            break;
        }
        default: break;
    }
}
