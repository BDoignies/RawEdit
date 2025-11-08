#include "app.h"
#include "raylib.h"
#include "raymath.h"
#include "rlImGui.h"
#include "imgui.h"

#include <iostream>

App::App() 
{
    const unsigned int screenWidth  = 1280;
    const unsigned int screenHeight = 1080;
    
    // Library setup
    InitWindow(screenWidth, screenHeight, "RawEdit");
    rlImGuiSetup(true);
}

int App::run() 
{
    while (!WindowShouldClose()) 
    {
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

void App::OnEvent() 
{
}

void App::OnRender()
{

    if (currentImage)
    {
        Vector2 pos = {
            .x = 10,
            .y = 10
        };
        DrawTextureEx(*currentImage, pos, 0.f, 0.1f, WHITE);
        DrawRectangleLines(pos.x, pos.y, 6264 / 10, 4180 / 10, WHITE);
    }
}

void App::OnUI()
{
    rlImGuiBegin();
    {
        // bool show = true;
        // ImGui::ShowDemoWindow(&show);

        ImGui::Begin("Debug");
        {
            if (ImGui::Button("Load test image"))
            {
                const char* path = "../tests/photo.cr3";

                RawEdit::core::Image im;
                RawEdit::core::Error err = im.open(path);

                if (err) 
                {
                    std::cerr << err.errorString << std::endl;
                }
                else
                {
                    // loadedImages.push_back(std::move(im));
                    if (currentImage)
                    {
                        UnloadTexture(*currentImage);
                        delete currentImage;
                    }

                    Image rlImage = {
                         .data   = (void*)im.buffer(),
                         .width  = (int)  im.width(),
                         .height = (int)  im.height(),
                         .mipmaps = 1,
                         .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8
                     };

                    // TODO: This is not working
                    currentImage = new Texture(LoadTextureFromImage(rlImage));
                }
            }
            
            if (ImGui::CollapsingHeader("Images"))
            {
                for (unsigned int i = 0; i < loadedImages.size(); ++i)
                {
                    ImGui::Text("%s", loadedImages[i].str().c_str());
                }
            }
        }
        ImGui::End();
    }
    rlImGuiEnd();
}


App::~App()
{
    rlImGuiShutdown();
    CloseWindow();
}

