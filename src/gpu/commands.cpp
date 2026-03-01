#include "commands.hpp"
#include "pipeline.hpp"
#include <cstdint>

void Commands::init(Context &device, GraphicsPipeline &graphicsPipeline) {

  this->device = &device;
  this->graphicsPipeline = &graphicsPipeline;

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

  // commandBuffer =
  //     vk::raii::CommandBuffers(device->getLogicalDevice(), allocInfo);

  auto buffers =
      vk::raii::CommandBuffers(device->getLogicalDevice(), allocInfo);
  for (auto &buf : buffers) {
    commandBuffer.push_back(std::move(buf));
  }
}

void Commands::recordCommandBuffer(uint32_t imageIndex, uint32_t frameIndex) {

  commandBuffer[frameIndex].begin({});
  // Before starting rendering, transition the swapchain image to
  // COLOR_ATTACHMENT_OPTIMAL
  transition_image_layout(
      imageIndex, frameIndex, vk::ImageLayout::eUndefined,
      vk::ImageLayout::eColorAttachmentOptimal,
      {}, // srcAccessMask (no need to wait for previous operations)
      vk::AccessFlagBits2::eColorAttachmentWrite,         // dstAccessMask
      vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
      vk::PipelineStageFlagBits2::eColorAttachmentOutput  // dstStage
  );
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
  commandBuffer[frameIndex].setViewport(
      0, vk::Viewport(0.0f, 0.0f,
                      static_cast<float>(device->getSwapChainExtent().width),
                      static_cast<float>(device->getSwapChainExtent().height),
                      0.0f, 1.0f));
  commandBuffer[frameIndex].setScissor(
      0, vk::Rect2D(vk::Offset2D(0, 0), device->getSwapChainExtent()));
  commandBuffer[frameIndex].draw(3, 1, 0, 0);
  commandBuffer[frameIndex].endRendering();
  // After rendering, transition the swapchain image to PRESENT_SRC
  transition_image_layout(
      imageIndex, frameIndex, vk::ImageLayout::eColorAttachmentOptimal,
      vk::ImageLayout::ePresentSrcKHR,
      vk::AccessFlagBits2::eColorAttachmentWrite,         // srcAccessMask
      {},                                                 // dstAccessMask
      vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
      vk::PipelineStageFlagBits2::eBottomOfPipe           // dstStage
  );
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
