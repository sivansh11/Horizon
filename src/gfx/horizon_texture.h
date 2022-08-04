#ifndef GFX_HORIZON_TEXTURE_H
#define GFX_HORIZON_TEXTURE_H

#include "gfx/horizon_device.h"
#include "gfx/horizon_buffer.h"

#include <string>

namespace horizon {

namespace gfx {

class Texture2D {
public:
    Texture2D(Device& deviceRef, std::string file);
    ~Texture2D();

    VkImage getImage() { return mImage; }
    VkDeviceMemory getImageMemory() { return mImageMemory; }
    VkImageView getImageView() { return mImageView; }
    VkSampler getSampler() { return mSampler; }
    VkFormat getImageFormat() { return mImageFormat; }
    VkImageLayout getImageLayout() { return mImageLayout; }

private:
    void transitionImageLayout(VkImageLayout oldImageLayout, VkImageLayout newImageLayout);
    void copyBufferToImage(Buffer& buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);
    void generateMipMaps();

private:
    Device& mDevice;
    std::string filePath;
    int mWidth, mHeight, mMiplevels;
    VkImage mImage;
    VkDeviceMemory mImageMemory;
    VkImageView mImageView;
    VkSampler mSampler;
    VkFormat mImageFormat;
    VkImageLayout mImageLayout;
};

} // namespace gfx

} // namespace horizon

#endif