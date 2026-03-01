#pragma once

#include "pipeline.hpp"

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

class Commands {

private:
  void createCommandPool();
  void createCommandBuffer();
  void createSyncObjects();

  void transition_image_layout(uint32_t imageIndex, vk::ImageLayout oldLayout,
                               vk::ImageLayout newLayout,
                               vk::AccessFlags2 srcAccessMask,
                               vk::AccessFlags2 dstAccessMask,
                               vk::PipelineStageFlags2 srcStageMask,
                               vk::PipelineStageFlags2 dstStageMask);

  Context *device;
  GraphicsPipeline *graphicsPipeline;
  vk::raii::CommandPool commandPool = nullptr;
  vk::raii::CommandBuffer commandBuffer = nullptr;

public:
  void init(Context &context, GraphicsPipeline &graphicsPipeline);

  void recordCommandBuffer(uint32_t imageIndex);
  const vk::raii::CommandBuffer &getCommandBuffer() const {
    return commandBuffer;
  }
};
