#pragma once

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
private:
};
