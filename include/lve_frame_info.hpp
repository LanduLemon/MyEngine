#pragma once

#include "lve_camera.hpp"

//lib
#include <vulkan/vulkan.h>
#include <lve_game_object.hpp>

namespace lve{
  struct FrameInfo
  {
    int frameIndex;
    float frameTime;
    VkCommandBuffer commandBuffer;
    LveCamera &camera;
    VkDescriptorSet globalDescriptorSet;
    LveGameObject::Map &gameObjects;
  };
  
} // namespace lve