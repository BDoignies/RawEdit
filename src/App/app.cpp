#include "app.h"
#include "nfd.h"
#include "raylib.h"
#include "raymath.h"
#include "rlImGui.h"
#include "imgui.h"
#include "spdlog/spdlog.h"
#include "imgui_internal.h"

#include <iostream>

#define EDITOR_WINDOW "Editor"

int RaylibFormatFromImage(RawEdit::core::ImagePtr img) 
{
    if (img->channels == 1)
    {
        if (img->bytes == 1)
            return PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
        else if (img->bytes == 2)
            return PIXELFORMAT_UNCOMPRESSED_R16;
    }
    else if (img->channels == 3)
    {
        if (img->bytes == 1)
            return PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    else if (img->bytes == 2)
            return PIXELFORMAT_UNCOMPRESSED_R16G16B16;
    }
    spdlog::warn("Unknown pixel format for image");
    return 0;
}

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
}

int App::run() 
{
    while (!WindowShouldClose()) 
    {
        OnProcess();
        BeginDrawing();
        {
            ClearBackground(DARKGRAY);

            OnEvent();
            OnUI();
            OnRender();
        }
        EndDrawing();
    }
    return true;
}

void App::SelectImageToDisplay(uint32_t id)
{
    imageId = id;
    imageRect.x = 0;
    imageRect.y = 0;
    imageRect.width  = textures[id].width;
    imageRect.height = textures[id].height;
}

void App::OnProcess()
{
    for (auto it = loaders.begin(); it != loaders.end(); )
    {
        auto state = it->wait_for(std::chrono::milliseconds(0));
        if (state == std::future_status::ready) 
        {
            auto result = it->get();
            if (result) 
            {
                if ((*result)->data != nullptr)
                {
                    Image im = {
                        .data    = (*result)->data,
                        .width   = (int)(*result)->width, 
                        .height  = (int)(*result)->height, 
                        .mipmaps = 1,
                        .format  = RaylibFormatFromImage(*result)
                    };
                    images.push_back(*result);
                    textures.push_back(LoadTextureFromImage(im));

                    SelectImageToDisplay(textures.size() - 1);
                }
                else 
                {
                    spdlog::error("Unknown error");
                }
            }
            else 
            {
                spdlog::error("{}", result.error().errorString);
            }
            it = loaders.erase(it);
        }
        else 
        {
            ++it;
        }
    }
}

void App::OnEvent() 
{
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


void App::OnRender()
{
    if (imageId >= textures.size()) 
        return;
    
    const auto& texture = textures[imageId];
    const float aspect  = texture.width / texture.height;
    
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

    DrawTexturePro(texture, imageRect, dest, Vector2Zero(), 0.f, WHITE); 
}

void App::OnUI()
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

        MainMenu();
        ParamMenu();
        // ImGui::ShowDebugLogWindow();
    }
    rlImGuiEnd();
}

void App::MainMenu()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open"))
            {
                std::vector<std::string> files = OpenDialog();
                for (const auto& p : files)
                    AsyncOpenFile(p);
            }
            ImGui::EndMenu();
        }
    }
    ImGui::EndMainMenuBar();
}

void App::ParamMenu()
{
    const unsigned int screenWidth  = GetScreenWidth();
    const unsigned int screenHeight = GetScreenHeight();

    ImGui::Begin(EDITOR_WINDOW);
    {
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

void App::AsyncOpenFile(const std::string& path)
{
    using namespace RawEdit::algorithm;
    using namespace RawEdit::core;

    loaders.push_back(
        std::async(std::launch::async,
        [=]() -> Failable<ImagePtr> {
            spdlog::info("Loading {}", path);
            return OpenImage(path.c_str());
        }
    ));
}

App::~App()
{
    rlImGuiShutdown();
    CloseWindow();
}


