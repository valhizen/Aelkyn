#include "commands.hpp"
#include "pipeline.hpp"
#include <array>
#include <cstdint>

void Commands::init(Context &device, GraphicsPipeline &graphicsPipeline,
                    Buffer &buffer) {
  this->device = &device;
  this->graphicsPipeline = &graphicsPipeline;
  this->buffer = &buffer;

  createCommandPool();
  createCommandBuffer();
}

void Commands::createCommandPool() {
  vk::CommandPoolCreateInfo poolInfo{
      .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      .queueFamilyIndex = device->getQueueFamilyIndex()};
  commandPool = vk::raii::CommandPool(device->getLogicalDevice(), poolInfo);
}

void Commands::createCommandBuffer() {
  vk::CommandBufferAllocateInfo allocInfo{
      .commandPool = commandPool,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = CMD_MAX_FLIGHT_FRAMES};

  auto buffers =
      vk::raii::CommandBuffers(device->getLogicalDevice(), allocInfo);
  for (auto &buf : buffers) {
    commandBuffer.push_back(std::move(buf));
  }
}

void Commands::recordCommandBuffer(uint32_t imageIndex, uint32_t frameIndex) {

  // FIX: Reset the command buffer before recording
  commandBuffer[frameIndex].reset();

  commandBuffer[frameIndex].begin({});
  transition_image_layout(imageIndex, frameIndex, vk::ImageLayout::eUndefined,
                          vk::ImageLayout::eColorAttachmentOptimal, {},
                          vk::AccessFlagBits2::eColorAttachmentWrite,
                          vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                          vk::PipelineStageFlagBits2::eColorAttachmentOutput);

  vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
  vk::RenderingAttachmentInfo attachmentInfo = {
      .imageView = device->getSwapChainImageView()[imageIndex],
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue = clearColor};
  vk::RenderingInfo renderingInfo = {
      .renderArea = {.offset = {0, 0}, .extent = device->getSwapChainExtent()},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &attachmentInfo};

  commandBuffer[frameIndex].beginRendering(renderingInfo);
  commandBuffer[frameIndex].bindPipeline(
      vk::PipelineBindPoint::eGraphics,
      *graphicsPipeline->getGraphicsPipeline());
    std::array<vk::DescriptorSet, 1> descriptorSets = {
      *graphicsPipeline->getDescriptorSets()[frameIndex]};
    commandBuffer[frameIndex].bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, *graphicsPipeline->getPipelineLayout(),
      0, descriptorSets, {});
      vk::Viewport viewport{};
      viewport.x = 0.0f;
      viewport.y = 0.0f;
      viewport.width = static_cast<float>(device->getSwapChainExtent().width);
      viewport.height = static_cast<float>(device->getSwapChainExtent().height);
      viewport.minDepth = 0.0f;
      viewport.maxDepth = 1.0f;
    commandBuffer[frameIndex].setViewport(0, viewport);

      vk::Rect2D scissor{};
      scissor.offset.x = 0;
      scissor.offset.y = 0;
      scissor.extent = device->getSwapChainExtent();
    commandBuffer[frameIndex].setScissor(0, scissor);

  // FIX: Bind the vertex buffer before drawing
  commandBuffer[frameIndex].bindVertexBuffers(0, *buffer->getVertexBuffer(),
                                              {0});
  commandBuffer[frameIndex].bindIndexBuffer(*buffer->getIndexBuffer(), 0,
                                            vk::IndexType::eUint16);
  commandBuffer[frameIndex].drawIndexed(buffer->indices.size(), 1, 0, 0, 0);
  commandBuffer[frameIndex].endRendering();

  transition_image_layout(imageIndex, frameIndex,
                          vk::ImageLayout::eColorAttachmentOptimal,
                          vk::ImageLayout::ePresentSrcKHR,
                          vk::AccessFlagBits2::eColorAttachmentWrite, {},
                          vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                          vk::PipelineStageFlagBits2::eBottomOfPipe);
  commandBuffer[frameIndex].end();
}

void Commands::transition_image_layout(uint32_t imageIndex, uint32_t frameIndex,
                                       vk::ImageLayout oldLayout,
                                       vk::ImageLayout newLayout,
                                       vk::AccessFlags2 srcAccessMask,
                                       vk::AccessFlags2 dstAccessMask,
                                       vk::PipelineStageFlags2 srcStageMask,
                                       vk::PipelineStageFlags2 dstStageMask) {
  vk::ImageMemoryBarrier2 barrier = {
      .srcStageMask = srcStageMask,
      .srcAccessMask = srcAccessMask,
      .dstStageMask = dstStageMask,
      .dstAccessMask = dstAccessMask,
      .oldLayout = oldLayout,
      .newLayout = newLayout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = device->getSwapChainImage()[imageIndex],
      .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                           .baseMipLevel = 0,
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1}};
  vk::DependencyInfo dependencyInfo = {.dependencyFlags = {},
                                       .imageMemoryBarrierCount = 1,
                                       .pImageMemoryBarriers = &barrier};
  commandBuffer[frameIndex].pipelineBarrier2(dependencyInfo);
}
