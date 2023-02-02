#include "platform.hpp"

#include <../sdl/include/SDL.h>
#include <../sdl/include/SDL_opengl.h>
#include <imgui.h>
#include <imgui_impl_opengl2.h>
#include <imgui_impl_sdl.h>

namespace Platform
{
static SDL_Window* s_window = nullptr;
static SDL_AudioDeviceID s_deviceId = 0;

void Initialize()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
    }

    SDL_GLContext glContext;

    // Setup window
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    s_window = SDL_CreateWindow("RedFish Demo Application", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    glContext = SDL_GL_CreateContext(s_window);
    SDL_GL_MakeCurrent(s_window, glContext);
    // Enable vsync
    SDL_GL_SetSwapInterval(1);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(s_window, glContext);
    ImGui_ImplOpenGL2_Init();
}

void CreateAudioDevice(void (*callback)(void* userdata, uint8_t* stream, int len), int sampleRate, int bufferSize, int channels)
{
    SDL_AudioSpec audioSpec;
    SDL_AudioSpec want;
    SDL_memset(&want, 0, sizeof(want));
    want.samples = static_cast<uint16_t>(bufferSize);
    want.freq = sampleRate;
    want.format = AUDIO_F32;
    want.channels = static_cast<uint8_t>(channels);
    want.callback = callback;
    want.userdata = nullptr;
    const int flags = SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_FORMAT_CHANGE;
    s_deviceId = SDL_OpenAudioDevice(NULL, 0, &want, &audioSpec, flags);
    SDL_PauseAudioDevice(s_deviceId, 0);
}

void DestroyAudioDevice()
{
    SDL_CloseAudioDevice(s_deviceId);
}

void LockAudioDevice()
{
    SDL_LockAudioDevice(s_deviceId);
}

void UnlockAudioDevice()
{
    SDL_UnlockAudioDevice(s_deviceId);
}

void Tick(const std::function<void(float dt)>& callback)
{
    ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImGuiIO& io = ImGui::GetIO();

    bool done = false;
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
            {
                done = true;
            }
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(s_window))
            {
                done = true;
            }
        }

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL2_NewFrame(s_window);
        ImGui::NewFrame();

        // About 60 fps
        static constexpr float k_fakeDt = 0.0166666666666667f;
        callback(k_fakeDt);

        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(s_window);

        static constexpr int k_sleepTime = static_cast<int>(1000 * k_fakeDt);
        Sleep(k_sleepTime);
    }
}

}  // namespace Platform
