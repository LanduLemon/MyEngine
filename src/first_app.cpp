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
    glm::mat4 projcetionView{1.f};
    glm::vec3 lightDirection = LIGHT_DIRECTION;
  };

  FirstApp::FirstApp() {
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
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        lveDevice.properties.limits.minUniformBufferOffsetAlignment);
        uboBuffers[i]->map();
    }

    SimpleRenderSystem simpleRenderSystem{ lveDevice, lveRenderer.getSwapChainRenderPass() };
    LveCamera camera{};
    //camera.setViewDirection(glm::vec3(0.0f),glm::vec3(0.5f,0,1.0f));
    camera.setViewTarget(glm::vec3(-1.0f, -2.0f, 2.0f), glm::vec3(0, 0, 2.5f));

    auto viewObject = LveGameObject::CreateGameObject();
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
      camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 10.f);
      if (auto commandBuffer = lveRenderer.beginFrame()) {
        int frameIndex = lveRenderer.getFrameIndex();
        FrameInfo frameInfo{
          frameIndex,
          frameTime,
          commandBuffer,
          camera
        };
        //update
        GlobalUbo ubo{};
        ubo.projcetionView = camera.getProjectionMatrix() * camera.getViewMatrix();
        uboBuffers[frameIndex]->writeToBuffer(&ubo, sizeof(ubo), frameIndex);
        //uboBuffers[frameIndex]->flush(frameIndex);
        //render
        lveRenderer.beginSwapChainRenderPass(commandBuffer);
        simpleRenderSystem.renderGameObjects(frameInfo, gameObjects);
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
    flatVase.transform.translation = {-.5f, .5f, 2.5f};
    flatVase.transform.scale = {3.f, 1.5f, 3.f};
    gameObjects.push_back(std::move(flatVase));

    lveModel = LveModel::createModelFromFile(lveDevice, "models/smooth_vase.obj");
    auto smoothVase = LveGameObject::CreateGameObject();
    smoothVase.model = lveModel;
    smoothVase.transform.translation = {.5f, .5f, 2.5f};
    smoothVase.transform.scale = {3.f, 1.5f, 3.f};
    gameObjects.push_back(std::move(smoothVase));
  }
}  // namespace LVE