#pragma once

#include "lve_device.hpp"
#include "lve_buffer.hpp"
// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>

namespace lve {
class LveModel {
public:
  struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec4 tangent{};  // w分量用于确定副切线的方向（通常是1或-1) 暂时没用到

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

    bool operator==(const Vertex &other) const {
      return position == other.position && color == other.color && normal == other.normal && uv == other.uv && tangent == other.tangent;
    }
  };
  //TODO 考虑之后添加PBR材质
  // struct PBRMaterial{
  //   glm::vec3 albedo{1.0f};
  //   float metallic{0.5f};
  //   float roughness{0.5f};
  //   float ao{1.0f};
  // };

  struct Builder {
      std::vector<Vertex> vertices{};
      std::vector<uint32_t> indices{};

      void loadModel(const std::string& filepath);
  private:
      void loadObjModel(const std::string& filepath);
      void loadGltfModel(const std::string& filepath);
      void computeTangents();
  };

  LveModel(LveDevice &device, const Builder &builder);
  ~LveModel();

  LveModel(const LveModel &) = delete;
  LveModel &operator=(const LveModel &) = delete;

  static std::unique_ptr<LveModel> createModelFromFile(LveDevice &device, const std::string &filePath);

  void bind(VkCommandBuffer commandBuffer);
  void draw(VkCommandBuffer commandBuffer);

private:
  void createVertexBuffers(const std::vector<Vertex> &vertices);
  void createIndexBuffers(const std::vector<uint32_t> &indices);

  LveDevice &lveDevice;

  std::unique_ptr<LveBuffer> vertexBuffer;
  uint32_t vertexCount;

  bool hasIndexBuffer = false;
  std::unique_ptr<LveBuffer> indexBuffer;
  uint32_t indexCount;
};
}  // namespace lve