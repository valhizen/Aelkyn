#include "buffer.hpp"
#include <chrono>
#include <cstring>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}};

void Buffer::init(Device &device) {
  this->device = &device;
  createVertexBuffer();
  createIndexBuffer();
  createUniformBuffers();
}

void Buffer::createVertexBuffer() {
  vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

  vk::raii::Buffer stagingBuffer({});
  vk::raii::DeviceMemory stagingMemory({});
  device->createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                       vk::MemoryPropertyFlagBits::eHostVisible |
                           vk::MemoryPropertyFlagBits::eHostCoherent,
                       stagingBuffer, stagingMemory);

  void *data = stagingMemory.mapMemory(0, bufferSize);
  memcpy(data, vertices.data(), bufferSize);
  stagingMemory.unmapMemory();

  device->createBuffer(bufferSize,
                       vk::BufferUsageFlagBits::eVertexBuffer |
                           vk::BufferUsageFlagBits::eTransferDst,
                       vk::MemoryPropertyFlagBits::eDeviceLocal, vertexBuffer,
                       vertexBufferMemory);

  device->copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
}

void Buffer::createIndexBuffer() {
  vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

  vk::raii::Buffer stagingBuffer({});
  vk::raii::DeviceMemory stagingMemory({});
  device->createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                       vk::MemoryPropertyFlagBits::eHostVisible |
                           vk::MemoryPropertyFlagBits::eHostCoherent,
                       stagingBuffer, stagingMemory);

  void *data = stagingMemory.mapMemory(0, bufferSize);
  memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
  stagingMemory.unmapMemory();

  device->createBuffer(bufferSize,
                       vk::BufferUsageFlagBits::eTransferDst |
                           vk::BufferUsageFlagBits::eIndexBuffer,
                       vk::MemoryPropertyFlagBits::eDeviceLocal, indexBuffer,
                       indexBufferMemory);

  device->copyBuffer(stagingBuffer, indexBuffer, bufferSize);
}

void Buffer::createUniformBuffers() {
  uniformBuffers.clear();
  uniformBuffersMemory.clear();
  uniformBuffersMapped.clear();

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
    vk::raii::Buffer buffer({});
    vk::raii::DeviceMemory bufferMem({});
    device->createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
                         vk::MemoryPropertyFlagBits::eHostVisible |
                             vk::MemoryPropertyFlagBits::eHostCoherent,
                         buffer, bufferMem);
    uniformBuffers.emplace_back(std::move(buffer));
    uniformBuffersMemory.emplace_back(std::move(bufferMem));
    uniformBuffersMapped.emplace_back(
        uniformBuffersMemory[i].mapMemory(0, bufferSize));
  }
}

void Buffer::updateUniformBuffer(uint32_t currentImage,
                                 const vk::Extent2D &extent) {
  static auto startTime = std::chrono::high_resolution_clock::now();

  auto currentTime = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration<float, std::chrono::seconds::period>(
                   currentTime - startTime)
                   .count();

  UniformBufferObject ubo{};
  ubo.model = rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                     glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.view = lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.proj = glm::perspective(glm::radians(45.0f),
                              static_cast<float>(extent.width) /
                                  static_cast<float>(extent.height),
                              0.1f, 10.0f);
  ubo.proj[1][1] *= -1;
  memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}
