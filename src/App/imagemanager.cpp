#include "imagemanager.h"
#include <numeric>

void ImageManager::AddImage(std::string path)
{
    spdlog::info("Adding {} to load queue", path);
    std::erase_if(failed, [&](const auto& f) {
        return f == path;
    });
    allPaths.push_back(std::move(path));
}

// TODO: allocation at each frame... Same for Check and Fetch
std::vector<uint32_t> ImageManager::GenerateWindowIndices() const
{
    std::vector<uint32_t> result;

    if (allPaths.size() == 0) return {};
    if (windowSize == 0) return {};

    if (allPaths.size() < windowSize)
    {
        result.resize(allPaths.size());
        std::iota(result.begin(), result.end(), 0);
        return result;
    }


    uint32_t i = 1;
    result.push_back(selected);
    while(result.size() < windowSize)
    {
        const int32_t idxLeft  = selected +  i;
        const int32_t idxRight = selected + -i;
        
        if (idxLeft >= allPaths.size() && idxRight < 0)
            break;

        if (idxLeft < allPaths.size())
            result.push_back(idxLeft);
        if (idxRight >= 0)
            result.push_back(idxRight);

        ++i;
    }
    return result;
}

void ImageManager::CheckAndFetch()
{
    const auto indices = GenerateWindowIndices();
    
    // Unload textures unwanted textures
    std::erase_if(images, [&](const auto& pair) {
        auto it = std::find_if(indices.begin(), indices.end(), [&](uint32_t idx){
            return pair.first == allPaths[idx];
        });
        return it == indices.end();
    });

    // Something can be loaded
    if (loaders.size() < maxLoader)
    {
        std::vector<std::string> currentPaths;
        for (const auto& im : images)
            currentPaths.push_back(im.first);

        for (const auto& loader : loaders)  
            currentPaths.push_back(loader.path);

        for (const auto& f : failed) 
            currentPaths.push_back(f);

        for (auto i : indices)
        {
            if (loaders.size() >= maxLoader)
                break;
            
            auto it = std::find(currentPaths.begin(), 
                                currentPaths.end(), 
                                allPaths[i]);
            if (it == currentPaths.end())
            {
                AsyncLoad(allPaths[i]);
                currentPaths.push_back(allPaths[i]);
            }
        }
    }
}

void ImageManager::Update()
{
    for (auto it = loaders.begin(); it != loaders.end();)
    {
        auto state = it->future.wait_for(std::chrono::milliseconds(0));
        if (state == std::future_status::ready)
        {
            auto result = it->future.get();
            if (result)
            {
                ImageLoaded(*result);
            }
            else 
            {
                errors.push_back(result.error());
                failed.push_back(it->path);
            }

            it = loaders.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // Unload unwanted textures and fetch new ones
    CheckAndFetch();
}

void ImageManager::SelectNext()
{
    Select((int32_t)selected + 1);
}

void ImageManager::SelectPrevious()
{
    Select((int32_t)selected - 1);
}

std::vector<RawEdit::Error> ImageManager::pullErrors()
{
    auto err = errors;
    errors.clear();
    return errors;
}

void ImageManager::Select(int32_t idx)
{
    selected = idx;
    if (selected < 0) selected = 0;
    if (selected >= allPaths.size()) selected = allPaths.size() - 1;
}

const RawEdit::ImagePtr ImageManager::CurrentImage() const
{
    if (allPaths.size() == 0) return nullptr;
    
    auto it = images.find(allPaths[selected]);
    if (it == images.end())
        return nullptr;
    return it->second.image;
}

const Texture2D* ImageManager::CurrentRLTexture() const
{
    if (allPaths.size() == 0) return nullptr;
    
    auto it = images.find(allPaths[selected]);
    if (it == images.end())
        return nullptr;
    return &it->second.texture;
}

RawEdit::Mask* ImageManager::CurrentMask()
{
    if (allPaths.size() == 0) return nullptr;

    auto it = images.find(allPaths[selected]);
    if (it == images.end())
        return nullptr;
    return &it->second.mask;
}

void ImageManager::AsyncLoad(const std::string& path)
{
    if (loaders.size() < maxLoader)
    {
        spdlog::info("Loading {}", path);
        loaders.push_back(Loader{
            .path = path,
            .future = std::async(std::launch::async,
            [=]()
            {
                return RawEdit::Load(path.c_str());
            })
        });
    }
}

void ImageManager::ImageLoaded(RawEdit::ImagePtr im)
{
    spdlog::info("{} loaded", im->metadata.path);

    // Resizing
    RawEdit::ImagePtr newIm(im->EmptyCopy(true));
    rescale.BindInputImage(im);
    rescale.BindOutputImage(newIm);

    RawEdit::Error err = rescale.Run();
    if (!err.empty())
    {
        spdlog::error("{}", err);
        errors.push_back(err);
        failed.push_back(im->metadata.path);
        return;
    }

    auto& loc = images[newIm->metadata.path];
    loc.image = newIm;
    loc.mask.FillData(newIm->width, newIm->height, 1, true);
    loc.texture = ConvertToRaylibTexture(newIm.get());
}

void ImageManager::Reload()
{
    images.clear();
}

void ImageManager::Clear()
{
    loaders.clear();
    allPaths.clear();
    images.clear();
}

float ImageManager::GetResizeFactor() const 
{
    return rescale["factor"].AsFloat();
}

float& ImageManager::GetResizeFactor()
{
    return rescale["factor"].AsFloat();
}

uint32_t ImageManager::NbImageLoading() const
{
    return loaders.size();
}

uint32_t ImageManager::NbImageLoaded() const
{
    return images.size();
}

