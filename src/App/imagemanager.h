#pragma once

#include <string_view>
#include <algorithm>
#include <future>
#include <vector>
#include <map>
#include <set>

#include "raweditraylib.h"
#include "spdlog/spdlog.h"
#include "raylib.h"
#include "RawEdit/RawEdit.h"

class ImageManager
{
public:
    void Update();

    const RawEdit::core::Image* CurrentImage() const;
    const Texture2D* CurrentRLTexture() const;
    
    void AddImage(std::string path);
    void SelectNext();
    void SelectPrevious();
    void Select(int32_t idx);

    void Reload();
    void SetResizeFactor(float factor);
    void SetResizeAlgorithm(const char* name);

    float GetResizeFactor() const;
    uint32_t NbImageLoading() const;
    uint32_t NbImageLoaded() const;
    uint32_t RamUsage() const;
    uint32_t VRamUsage() const;
private:
    std::vector<uint32_t> GenerateWindowIndices() const;

    struct Loader
    {
        std::string path;
        std::future<RawEdit::core::Failable<RawEdit::core::ImagePtr>> future;
    };

    struct LoadedImage
    {
        RawEdit::core::ImagePtr image;
        Texture2D texture;
    };

    void AsyncLoad(const std::string& path);
    void ImageLoaded(RawEdit::core::ImagePtr ptr);
    void CheckAndFetch();
    
    float resizeFactor  = .5f;
    uint32_t maxLoader  = 3;
    uint32_t windowSize = 3;

    uint32_t selected = 0;

    std::vector<std::string> allPaths;
    std::vector<Loader> loaders;

    std::vector<RawEdit::core::Error> errors;
    std::map<std::string, LoadedImage> images;
};
