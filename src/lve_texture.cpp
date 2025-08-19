#include "lve_texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb/stb_image.h"

#include <array>
#include <stdexcept>

namespace lve {
LveTexture::LveTexture(LveDevice& device, const std::string& filepath) : device_{device} {
		stbi_set_flip_vertically_on_load(true);
    createTextureImage(filepath);
    createTextureImageView();
    createTextureSampler();
}

LveTexture::LveTexture(LveDevice& device, const std::array<std::string, 6>& faces) : device_{device} {
		stbi_set_flip_vertically_on_load(true);
    isCubemap_ = true;
    createCubemapImage(faces);
    createTextureImageView();
    createTextureSampler();
}

LveTexture::~LveTexture() {
    vkDestroySampler(device_.device(), textureSampler, nullptr);
    vkDestroyImageView(device_.device(), textureImageView, nullptr);
    vkDestroyImage(device_.device(), textureImage, nullptr);
    vkFreeMemory(device_.device(), textureImageMemory, nullptr);
}

void LveTexture::createTextureImage(const std::string& filepath){
	int texWidth, texHeight, texChannels;
	stbi_uc *pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4; // Assuming 4 channels (RGBA)
	if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
	}
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	device_.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
												VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
														VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
												stagingBuffer, stagingBufferMemory);
	void *data;
	vkMapMemory(device_.device(), stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(device_.device(), stagingBufferMemory);
	stbi_image_free(pixels);

	device_.createImage(
			texWidth, texHeight, 1, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
	device_.transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
												VK_IMAGE_LAYOUT_UNDEFINED,
												VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	device_.copyBufferToImage(stagingBuffer, textureImage,
										static_cast<uint32_t>(texWidth),
										static_cast<uint32_t>(texHeight), 1);
	device_.transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
												VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
												VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	vkDestroyBuffer(device_.device(), stagingBuffer, nullptr);
	vkFreeMemory(device_.device(), stagingBufferMemory, nullptr);
}

void LveTexture::createCubemapImage(const std::array<std::string, 6>& faces) {
	int texWidth, texHeight, texChannels;
	std::vector<stbi_uc*> pixels(6);
	for (size_t i = 0; i < faces.size(); ++i) {
			pixels[i] = stbi_load(faces[i].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			if (!pixels[i]) {
					for (size_t j = 0; j < i; ++j) stbi_image_free(pixels[j]);
					throw std::runtime_error("failed to load cubemap face: " + faces[i]);
			}
	}
	VkDeviceSize faceSize = texWidth * texHeight * 4;
	VkDeviceSize totalSize = faceSize * 6;
	
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	device_.createBuffer(totalSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
												VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
														VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
												stagingBuffer, stagingBufferMemory);
	void *data;
	vkMapMemory(device_.device(), stagingBufferMemory, 0, totalSize, 0, &data);
	for (size_t i = 0; i < faces.size(); ++i) {
		memcpy(static_cast<uint8_t*>(data) + i * faceSize, pixels[i], static_cast<size_t>(faceSize));
		stbi_image_free(pixels[i]);
	}
	vkUnmapMemory(device_.device(), stagingBufferMemory);

	device_.createImage(
			texWidth, texHeight, 6, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
	device_.transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
												VK_IMAGE_LAYOUT_UNDEFINED,
												VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,6);
	device_.copyBufferToImage(stagingBuffer, textureImage,
														static_cast<uint32_t>(texWidth),
														static_cast<uint32_t>(texHeight),
														6);
	device_.transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
																VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
																VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
																6);
	vkDestroyBuffer(device_.device(), stagingBuffer, nullptr);
	vkFreeMemory(device_.device(), stagingBufferMemory, nullptr);
}

void LveTexture::createTextureImageView() {
	if (isCubemap_) {
		textureImageView = device_.createImageView(textureImage, VK_IMAGE_VIEW_TYPE_CUBE, VK_FORMAT_R8G8B8A8_SRGB);
	}else{
		textureImageView = device_.createImageView(textureImage, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_SRGB);
	}
}

void LveTexture::createTextureSampler() {
	VkSamplerCreateInfo samplerInfo ={};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = isCubemap_ ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = isCubemap_ ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = isCubemap_ ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	if (vkCreateSampler(device_.device(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

}