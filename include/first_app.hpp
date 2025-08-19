#pragma once

#include "lve_descriptors.hpp"
#include "lve_device.hpp"
#include "lve_game_object.hpp"
#include "lve_renderer.hpp"
#include "lve_window.hpp"
#include "lve_texture.hpp"

// std
#include <memory>
#include <vector>

namespace lve {
class FirstApp {
 public:
  static constexpr int WIDTH = 800;
  static constexpr int HEIGHT = 600;

  FirstApp();
  ~FirstApp();

  FirstApp(const FirstApp &) = delete;
  FirstApp &operator=(const FirstApp &) = delete;

  void run();

 private:
  void loadGameObjects();

  LveWindow lveWindow{WIDTH, HEIGHT, "Vulkan Tutorial"};
  LveDevice lveDevice{lveWindow};
  LveRenderer lveRenderer{lveWindow, lveDevice};

  std::unique_ptr<LveDescriptorPool> globalPool{};
  std::unique_ptr<LveDescriptorPool> materialPool{};
  LveGameObject::Map gameObjects;

	std::shared_ptr<LveTexture> defaultTexture_;
	std::shared_ptr<LveTexture> skyboxTexture_;
};
}  // namespace lve
