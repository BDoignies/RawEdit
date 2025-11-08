#include "app.h"
#include "raylib.h"
#include "raymath.h"
#include "rlImGui.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <iostream>

App::App() 
{
    const unsigned int screenWidth  = 1280;
    const unsigned int screenHeight = 1080;
    
    // Library setup
    InitWindow(screenWidth, screenHeight, "RawEdit");
    rlImGuiSetup(true);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
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

            ImGui::DockBuilderDockWindow("Editor", rightId);
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

    ImGui::Begin("Editor");
    {
    }
    ImGui::End();
}

App::~App()
{
    rlImGuiShutdown();
    CloseWindow();
}

