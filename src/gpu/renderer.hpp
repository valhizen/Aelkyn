#pragma once

#include "../core/constants.hpp"
#include "buffer.hpp"
#include "device.hpp"
#include "pipeline.hpp"
#include "swapchain.hpp"
#include <vector>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

class Renderer {
public:
  void init(Device &device, SwapChain &swapChain, GraphicsPipeline &pipeline,
            Buffer &buffer, Window &window);
  void drawFrame();
  void waitIdle() { device->logical().waitIdle(); }

private:
  void createCommandPool();
  void createCommandBuffers();
  void createSyncObjects();

  void recordCommandBuffer(uint32_t imageIndex, uint32_t frameIndex);
  void transitionImageLayout(uint32_t imageIndex, uint32_t frameIndex,
                             vk::ImageLayout oldLayout,
                             vk::ImageLayout newLayout,
                             vk::AccessFlags2 srcAccessMask,
                             vk::AccessFlags2 dstAccessMask,
                             vk::PipelineStageFlags2 srcStageMask,
                             vk::PipelineStageFlags2 dstStageMask);

  Device *device = nullptr;
  SwapChain *swapChain = nullptr;
  GraphicsPipeline *pipeline = nullptr;
  Buffer *buffer = nullptr;
  Window *window = nullptr;

  vk::raii::CommandPool commandPool = nullptr;
  std::vector<vk::raii::CommandBuffer> commandBuffers;

  std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
  std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
  std::vector<vk::raii::Fence> inFlightFences;

  uint32_t currentFrame = 0;
};
