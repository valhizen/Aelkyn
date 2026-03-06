#pragma once

#include "../core/constants.hpp"
#include "../core/rendertypes.hpp"
#include "device.hpp"
#include <vector>

class Buffer {
public:
  void init(Device &device);

  void updateUniformBuffer(uint32_t currentImage, const vk::Extent2D &extent);

  const vk::raii::Buffer &getVertexBuffer() const { return vertexBuffer; }
  const vk::raii::Buffer &getIndexBuffer() const { return indexBuffer; }
  const std::vector<vk::raii::Buffer> &getUniformBuffers() const {
    return uniformBuffers;
  }

  // const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};
  const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4};

private:
  void createVertexBuffer();
  void createIndexBuffer();
  void createUniformBuffers();

  Device *device = nullptr;

  vk::raii::Buffer vertexBuffer = nullptr;
  vk::raii::DeviceMemory vertexBufferMemory = nullptr;
  vk::raii::Buffer indexBuffer = nullptr;
  vk::raii::DeviceMemory indexBufferMemory = nullptr;

  std::vector<vk::raii::Buffer> uniformBuffers;
  std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
  std::vector<void *> uniformBuffersMapped;
};
