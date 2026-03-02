#pragma once

#include "buffer.hpp"
#include "pipeline.hpp"
#include <cstdint>
#include <vector>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

constexpr int CMD_MAX_FLIGHT_FRAMES = 2;

class Commands {

private:
  void createCommandPool();
  void createCommandBuffer();

  void transition_image_layout(uint32_t imageIndex, uint32_t frameIndex,
                               vk::ImageLayout oldLayout,
                               vk::ImageLayout newLayout,
                               vk::AccessFlags2 srcAccessMask,
                               vk::AccessFlags2 dstAccessMask,
                               vk::PipelineStageFlags2 srcStageMask,
                               vk::PipelineStageFlags2 dstStageMask);

  Context *device;
  GraphicsPipeline *graphicsPipeline;
  Buffer *buffer;
  vk::raii::CommandPool commandPool = nullptr;
  std::vector<vk::raii::CommandBuffer> commandBuffer;

public:
  void init(Context &context, GraphicsPipeline &graphicsPipeline,
            Buffer &buffer);

  void recordCommandBuffer(uint32_t imageIndex, uint32_t frameIndex);
  const vk::raii::CommandBuffer &getCommandBuffer(uint32_t frameIndex) const {
    return commandBuffer[frameIndex];
  }
  const vk::raii::CommandPool &getCommandPool() const { return commandPool; }
};
