#include "buffer.hpp"
#include <cstring>
#include <vector>

const std::vector<Vertex> vertices = {{{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
                                      {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                                      {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

void Buffer::init(Context &device) {
  this->device = &device;
  createVertexBuffer();
}

void Buffer::createVertexBuffer() {
  vk::BufferCreateInfo bufferInfo{.size = sizeof(vertices[0]) * vertices.size(),
                                  .usage =
                                      vk::BufferUsageFlagBits::eVertexBuffer,
                                  .sharingMode = vk::SharingMode::eExclusive};
  vertexBuffer = vk::raii::Buffer(device->getLogicalDevice(), bufferInfo);

  vk::MemoryRequirements memRequirements = vertexBuffer.getMemoryRequirements();
  vk::MemoryAllocateInfo memoryAllocateInfo{
      .allocationSize = memRequirements.size,
      .memoryTypeIndex =
          findMemoryType(memRequirements.memoryTypeBits,
                         vk::MemoryPropertyFlagBits::eHostVisible |
                             vk::MemoryPropertyFlagBits::eHostCoherent)};
  vertexBufferMemory =
      vk::raii::DeviceMemory(device->getLogicalDevice(), memoryAllocateInfo);

  vertexBuffer.bindMemory(*vertexBufferMemory, 0);

  void *data = vertexBufferMemory.mapMemory(0, bufferInfo.size);
  memcpy(data, vertices.data(), bufferInfo.size);
  vertexBufferMemory.unmapMemory();
}

uint32_t Buffer::findMemoryType(uint32_t typeFilter,
                                vk::MemoryPropertyFlags properties) {
  vk::PhysicalDeviceMemoryProperties memProperties =
      device->getPhysicalDevice().getMemoryProperties();

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags &
                                    properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}
