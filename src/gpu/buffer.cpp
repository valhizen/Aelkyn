#include "buffer.hpp"
#include "vulkan/vulkan.hpp"
#include <cstring>
#include <vector>

// const std::vector<Vertex> vertices = {{{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
//                                       {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
//                                       {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};
const std::vector<Vertex> vertices = {{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                      {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                                      {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
                                      {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};

void Buffer::init(Context &device) {

  this->device = &device;
  createVertexBuffer();
  createIndexBuffer();
}

void Buffer::createVertexBuffer() {
  vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

  vk::BufferCreateInfo stagingInfo{.size = bufferSize,
                                   .usage =
                                       vk::BufferUsageFlagBits::eTransferSrc,
                                   .sharingMode = vk::SharingMode::eExclusive};
  vk::raii::Buffer stagingBuffer(device->getLogicalDevice(), stagingInfo);
  vk::MemoryRequirements memRequirementsStaging =
      stagingBuffer.getMemoryRequirements();
  vk::MemoryAllocateInfo memoryAllocateInfoStaging{
      .allocationSize = memRequirementsStaging.size,
      .memoryTypeIndex =
          findMemoryType(memRequirementsStaging.memoryTypeBits,
                         vk::MemoryPropertyFlagBits::eHostVisible |
                             vk::MemoryPropertyFlagBits::eHostCoherent)};
  vk::raii::DeviceMemory stagingBufferMemory(device->getLogicalDevice(),
                                             memoryAllocateInfoStaging);

  stagingBuffer.bindMemory(stagingBufferMemory, 0);
  void *dataStaging = stagingBufferMemory.mapMemory(0, stagingInfo.size);
  memcpy(dataStaging, vertices.data(), stagingInfo.size);
  stagingBufferMemory.unmapMemory();

  vk::BufferCreateInfo bufferInfo{.size = bufferSize,
                                  .usage =
                                      vk::BufferUsageFlagBits::eVertexBuffer |
                                      vk::BufferUsageFlagBits::eTransferDst,
                                  .sharingMode = vk::SharingMode::eExclusive};
  vertexBuffer = vk::raii::Buffer(device->getLogicalDevice(), bufferInfo);

  vk::MemoryRequirements memRequirements = vertexBuffer.getMemoryRequirements();
  vk::MemoryAllocateInfo memoryAllocateInfo{
      .allocationSize = memRequirements.size,
      .memoryTypeIndex =
          findMemoryType(memRequirements.memoryTypeBits,
                         vk::MemoryPropertyFlagBits::eDeviceLocal)};
  vertexBufferMemory =
      vk::raii::DeviceMemory(device->getLogicalDevice(), memoryAllocateInfo);

  vertexBuffer.bindMemory(*vertexBufferMemory, 0);

  device->copyBuffer(stagingBuffer, vertexBuffer, stagingInfo.size);
}

void Buffer::createIndexBuffer() {
  vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

  vk::raii::Buffer stagingBuffer({});
  vk::raii::DeviceMemory stagingBufferMemory({});
  createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
               vk::MemoryPropertyFlagBits::eHostVisible |
                   vk::MemoryPropertyFlagBits::eHostCoherent,
               stagingBuffer, stagingBufferMemory);

  void *data = stagingBufferMemory.mapMemory(0, bufferSize);
  memcpy(data, indices.data(), (size_t)bufferSize);
  stagingBufferMemory.unmapMemory();

  createBuffer(bufferSize,
               vk::BufferUsageFlagBits::eTransferDst |
                   vk::BufferUsageFlagBits::eIndexBuffer,
               vk::MemoryPropertyFlagBits::eDeviceLocal, indexBuffer,
               indexBufferMemory);

  device->copyBuffer(stagingBuffer, indexBuffer, bufferSize);
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

void Buffer::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                          vk::MemoryPropertyFlags properties,
                          vk::raii::Buffer &buffer,
                          vk::raii::DeviceMemory &bufferMemory) {
  vk::BufferCreateInfo bufferInfo{
      .size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive};

  buffer = vk::raii::Buffer(device->getLogicalDevice(), bufferInfo);
  vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
  vk::MemoryAllocateInfo allocInfo{
      .allocationSize = memRequirements.size,
      .memoryTypeIndex =
          findMemoryType(memRequirements.memoryTypeBits, properties)};
  bufferMemory = vk::raii::DeviceMemory(device->getLogicalDevice(), allocInfo);
  buffer.bindMemory(*bufferMemory, 0);
}
