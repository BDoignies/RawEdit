#include "nfd.h"
#include "raylib.h"
#include "raymath.h"
#include "rlImGui.h"
#include "imgui.h"
#include "spdlog/spdlog.h"
#include "imgui_internal.h"

#include "app.h"
#include "utils.h"

#include <iostream>
#include <cmath>

#define EDITOR_WINDOW "Editor"

App::App() 
{
    const unsigned int screenWidth  = 1280;
    const unsigned int screenHeight = 1080;
    
    // Library setup
    NFD_Init();
    InitWindow(screenWidth, screenHeight, "RawEdit");
    rlImGuiSetup(true);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glDebugOutput, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

    RawEdit::core::ShaderManager::SetShaderPath(exeDirectory() / "shaders/");

    const auto shader = RawEdit::core::ShaderManager::GetShader("emptyshader");
    shader->Bind();
    shader->AddUniform("inTex");
    shader->AddUniform("outTex");
    shader->AddUniform("exposure");
}

int App::run() 
{
    float time = 0.f;
    while (!WindowShouldClose()) 
    {
        const float current = GetTime();
        const float dt = current - time;

        OnProcess(dt);
        OnEvent();
        BeginDrawing();
        {
            ClearBackground(DARKGRAY);

            OnUI(dt);
            OnRender(dt);
        }
        EndDrawing();

        time = current;
    }
    return true;
}

void App::OnProcess(float dt)
{
    manager.Update();
    for (auto err : manager.pullErrors())
    {
        if (err)
            spdlog::error(err.errorString);
    }
}

void App::OnEvent()
{
    auto im = manager.CurrentImage();
    if (im != nullptr)
    {
        const Vector2 pos = GetMousePosition();
        const Rectangle area = ComputeMainImageArea();
        if (CheckCollisionPointRec(pos, area))
        {
            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
            {
                auto mask = manager.CurrentMask();
                const float w = mask->GetMask()->width;
                const float h = mask->GetMask()->height;
                const float ax = (pos.x - area.x) / (float)area.width;
                const float ay = (pos.y - area.y) / (float)area.height;
                const float cW = std::min(w / imageZoom, w);
                const float cH = std::min(h / imageZoom, h);
                
                if (mask->GetMaskCount() < 1)
                    mask->NewMask();

                mask->Circle(0, imagePos.x + ax * cW, imagePos.y + ay * cH, 15.f);
                mask->Update();
            }
        }
    }
}


Rectangle App::GetAvailableRegion() const
{
    const uint32_t padding = 50;

    int x, y, w, h;
    x = y = 0;
    w = GetScreenWidth();
    h = GetScreenHeight();

    ImGuiWindow* menu = ImGui::FindWindowByName("##MainMenuBar");
    ImGuiWindow* edit = ImGui::FindWindowByName(EDITOR_WINDOW);
    
    if (menu) y += menu->Size.y;
    if (edit) w -= edit->Size.x;

    // Padding will be substracted after
    w = std::max(w, 2 * (int)padding);
    h = std::max(h, 2 * (int)padding);

    return Rectangle {
        .x = (float)(x + padding),
        .y = (float)(y + padding),
        .width  = (float)(w - 2 * padding),
        .height = (float)(h - 2 * padding)
    };
}

Rectangle App::ComputeMainImageArea() const
{
    const auto& texture = *manager.CurrentRLTexture();
    const Rectangle available = GetAvailableRegion();
    Rectangle dest = available;
    if (texture.width > texture.height)
    {
        int tmpHeight = dest.width * texture.height / texture.width;
        if (tmpHeight > dest.height)
            dest.width = dest.height * texture.width / texture.height;
        else
            dest.height = tmpHeight;
    }
    else 
    {
        int tmpWidth = dest.height * texture.width / texture.height;
        if (tmpWidth > dest.width)
            dest.height = dest.width * texture.height / texture.width;
        else
            dest.width = tmpWidth;
    }
    // Update position
    dest.x += 0.5f * (available.width  - dest.width);
    dest.y += 0.5f * (available.height - dest.height);
    
    return dest;
}

Rectangle App::ComputeMainImageSrcArea(Rectangle area) 
{
    const float maxZoom = 1000.f;
    const float scrollSpeed = 5.f;
    const float zoomSpeed = 0.2f;
    const auto& texture = *manager.CurrentRLTexture();

    const Vector2 pos = GetMousePosition();
    const Vector2 delta = GetMouseDelta();
    const Vector2 topLeft = Vector2{.x = area.x, .y = area.y};
    const Vector2 texSize = {.x = (float)texture.width, .y = (float)texture.height};
    const float wheel = GetMouseWheelMove();
    if (CheckCollisionPointRec(pos, area))
    {
        if (wheel != 0)
        {
            const float oldZoom = imageZoom;
            const float newZoom = std::exp(std::log(imageZoom) + zoomSpeed * wheel);
            
            if (newZoom < maxZoom)
            {
                const float ax = (pos.x - area.x) / area.width ;
                const float ay = (pos.y - area.y) / area.height;
                const float cW = std::min(texSize.x / oldZoom, texSize.x);
                const float cH = std::min(texSize.y / oldZoom, texSize.y);
                const float nW = std::min(texSize.x / newZoom, texSize.x);
                const float nH = std::min(texSize.y / newZoom, texSize.y);
                
                imagePos.x -= ax * (nW - cW);
                imagePos.y -= ay * (nH - cH);
                imageZoom = newZoom;
            }
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            const Vector2 scaledDelta = Vector2Scale(delta, -scrollSpeed / imageZoom);
            imagePos = Vector2Add(imagePos, scaledDelta);
        }
    }
    imageZoom  = Clamp(imageZoom, 1, maxZoom);
    
    const float width  = std::min(texSize.x / imageZoom, texSize.x);
    const float height = std::min(texSize.y / imageZoom, texSize.y);

    imagePos.x = Clamp(imagePos.x, 0, texSize.x - width);
    imagePos.y = Clamp(imagePos.y, 0, texSize.y - height);
    const float x = imagePos.x;
    const float y = imagePos.y;

    return Rectangle {
        .x = x, .y = y, .width = width, .height = height
    };
}

void App::OnRender(float dt)
{
    auto im = manager.CurrentImage();
    if (im != nullptr)
    {
        const auto& texture = *manager.CurrentRLTexture();
        const float aspect  = texture.width / texture.height;
        
        const Rectangle dest = ComputeMainImageArea();
        const Rectangle src  = ComputeMainImageSrcArea(dest);
        
        const auto shader = RawEdit::core::ShaderManager::GetShader("emptyshader");
        
        shader->Bind();
            shader->ComputeDispatchSizes(texture.width, texture.height);
            shader->SetUniform("inTex" , im->gpuImage   ,  true, false);
            shader->SetUniform("outTex", im->workingCopy, false, true);
            shader->SetUniform("exposure", tmpExposure);
        shader->RunAndWait();

        DrawTexturePro(texture, src, dest, Vector2Zero(), 0.f, WHITE); 
        
        auto mask = manager.CurrentMask();
        if (mask != nullptr)
        {
            auto maskIm = mask->GetMask();
            
            DrawTexturePro(ConvertToRaylibTexture(maskIm), src, dest, Vector2Zero(), 0.f, WHITE); 
        }
    }
}

void App::OnUI(float dt)
{
    static bool init = true;
    rlImGuiBegin();
    { 
        // Main DockSpace
        ImGuiID dockId = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        if (init)
        {
            init = false;

            ImGui::DockBuilderAddNode(dockId, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockId, ImGui::GetMainViewport()->Size);
            ImGuiID leftId, rightId;
            ImGui::DockBuilderSplitNode(dockId, ImGuiDir_Right, 0.25f, &rightId, &leftId);

            ImGui::DockBuilderDockWindow(EDITOR_WINDOW, rightId);
        }

        MainMenu(dt);
        ParamMenu(dt);
        ImGui::ShowDebugLogWindow();
    }
    rlImGuiEnd();
}

void App::MainMenu(float dt)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open"))
            {
                std::vector<std::string> files = OpenDialog();
                for (const auto& p : files)
                    manager.AddImage(p);
            }
            ImGui::EndMenu();
        }
    }
    ImGui::EndMainMenuBar();
}

void App::ParamMenu(float dt)
{
    static int fpsAvg = 0;
    static int fpsCount = 0;
    const int fps = static_cast<int>(1.f / dt);

    fpsAvg = fpsAvg + (fps - fpsAvg) / ++fpsCount;

    const unsigned int screenWidth  = GetScreenWidth();
    const unsigned int screenHeight = GetScreenHeight();
    ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_DefaultOpen;
    ImGui::Begin(EDITOR_WINDOW);
    {
        if (ImGui::TreeNodeEx("Performance", flag))
        {
            static const char* rescaleMethods[]{
                "None", "Linear", "Bilinear", "Bicubic"
            };
            ImGui::Text("FPS: %d (%d)", fpsAvg, fps);
            ImGui::Text("Imaged loaded: %d", manager.NbImageLoaded());
            ImGui::Text("Imaged loading: %d", manager.NbImageLoading());
            ImGui::Text("Ram: %dM / VRam: %dM", manager.RamUsage(), manager.VRamUsage());
            
            float factor = manager.GetResizeFactor();
            ImGui::SliderFloat("Resize Factor", &factor, 0.f, 1.f);
            manager.SetResizeFactor(factor);
            if (ImGui::Button("Reload all"))
                manager.Reload();
            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Exposure", flag))
        {
            ImGui::SliderFloat("Exposure", &tmpExposure, 0.f, 10.f);
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

std::vector<std::string> App::OpenDialog()
{
    const nfdpathset_t* outPaths;
    nfdresult_t result = NFD_OpenDialogMultiple(&outPaths, nullptr, 0, nullptr);

    if (result == NFD_OKAY) 
    {
        nfdpathsetsize_t numPaths;
        NFD_PathSet_GetCount(outPaths, &numPaths);

        std::vector<std::string> paths(numPaths);
        for (size_t i = 0; i < numPaths; ++i)
        {
            nfdchar_t* path;
            NFD_PathSet_GetPathU8(outPaths, i, &path);

            paths[i] = path;
            NFD_PathSet_FreePath(path);
        }
        return paths;
    }

    return {};
}

App::~App()
{
    manager.Clear();
    rlImGuiShutdown();
    CloseWindow();
}


