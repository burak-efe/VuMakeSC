#pragma once

#include "Common.h"
#include "VuBuffer.h"
#include "VuImage.h"

namespace std::filesystem {
    class path;
}

namespace Vu {

    struct VuTextureCreateInfo {
        std::filesystem::path path;
        VkFormat              format = VK_FORMAT_R8G8B8A8_SRGB;
    };

    struct VuTexture {
    public:
        VkImage        image;
        VkDeviceMemory imageMemory;
        VkImageView    imageView;

        void init(const VuTextureCreateInfo& info) {
            std::cout << "VuTexture::init()" << std::endl;
            //Image
            int texWidth;
            int texHeight;
            int texChannels;

            stbi_uc* pixels;
            loadImageFile(info, texWidth, texHeight, texChannels, pixels);
            const auto imageSize = static_cast<VkDeviceSize>(texWidth * texHeight * 4);

            if (pixels == nullptr) {
                throw std::runtime_error("failed to load texture image!");
            }

            // VuBuffer staging{};
            // VkDeviceSize size = texWidth * texHeight;
            // staging.init({
            //     .lenght = size,
            //     .strideInBytes = 4,
            //     .usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
            // });
            VuBuffer::globalStagingBuffer->setData(pixels, imageSize);
            stbi_image_free(pixels);

            VuImage::createImage(texWidth, texHeight, info.format, VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 image,
                                 imageMemory);

            VuImage::transitionImageLayout(image,
                                           VK_IMAGE_LAYOUT_UNDEFINED,
                                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            VuImage::copyBufferToImage(VuBuffer::globalStagingBuffer->buffer, image,
                                       static_cast<uint32>(texWidth), static_cast<uint32>(texHeight));

            VuImage::transitionImageLayout(image,
                                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            VuImage::createImageView(info.format, image,VK_IMAGE_ASPECT_COLOR_BIT, imageView);

            //staging.uninit();
        }

        void uninit() {
            std::cout << "VuTexture::uninit()" << std::endl;
            //vkDestroyImage(ctx::vuDevice->device, image, nullptr);
            //vkFreeMemory(ctx::vuDevice->device, imageMemory, nullptr);
            //vkDestroyImageView(ctx::vuDevice->device, imageView, nullptr);
        } //

    private:
        static void loadImageFile(const VuTextureCreateInfo& info, int& texWidth, int& texHeight, int& texChannels, stbi_uc*& pixels) {
            pixels = stbi_load(info.path.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        }
    };

}
