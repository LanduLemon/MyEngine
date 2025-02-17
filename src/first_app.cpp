#include "first_app.hpp"

#include "keyboard_movement.hpp"
#include "simple_render_system.hpp"
#include "lve_camera.hpp"
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

namespace lve {
  FirstApp::FirstApp() {
    loadGameObjects();
  }

  FirstApp::~FirstApp() {}

  void FirstApp::run() {
    SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};
    LveCamera camera{};
    //camera.setViewDirection(glm::vec3(0.0f),glm::vec3(0.5f,0,1.0f));
    camera.setViewTarget(glm::vec3(-1.0f, -2.0f, 2.0f), glm::vec3(0,0,2.5f));

    auto viewObject = LveGameObject::CreateGameObject();
    KeyboardMovementController cameraController{};

    auto currentTime = std::chrono::high_resolution_clock::now();

    while (!lveWindow.shouldClose()) {
      glfwPollEvents();

      if(glfwGetKey(lveWindow.getGLFWwindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS){
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
      if(auto commandBuffer = lveRenderer.beginFrame()){
        lveRenderer.beginSwapChainRenderPass(commandBuffer);
        simpleRenderSystem.renderGameObjects(commandBuffer,gameObjects,camera);
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