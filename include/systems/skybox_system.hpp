#pragma once

#include "lve_camera.hpp"
#include "lve_device.hpp"
#include "lve_game_object.hpp"
#include "lve_pipeline.hpp"
#include "lve_frame_info.hpp"

// std
#include <memory>
#include <vector>

namespace lve {

class SkyboxRenderSystem {
 public:
  SkyboxRenderSystem(LveDevice& device, VkRenderPass renderPass,
                     VkDescriptorSetLayout globalSetLayout,
                     VkDescriptorSetLayout cubemapSetLayout,
										 VkDescriptorSet skyboxSet);
  ~SkyboxRenderSystem();

  SkyboxRenderSystem(const SkyboxRenderSystem&) = delete;
  SkyboxRenderSystem& operator=(const SkyboxRenderSystem&) = delete;

  void renderSkybox(FrameInfo& frameInfo);

 private:
  void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout cubemapSetLayout);
  void createPipeline(VkRenderPass renderPass);

  LveDevice& lveDevice;

  std::unique_ptr<LvePipeline> lvePipeline;
  VkPipelineLayout pipelineLayout;
	VkDescriptorSet skyboxSet_{VK_NULL_HANDLE}; // 新增
};

}  // namespace lve