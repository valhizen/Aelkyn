#pragma once

#include "../core/constants.hpp"
#include "../core/rendertypes.hpp"
#include "device.hpp"
#include <vector>

class Buffer {
public:
  void init(Device &device, const std::vector<Vertex> &vertices,
            const std::vector<uint32_t> &indices);

  void updateUniformBuffer(uint32_t currentImage, const vk::Extent2D &extent);

  const vk::raii::Buffer &getVertexBuffer() const { return vertexBuffer; }
  const vk::raii::Buffer &getIndexBuffer() const { return indexBuffer; }
  const std::vector<vk::raii::Buffer> &getUniformBuffers() const {
    return uniformBuffers;
  }

  uint32_t indexCount() const { return indexCount_; }

private:
  void createVertexBuffer();
  void createIndexBuffer();
  void createUniformBuffers();

  Device *device = nullptr;

  // CPU-side copies kept for staging upload
  std::vector<Vertex> vertices_;
  std::vector<uint32_t> indices_;
  uint32_t indexCount_ = 0;

  vk::raii::Buffer vertexBuffer = nullptr;
  vk::raii::DeviceMemory vertexBufferMemory = nullptr;
  vk::raii::Buffer indexBuffer = nullptr;
  vk::raii::DeviceMemory indexBufferMemory = nullptr;

  std::vector<vk::raii::Buffer> uniformBuffers;
  std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
  std::vector<void *> uniformBuffersMapped;
};
