#pragma once

#include <future>
#include <string>
#include <vector>

#include "raylib.h"
#include "RawEdit/RawEdit.h"
#include "raweditraylib.h"
#include "imagemanager.h"

class App
{
public:
    App();

    int run();

    ~App();
private:
    void OnProcess(float dt);
    void OnEvent();
    void OnUI(float dt);
    void OnRender(float dt);
    
    void LogMenu(float dt);
    void MainMenu(float dt);
    void ParamMenu(float dt);

    void DisplayParam(RawEdit::Param& param);

    std::vector<std::string> OpenDialog();
private:
    Rectangle GetAvailableRegion() const;
    Rectangle ComputeMainImageArea() const;
    Rectangle ComputeMainImageSrcArea(Rectangle area);
private: // Image data
    ImageManager manager;
private: // Display Image data
    Vector2 imagePos{0};
    float   imageZoom = 1.f;
private:
    std::vector<std::string> logs;
};
