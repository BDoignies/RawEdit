#pragma once

#include <future>

#include "raylib.h"
#include "RawEdit/RawEdit.h"

class App
{
public:
    App();

    int run();

    ~App();
private:
    void OnEvent();
    void OnUI();
    void OnRender();

    void MainMenu();
    void ParamMenu();
    void Debug();

    void AsyncOpenFile(uint32_t idx, const char* path);
private:
    std::vector<RawEdit::core::RawImage> images;
    std::vector<Texture2D> textures;

    std::vector<std::string> paths;

    // Image loaders
    std::vector<std::future<RawEdit::core::Failable<RawEdit::core::RawImage>>> loaders;
};

