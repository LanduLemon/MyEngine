#pragma once
#include "lve_device.hpp"
#include <string>

namespace lve {
class LveTexture {
public:
    LveTexture(LveDevice& device, const std::string& filepath);
		LveTexture(LveDevice& device, const std::array<std::string, 6>& faces); // cubemap
    ~LveTexture();

    VkImageView getImageView() const { return textureImageView; }
    VkSampler getSampler() const { return textureSampler; }

		VkDescriptorImageInfo descriptorInfo() const {
			return VkDescriptorImageInfo{
				textureSampler,
				textureImageView,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};
		}

private:
    void createTextureImage(const std::string& filepath);
		void createCubemapImage(const std::array<std::string, 6>& faces);
    void createTextureImageView();
    void createTextureSampler();

    LveDevice& device_;
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

		bool isCubemap_{false};   // 新增
};
}