#pragma once
#include <memory>
#include <vulkan/vulkan.h>
#include "lve_texture.hpp"
#include "lve_descriptors.hpp"

namespace lve {
class LveMaterial {
 public:
  LveMaterial::LveMaterial() {}
  bool buildDescriptor(LveDescriptorSetLayout& layout,
                       LveDescriptorPool& pool) {
    if (!baseColor) return false;
    auto imageInfo = baseColor->descriptorInfo();
    return LveDescriptorWriter(layout, pool)
        .writeImage(0, &imageInfo)
        .build(descriptorSet);
  }

  VkDescriptorSet getDescriptorSet() const { return descriptorSet; }
	void SetTexture(std::shared_ptr<LveTexture> tex){baseColor = tex;}
 private:
  VkDescriptorSet descriptorSet{VK_NULL_HANDLE};
	std::shared_ptr<LveTexture> baseColor;
};
}  // namespace lve