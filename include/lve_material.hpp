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
    if (!baseTex_) return false;
    auto imageInfo = baseTex_->descriptorInfo();
    return LveDescriptorWriter(layout, pool)
        .writeImage(0, &imageInfo)
        .build(descriptorSet);
  }

	void SetColor(const glm::vec4& color) { color_ = color; }
  const glm::vec4& GetColor() const { return color_; }

  VkDescriptorSet getDescriptorSet() const { return descriptorSet; }
	void SetTexture(std::shared_ptr<LveTexture> tex){baseTex_ = tex;}
 private:
  VkDescriptorSet descriptorSet{VK_NULL_HANDLE};
	std::shared_ptr<LveTexture> baseTex_;
	glm::vec4 color_{1.f,1.f,1.f,1.f};
};
}  // namespace lve