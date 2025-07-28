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
  SkyboxRenderSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
  ~SkyboxRenderSystem();

  SkyboxRenderSystem(const SkyboxRenderSystem&) = delete;
  SkyboxRenderSystem& operator=(const SkyboxRenderSystem&) = delete;

  void renderSkybox(FrameInfo& frameInfo);

 private:
  void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
  void createPipeline(VkRenderPass renderPass);

  LveDevice& lveDevice;

  std::unique_ptr<LvePipeline> lvePipeline;
  VkPipelineLayout pipelineLayout;
};

}  // namespace lve