#pragma once

#include <future>
#include <string>
#include <vector>

#include "raylib.h"
#include "RawEdit/RawEdit.h"

class App
{
public:
    App();

    int run();

    ~App();
private:
    void OnProcess();
    void OnEvent();
    void OnUI();
    void OnRender();

    void MainMenu();
    void ParamMenu();

    void SelectImageToDisplay(uint32_t id);

    std::vector<std::string> OpenDialog();
    
    void AsyncOpenFile(const std::string& path);
private:
    Rectangle GetAvailableRegion() const;
    Rectangle ComputeMainImageArea() const;
    Rectangle ComputeMainImageSrcArea(Rectangle area);
private: // Image data
    std::vector<RawEdit::core::ImagePtr> images;
    std::vector<Texture2D> textures;

    std::vector<std::string> paths;

    // Image loaders
    std::vector<std::future<RawEdit::core::Failable<RawEdit::core::ImagePtr>>> loaders;
private: // Display Image data
    uint32_t imageId = 0;

    Vector2 imagePos{0};
    float   imageZoom = 1.f;
};
