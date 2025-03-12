#include "systems/point_light_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <map>
#include <cassert>
#include <stdexcept>
#include <iostream>

namespace lve {

  struct PointLightPushConstants {
    glm::vec4 position{};
    glm::vec4 color{};
    float radius;
  };

PointLightSystem::PointLightSystem(
    LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
    : lveDevice{device} {
  createPipelineLayout(globalSetLayout);
  createPipeline(renderPass);
}

PointLightSystem::~PointLightSystem() {
  vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
}

void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(PointLightPushConstants);

  std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
  pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
  if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
    VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }
}

  void PointLightSystem::createPipeline(VkRenderPass renderPass) {
    assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
    LvePipeline::enableAlphaBlending(pipelineConfig);
    pipelineConfig.bindingDescriptions.clear();
    pipelineConfig.attributeDescriptions.clear();
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = pipelineLayout;
    lvePipeline = std::make_unique<LvePipeline>(
      lveDevice,
      "shaders/point_light.vert.spv",
      "shaders/point_light.frag.spv",
      pipelineConfig);
}

void PointLightSystem::update(FrameInfo & frameInfo, GlobalUbo & ubo) {
  auto rotateLight = glm::rotate(glm::mat4(1.f), frameInfo.frameTime, glm::vec3(0.f, -1.f, 0.f));
  accumulatedTime += 0.016f;  // 固定增量，约等于60FPS
  float pulseSpeed = 0.1f;  // 控制变化速度
  float pulseMin = 0.2f;    // 最小强度
  float pulseMax = 1.0f;    // 最大强度
  
  int lightIndex = 0;
  for(auto &kv : frameInfo.gameObjects){
    auto &obj = kv.second;
    if(obj.pointLight == nullptr) continue;
    assert(lightIndex < MAX_LIGHTS && "Point light index out of bounds");

    //update light position
    //obj.transform.translation = glm::vec3(rotateLight * glm::vec4(obj.transform.translation, 1.f));


    float baseIntensity =  obj.pointLight->lightIntensity;
    // 呼吸灯效果
    float intensity = glm::mix(pulseMin, pulseMax, (sin(accumulatedTime * pulseSpeed) + 1.0f) * 0.5f) * baseIntensity;
    //copy light to ubo
    ubo.pointLights[lightIndex].position = glm::vec4(obj.transform.translation, 1.f);
    //ubo.pointLights[lightIndex].color = glm::vec4(obj.color, intensity);
    ubo.pointLights[lightIndex].color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
    lightIndex++;
  }
  ubo.numLights = lightIndex;
}

void PointLightSystem::render(FrameInfo &frameInfo) {
  // sort lights
  std::map<float, LveGameObject::id_t> sorted;
  for (auto& kv : frameInfo.gameObjects) {
    auto& obj = kv.second;
    if (obj.pointLight == nullptr) continue;

    // calculate distance
    auto offset = frameInfo.camera.getPosition() - obj.transform.translation;
    float disSquared = glm::dot(offset, offset);
    sorted[disSquared] = obj.GetId();
  }
  lvePipeline->bind(frameInfo.commandBuffer);

  vkCmdBindDescriptorSets(
      frameInfo.commandBuffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      pipelineLayout,
      0,
      1,
      &frameInfo.globalDescriptorSet,
      0,
      nullptr);
  for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
    // use game obj id to find light object
    auto& obj = frameInfo.gameObjects.at(it->second);
    PointLightPushConstants pushConstants{};
    pushConstants.position = glm::vec4(obj.transform.translation, 1.f);
    pushConstants.color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
    pushConstants.radius = obj.transform.scale.x;

    vkCmdPushConstants(
      frameInfo.commandBuffer,
      pipelineLayout,
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      0,
      sizeof(PointLightPushConstants),
      &pushConstants);
    vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
  }
}

}  // namespace lve