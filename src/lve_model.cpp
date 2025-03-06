#include "lve_model.hpp"

#include "lve_utils.hpp"

//libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#define CGLTF_IMPLEMENTATION
#include "third_party/cgltf/cgltf.h"

// std
#include <cassert>
#include <cstring>
#include <unordered_map>
#include <iostream>

namespace std {
template<>
struct hash<lve::LveModel::Vertex> {
  size_t operator()(lve::LveModel::Vertex const &vertex) const {
    size_t seed = 0;
    lve::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
    return seed;
  }
};
} // namespace std


namespace lve {

LveModel::LveModel(LveDevice &device, const LveModel::Builder &builder) : lveDevice{ device } {
  createVertexBuffers(builder.vertices);
  createIndexBuffers(builder.indices);
}

LveModel::~LveModel() {}

void LveModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
  vertexCount = static_cast<uint32_t>(vertices.size());
  assert(vertexCount >= 3 && "Vertex count must be at least 3");
  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
  uint32_t vertexSize = sizeof(vertices[0]);

  LveBuffer stagingBuffer{
    lveDevice,
    vertexSize,
    vertexCount,
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void *)vertices.data());

  vertexBuffer = std::make_unique<LveBuffer>(
    lveDevice,
    vertexSize,
    vertexCount,
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  lveDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
}

void LveModel::createIndexBuffers(const std::vector<uint32_t> &indices) {
  indexCount = static_cast<uint32_t>(indices.size());
  hasIndexBuffer = indexCount > 0;
  if (!hasIndexBuffer) {
    return;
  }

  VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
  uint32_t indicesSize = sizeof(indices[0]);

  LveBuffer stagingBuffer{
    lveDevice,
    indicesSize,
    indexCount,
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void *)indices.data());

  indexBuffer = std::make_unique<LveBuffer>(
    lveDevice,
    indicesSize,
    indexCount,
    VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  lveDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
}

void LveModel::draw(VkCommandBuffer commandBuffer) {
  if (hasIndexBuffer) {
    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
  } else {
    vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
  }
}

std::unique_ptr<LveModel> LveModel::createModelFromFile(LveDevice &device, const std::string &filePath)
{
  Builder builder{};
  builder.loadModel(filePath);
  return std::make_unique<LveModel>(device, builder);
}

void LveModel::bind(VkCommandBuffer commandBuffer) {
  VkBuffer buffers[] = { vertexBuffer->getBuffer() };
  VkDeviceSize offsets[] = { 0 };
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

  if (hasIndexBuffer) {
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
  }
}

std::vector<VkVertexInputBindingDescription> LveModel::Vertex::getBindingDescriptions() {
  std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
  bindingDescriptions[0].binding = 0;
  bindingDescriptions[0].stride = sizeof(Vertex);
  bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> LveModel::Vertex::getAttributeDescriptions() {
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

  attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
  attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
  attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
  attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});
  attributeDescriptions.push_back({4,0,VK_FORMAT_R32G32B32A32_SFLOAT,offsetof(Vertex,tangent)});
  return attributeDescriptions;
}

void LveModel::Builder::loadModel(const std::string& filepath) {
    try {
        std::cout << "Loading model: " << filepath << std::endl;
        
        // 检查文件扩展名
        std::string extension = filepath.substr(filepath.find_last_of(".") + 1);
        if (extension == "gltf" || extension == "glb") {
            std::cout << "Detected GLTF format" << std::endl;
            loadGltfModel(filepath);
        } else {
            std::cout << "Detected OBJ format" << std::endl;
            loadObjModel(filepath);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading model: " << e.what() << std::endl;
        throw;
    }
}

void LveModel::Builder::loadObjModel(const std::string &filePath) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str())) {
    throw std::runtime_error(warn + err);
  }

  vertices.clear();
  indices.clear();

  std::unordered_map<Vertex, uint32_t> uniqueVertices{};
  for (const auto &shape : shapes) {
    for (const auto &index : shape.mesh.indices) {
      Vertex vertex{};
      if (index.vertex_index >= 0) {
        vertex.position = {
          attrib.vertices[3 * index.vertex_index + 0],
          attrib.vertices[3 * index.vertex_index + 1],
          attrib.vertices[3 * index.vertex_index + 2]
        };

        vertex.color = {
            attrib.colors[3 * index.vertex_index + 0],
            attrib.colors[3 * index.vertex_index + 1],
            attrib.colors[3 * index.vertex_index + 2],
        };
      }

      if (index.normal_index >= 0) {
        vertex.normal = {
          attrib.normals[3 * index.normal_index + 0],
          attrib.normals[3 * index.normal_index + 1],
          attrib.normals[3 * index.normal_index + 2]
        };
      }

      if (index.texcoord_index >= 0) {
        vertex.uv = {
          attrib.texcoords[2 * index.texcoord_index + 0],
          attrib.texcoords[2 * index.texcoord_index + 1]
        };
      }
      if(uniqueVertices.count(vertex) == 0){
        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
        vertices.push_back(vertex);
      }
      indices.push_back(uniqueVertices[vertex]);
    }
  }
}

void LveModel::Builder::loadGltfModel(const std::string& filepath) {
    std::cout << "Starting GLTF load..." << std::endl;
    
    cgltf_options options{};
    cgltf_data* data = nullptr;
    
    // 解析文件
    cgltf_result result = cgltf_parse_file(&options, filepath.c_str(), &data);
    if (result != cgltf_result_success) {
        throw std::runtime_error("Failed to parse gltf file: " + filepath);
    }

    // 检查是否需要加载外部buffer
    bool needsExternalBuffers = false;
    for (size_t i = 0; i < data->buffers_count; i++) {
        if (data->buffers[i].uri != nullptr && strlen(data->buffers[i].uri) > 0) {
            needsExternalBuffers = true;
            break;
        }
    }

    // 如果数据是嵌入式的，不需要加载外部buffer
    if (!needsExternalBuffers) {
        std::cout << "Using embedded data, no external buffers needed" << std::endl;
    } else {
        // 尝试加载外部buffer
        result = cgltf_load_buffers(&options, data, filepath.c_str());
        if (result != cgltf_result_success) {
            cgltf_free(data);
            throw std::runtime_error("Failed to load external buffers");
        }
    }

    vertices.clear();
    indices.clear();

    // 遍历所有网格
    for (size_t i = 0; i < data->meshes_count; i++) {
        const cgltf_mesh& mesh = data->meshes[i];
        std::cout << "Processing mesh " << i + 1 << "/" << data->meshes_count << std::endl;
        
        for (size_t j = 0; j < mesh.primitives_count; j++) {
            const cgltf_primitive& primitive = mesh.primitives[j];
            size_t vertexStart = vertices.size();
            
            // 首先找到顶点数量
            size_t vertexCount = 0;
            for (size_t k = 0; k < primitive.attributes_count; k++) {
                const cgltf_attribute& attribute = primitive.attributes[k];
                if (attribute.type == cgltf_attribute_type_position) {
                    vertexCount = attribute.data->count;
                    break;
                }
            }

            if (vertexCount == 0) {
                continue;
            }

            // 预分配顶点
            vertices.resize(vertexStart + vertexCount);
            
            // 设置默认值
            for (size_t v = vertexStart; v < vertexStart + vertexCount; v++) {
                vertices[v].color = glm::vec3(1.0f);
                vertices[v].normal = glm::vec3(0.0f, 1.0f, 0.0f);
                vertices[v].uv = glm::vec2(0.0f);
                vertices[v].tangent = glm::vec4(0.0f);
            }

            // 读取顶点属性
            for (size_t k = 0; k < primitive.attributes_count; k++) {
                const cgltf_attribute& attribute = primitive.attributes[k];
                const cgltf_accessor& accessor = *attribute.data;
                
                switch (attribute.type) {
                    case cgltf_attribute_type_position: {
                        for (size_t v = 0; v < accessor.count; v++) {
                            cgltf_accessor_read_float(&accessor, v, &vertices[vertexStart + v].position.x, 3);
                        }
                        break;
                    }
                    case cgltf_attribute_type_normal: {
                        for (size_t v = 0; v < accessor.count; v++) {
                            cgltf_accessor_read_float(&accessor, v, &vertices[vertexStart + v].normal.x, 3);
                        }
                        break;
                    }
                    case cgltf_attribute_type_texcoord: {
                        for (size_t v = 0; v < accessor.count; v++) {
                            cgltf_accessor_read_float(&accessor, v, &vertices[vertexStart + v].uv.x, 2);
                        }
                        break;
                    }
                    case cgltf_attribute_type_tangent: {
                        for (size_t v = 0; v < accessor.count; v++) {
                            cgltf_accessor_read_float(&accessor, v, &vertices[vertexStart + v].tangent.x, 4);
                        }
                        break;
                    }
                    case cgltf_attribute_type_color: {
                        for (size_t v = 0; v < accessor.count; v++) {
                            cgltf_accessor_read_float(&accessor, v, &vertices[vertexStart + v].color.x, 3);
                        }
                        break;
                    }
                }
            }

            // 读取索引
            if (primitive.indices) {
                size_t indexCount = primitive.indices->count;
                size_t oldSize = indices.size();
                indices.resize(oldSize + indexCount);
                
                for (size_t k = 0; k < indexCount; k++) {
                    indices[oldSize + k] = static_cast<uint32_t>(
                        cgltf_accessor_read_index(primitive.indices, k)
                    ) + static_cast<uint32_t>(vertexStart);
                }
            }
        }
    }

    std::cout << "Loaded " << vertices.size() << " vertices and " << indices.size() << " indices" << std::endl;

    // 如果需要，计算切线
    if (!indices.empty()) {
        computeTangents();
    }

    cgltf_free(data);
    std::cout << "GLTF load completed successfully" << std::endl;
}

void LveModel::Builder::computeTangents() {
    for (size_t i = 0; i < indices.size(); i += 3) {
        Vertex& v0 = vertices[indices[i]];
        Vertex& v1 = vertices[indices[i + 1]];
        Vertex& v2 = vertices[indices[i + 2]];

        glm::vec3 edge1 = v1.position - v0.position;
        glm::vec3 edge2 = v2.position - v0.position;
        glm::vec2 deltaUV1 = v1.uv - v0.uv;
        glm::vec2 deltaUV2 = v2.uv - v0.uv;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        if (std::isfinite(f)) {
            glm::vec3 tangent;
            tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
            tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
            tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
            tangent = glm::normalize(tangent);

            glm::vec3 bitangent;
            bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
            bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
            bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
            bitangent = glm::normalize(bitangent);

            float handedness = (glm::dot(glm::cross(v0.normal, tangent), bitangent) < 0.0f) ? -1.0f : 1.0f;

            v0.tangent = v1.tangent = v2.tangent = glm::vec4(tangent, handedness);
        }
    }
}

}  // namespace lve