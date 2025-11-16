#include "imagemanager.h"

void ImageManager::AddImage(std::string path)
{
    allPaths.push_back(std::move(path));
}

std::vector<uint32_t> ImageManager::GenerateWindowIndices() const
{
    if (allPaths.size() == 0) return {};
    if (windowSize == 0) return {};

    std::vector<uint32_t> result;
    result.reserve(windowSize);
    result.push_back(selected);

    uint32_t i = 1;
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
        std::set<std::string> currentPaths;
        for (const auto& im : images)       currentPaths.insert(im.first);
        for (const auto& loader : loaders)  currentPaths.insert(loader.path);

        for (auto i : indices)
        {
            if (loaders.size() >= maxLoader)
                break;
            
            if (currentPaths.find(allPaths[i]) == currentPaths.end())
            {
                AsyncLoad(allPaths[i]);
                currentPaths.insert(allPaths[i]);
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
                auto im = *result;
                if (im->data != nullptr)
                    ImageLoaded(im);
                else
                    errors.push_back(RawEdit::core::UkError("No data loaded"));
            }
            else 
            {
                errors.push_back(result.error());
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

std::vector<RawEdit::core::Error> ImageManager::pullErrors()
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

const RawEdit::core::Image* ImageManager::CurrentImage() const
{
    if (allPaths.size() == 0) return nullptr;
    
    auto it = images.find(allPaths[selected]);
    if (it == images.end())
        return nullptr;
    return it->second.image.get();
}

const Texture2D* ImageManager::CurrentRLTexture() const
{
    if (allPaths.size() == 0) return nullptr;
    
    auto it = images.find(allPaths[selected]);
    if (it == images.end())
        return nullptr;
    return &it->second.texture;
}

RawEdit::algorithm::Mask* ImageManager::CurrentMask()
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
                return RawEdit::core::OpenImage(path.c_str());
            })
        });
    }
}

void ImageManager::ImageLoaded(RawEdit::core::ImagePtr im)
{
    spdlog::info("{} / {} loaded", im->metadata.path, im->gpuImage.id);

    // Resizing
    if (resizeFactor != 1.f)
    {
        const uint32_t targetW = im->width  * resizeFactor;
        const uint32_t targetH = im->height * resizeFactor;
        
        RawEdit::algorithm::Rescale(
            im, targetW, targetH, 
            RawEdit::algorithm::RescaleMethod::NEAREST,
            RawEdit::core::Device::GPU_OPENGL
        );
    }
    else
    {
        auto err = im->UploadGPU(true);
        if (err) errors.push_back(err);
    }

    images[im->metadata.path] = { 
        .image   = im, 
        .mask    = RawEdit::algorithm::Mask(im->gpuImage.width, im->gpuImage.height),
        .texture = ConvertToRaylibTexture(im.get()) 
    };
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

void ImageManager::SetResizeFactor(float factor)
{
    resizeFactor = factor;
}

float ImageManager::GetResizeFactor() const 
{
    return resizeFactor;
}

uint32_t ImageManager::NbImageLoading() const
{
    return loaders.size();
}

uint32_t ImageManager::NbImageLoaded() const
{
    return images.size();
}

uint32_t ImageManager::RamUsage() const
{
    uint32_t count = 0;
    for (const auto& im : images)
        count += (im.second.image->DataSize() + im.second.mask.GetMask()->DataSize());
    return count / (1024 * 1024);
}

uint32_t ImageManager::VRamUsage() const
{
    uint32_t count = 0;
    for (const auto& im : images)
        count += (im.second.image->GPUDataSize() + im.second.mask.GetMask()->GPUDataSize());
    return count / (1024 * 1024);

}
