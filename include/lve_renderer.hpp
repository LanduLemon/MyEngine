#pragma once

#include "lve_device.hpp"
#include "lve_swap_chain.hpp"
#include "lve_window.hpp"

// std
#include <memory>
#include <vector>

namespace lve {
class LveRenderer {
 public:
  LveRenderer();
  ~LveRenderer();

  LveRenderer(const LveRenderer &) = delete;
  LveRenderer &operator=(const LveRenderer &) = delete;

  void run();

 private:
  void createCommandBuffers();
  void freeCommandBuffers();
  void drawFrame();
  void recreateSwapChain();

  LveWindow& lveWindow;
  LveDevice& lveDevice;
  std::unique_ptr<LveSwapChain> lveSwapChain;
  std::vector<VkCommandBuffer> commandBuffers;
};
}  // namespace lve
