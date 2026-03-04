#pragma once

#include "context.hpp"
#include <array>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <glm/glm.hpp>
#include <vulkan/vulkan_raii.hpp>

struct Vertex {
  glm::vec2 pos;
  glm::vec3 color;

  static vk::VertexInputBindingDescription getBindingDescription() {
    return {0, sizeof(Vertex), vk::VertexInputRate::eVertex};
  }

  static std::array<vk::VertexInputAttributeDescription, 2>
  getAttributeDescriptions() {
    return {vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat,
                                                offsetof(Vertex, pos)),
            vk::VertexInputAttributeDescription(
                1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color))};
  }
};

class Buffer {
private:
  vk::raii::Buffer vertexBuffer = nullptr;
  vk::raii::DeviceMemory vertexBufferMemory = nullptr;
  vk::raii::Buffer indexBuffer = nullptr;
  vk::raii::DeviceMemory indexBufferMemory = nullptr;

  Context *device;

  uint32_t findMemoryType(uint32_t typeFilter,
                          vk::MemoryPropertyFlags properties);
  void createVertexBuffer();
  void createIndexBuffer();

  void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                    vk::MemoryPropertyFlags properties,
                    vk::raii::Buffer &buffer,
                    vk::raii::DeviceMemory &bufferMemory);

public:
  void init(Context &device);
  const vk::raii::Buffer &getVertexBuffer() const { return vertexBuffer; }
  const vk::raii::Buffer &getIndexBuffer() const { return indexBuffer; }

  const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};
};
