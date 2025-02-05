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

  std::unique_ptr<LveModel> createCubeModel(LveDevice& device, glm::vec3 offset) {
    LveModel::Builder modelBuilder{};
    modelBuilder.vertices = {
        // left face (white)
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
  
        // right face (yellow)
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
  
        // top face (orange, remember y axis points down)
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
  
        // bottom face (red)
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
  
        // nose face (blue)
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
  
        // tail face (green)
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
    };
    for (auto& v : modelBuilder.vertices) {
      v.position += offset;
    }
  
    modelBuilder.indices = {0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
                            12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21};
  
    return std::make_unique<LveModel>(device, modelBuilder);
  }

  void FirstApp::loadGameObjects() {
    std::shared_ptr<LveModel> lveModel = createCubeModel(lveDevice, {0.f, 0.f, 0.f});

    auto cube = LveGameObject::CreateGameObject();
    cube.model = lveModel;
    cube.transform.translation = {0.f, 0.f, 2.5f};
    cube.transform.scale = {.5f, .5f, .5f};
    gameObjects.push_back(std::move(cube));
  }
}  // namespace lve