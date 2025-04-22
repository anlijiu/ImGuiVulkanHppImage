#pragma once

#ifndef _IMAGE_CACHE_H_
#define _IMAGE_CACHE_H_

#include <filesystem>
#include <unordered_map>
#include <vector>

#include "IdTypes.h"
#include "GPUImage.h"

#include "BindlessSetManager.h"

class GfxDevice;

class ImageCache {
    friend class ResourcesInspector;

public:
    ImageCache(GfxDevice& gfxDevice);

    ImageId loadImageFromFile(
        const std::filesystem::path& path,
        VkFormat format,
        VkImageUsageFlags usage,
        bool mipMap);

    ImageId addImage(GPUImage image);
    ImageId addImage(ImageId id, GPUImage image);
    const GPUImage& getImage(ImageId id) const;

    ImageId getFreeImageId() const;

    void destroyImages();

    BindlessSetManager bindlessSetManager;

    void setErrorImageId(ImageId id) { errorImageId = id; }

private:
    std::vector<GPUImage> images;
    GfxDevice& gfxDevice;

    struct LoadedImageInfo {
        std::filesystem::path path;
        VkFormat format;
        VkImageUsageFlags usage;
        bool mipMap;
    };
    std::unordered_map<ImageId, LoadedImageInfo> loadedImagesInfo;
    ImageId errorImageId{NULL_IMAGE_ID};
};

#endif /* ifndef _IMAGE_CACHE_H_ */
