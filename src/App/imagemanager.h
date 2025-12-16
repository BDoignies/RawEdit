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

    // Those methods are marked const because but it may give the
    // impression the image will never be modified, because opengl
    // requires uses an id.
    const RawEdit::ImagePtr CurrentImage() const;
    const Texture2D* CurrentRLTexture() const;
    RawEdit::Mask* CurrentMask();
    
    void AddImage(std::string path);
    void SelectNext();
    void SelectPrevious();
    void Select(int32_t idx);

    std::vector<RawEdit::Error> pullErrors();

    void Reload();
    void Clear();

    float  GetResizeFactor() const;
    float& GetResizeFactor();
    uint32_t NbImageLoading() const;
    uint32_t NbImageLoaded() const;
private:
    std::vector<uint32_t> GenerateWindowIndices() const;

    struct Loader
    {
        std::string path;
        std::future<RawEdit::Failable<RawEdit::ImagePtr>> future;
    };

    struct LoadedImage
    {
        RawEdit::ImagePtr image;
        RawEdit::Mask mask;
        Texture2D texture;
    };

    void AsyncLoad(const std::string& path);
    void ImageLoaded(RawEdit::ImagePtr ptr);
    void CheckAndFetch();
    
    RawEdit::Rescale rescale;
    uint32_t maxLoader  = 3;
    uint32_t windowSize = 3;

    uint32_t selected = 0;

    std::vector<std::string> allPaths;
    std::vector<std::string> failed;
    std::vector<Loader> loaders;

    std::vector<RawEdit::Error> errors;
    std::map<std::string, LoadedImage> images;
};
