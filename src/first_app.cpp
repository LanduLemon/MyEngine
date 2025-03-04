#include "first_app.hpp"

#include "keyboard_movement.hpp"
#include "simple_render_system.hpp"
#include "lve_camera.hpp"
#include "lve_buffer.hpp"
// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <chrono>
#include <cassert>
#include <stdexcept>

#define LIGHT_DIRECTION glm::vec3(1.0, -3.0, -1.0);

namespace lve {

  struct GlobalUbo {
    glm::mat4 projection{1.f};
    glm::mat4 view{1.f};
    glm::vec4 ambientLightColor{1.f, 1.f, 1.f, 0.1f}; //w is intensity 
    glm::vec3 lightPosition{-1.f};
    alignas(16) glm::vec4 lightColor{1.f}; //w is light intensity
  };
  // struct GlobalUbo {
  //   glm::mat4 projectionView{1.f};
  //   glm::vec3 lightDirection = LIGHT_DIRECTION;
  // };

  FirstApp::FirstApp() {
    globalPool = LveDescriptorPool::Builder(lveDevice)
      .setMaxSets(2)
      .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
      .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
      .build();
    loadGameObjects();
  }

  FirstApp::~FirstApp() {}

  void FirstApp::run() {
    std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < uboBuffers.size(); i++) {
      uboBuffers[i] = std::make_unique<LveBuffer>(
        lveDevice,
        sizeof(GlobalUbo),
        1,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        uboBuffers[i]->map();
    }
    auto globalSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
      .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
      .build();

    std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
    for(int i = 0; i < globalDescriptorSets.size(); i++){
      auto bufferInfo = uboBuffers[i]->descriptorInfo();
      LveDescriptorWriter(*globalSetLayout, *globalPool)
      .writeBuffer(0, &bufferInfo)
      .build(globalDescriptorSets[i]);
    }


    SimpleRenderSystem simpleRenderSystem{ lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
    LveCamera camera{};

    auto viewObject = LveGameObject::CreateGameObject();
    viewObject.transform.translation.z = -2.0f;
    KeyboardMovementController cameraController{};

    auto currentTime = std::chrono::high_resolution_clock::now();

    while (!lveWindow.shouldClose()) {
      glfwPollEvents();

      if (glfwGetKey(lveWindow.getGLFWwindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(lveWindow.getGLFWwindow(), true);
      }

      auto newTime = std::chrono::high_resolution_clock::now();
      float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
      currentTime = newTime;

      //frameTime = std::min(frameTime, MAX_FRAME_TIME);

      cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewObject);
      camera.setViewYXZ(viewObject.transform.translation, viewObject.transform.rotation);

      float aspect = lveRenderer.getAspectRatio();
      //camera.setOrthographicProjection(-aspect,aspect,-1,1,-1,1);
      camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 1000.f);
      if (auto commandBuffer = lveRenderer.beginFrame()) {
        int frameIndex = lveRenderer.getFrameIndex();
        FrameInfo frameInfo{
          frameIndex,
          frameTime,
          commandBuffer,
          camera,
          globalDescriptorSets[frameIndex],
          gameObjects
        };
        //update
        GlobalUbo ubo{};
        ubo.projection = camera.getProjectionMatrix();
        ubo.view = camera.getViewMatrix();
        uboBuffers[frameIndex]->writeToBuffer(&ubo);
        uboBuffers[frameIndex]->flush();
        //render
        lveRenderer.beginSwapChainRenderPass(commandBuffer);
        simpleRenderSystem.renderGameObjects(frameInfo);
        lveRenderer.endSwapChainRenderPass(commandBuffer);
        lveRenderer.endFrame();
      }
    }

    vkDeviceWaitIdle(lveDevice.device());
  }

  void FirstApp::loadGameObjects() {
    std::shared_ptr<LveModel> lveModel = lveModel->createModelFromFile(lveDevice, "models/flat_vase.obj");
    auto flatVase = LveGameObject::CreateGameObject();
    flatVase.model = lveModel;
    flatVase.transform.translation = {-.5f, .5f, 0};
    flatVase.transform.scale = {3.f, 1.5f, 3.f};
    gameObjects.emplace(flatVase.GetId(), std::move(flatVase));

    lveModel = LveModel::createModelFromFile(lveDevice, "models/smooth_vase.obj");
    auto smoothVase = LveGameObject::CreateGameObject();
    smoothVase.model = lveModel;
    smoothVase.transform.translation = {.5f, .5f, 0};
    smoothVase.transform.scale = {3.f, 1.5f, 3.f};
    gameObjects.emplace(smoothVase.GetId(), std::move(smoothVase));

    lveModel = LveModel::createModelFromFile(lveDevice, "models/quad.obj");
    auto quad_floor = LveGameObject::CreateGameObject();
    quad_floor.model = lveModel;
    quad_floor.transform.translation = {.5f, .5f, 0};
    quad_floor.transform.scale = {3.f, 1.5f, 3.f};
    gameObjects.emplace(quad_floor.GetId(), std::move(quad_floor));
  }
}  // namespace LVE