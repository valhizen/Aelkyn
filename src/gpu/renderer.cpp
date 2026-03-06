#include "renderer.hpp"
#include <array>
#include <cassert>
#include <cstdint>

void Renderer::init(Device &device, SwapChain &swapChain,
                    GraphicsPipeline &pipeline, Buffer &buffer,
                    Window &window) {
  this->device = &device;
  this->swapChain = &swapChain;
  this->pipeline = &pipeline;
  this->buffer = &buffer;
  this->window = &window;

  createCommandPool();
  createCommandBuffers();
  createSyncObjects();
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

void Renderer::createCommandPool() {
  vk::CommandPoolCreateInfo poolInfo{
      .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      .queueFamilyIndex = device->queueFamily()};
  commandPool = vk::raii::CommandPool(device->logical(), poolInfo);
}

void Renderer::createCommandBuffers() {
  vk::CommandBufferAllocateInfo allocInfo{
      .commandPool = commandPool,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = MAX_FRAMES_IN_FLIGHT};

  auto buffers = vk::raii::CommandBuffers(device->logical(), allocInfo);
  for (auto &buf : buffers) {
    commandBuffers.push_back(std::move(buf));
  }
}

void Renderer::createSyncObjects() {
  for (size_t i = 0; i < swapChain->imageCount(); i++) {
    renderFinishedSemaphores.emplace_back(device->logical(),
                                          vk::SemaphoreCreateInfo());
  }

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    presentCompleteSemaphores.emplace_back(device->logical(),
                                           vk::SemaphoreCreateInfo());
    inFlightFences.emplace_back(
        device->logical(),
        vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
  }
}

// ---------------------------------------------------------------------------
// Frame loop
// ---------------------------------------------------------------------------

void Renderer::drawFrame() {
  auto fenceResult = device->logical().waitForFences(
      *inFlightFences[currentFrame], vk::True, UINT64_MAX);
  if (fenceResult != vk::Result::eSuccess) {
    throw std::runtime_error("failed to wait for fence!");
  }

  auto [acquireResult, imageIndex] = swapChain->handle().acquireNextImage(
      UINT64_MAX, *presentCompleteSemaphores[currentFrame], nullptr);

  if (acquireResult == vk::Result::eErrorOutOfDateKHR) {
    swapChain->recreate();
    return;
  }
  if (acquireResult != vk::Result::eSuccess &&
      acquireResult != vk::Result::eSuboptimalKHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  device->logical().resetFences(*inFlightFences[currentFrame]);

  buffer->updateUniformBuffer(currentFrame, swapChain->extent());

  recordCommandBuffer(imageIndex, currentFrame);

  vk::PipelineStageFlags waitStage(
      vk::PipelineStageFlagBits::eColorAttachmentOutput);

  const vk::SubmitInfo submitInfo{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &*presentCompleteSemaphores[currentFrame],
      .pWaitDstStageMask = &waitStage,
      .commandBufferCount = 1,
      .pCommandBuffers = &*commandBuffers[currentFrame],
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &*renderFinishedSemaphores[imageIndex]};
  device->queue().submit(submitInfo, *inFlightFences[currentFrame]);

  const vk::PresentInfoKHR presentInfo{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &*renderFinishedSemaphores[imageIndex],
      .swapchainCount = 1,
      .pSwapchains = &*swapChain->handle(),
      .pImageIndices = &imageIndex};
  auto presentResult = device->queue().presentKHR(presentInfo);

  if (presentResult == vk::Result::eSuboptimalKHR ||
      presentResult == vk::Result::eErrorOutOfDateKHR || window->wasResized()) {
    window->resetResizedFlag();
    swapChain->recreate();
  } else {
    assert(presentResult == vk::Result::eSuccess);
  }

  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

// ---------------------------------------------------------------------------
// Command recording
// ---------------------------------------------------------------------------

void Renderer::recordCommandBuffer(uint32_t imageIndex, uint32_t frameIndex) {
  commandBuffers[frameIndex].reset();
  commandBuffers[frameIndex].begin({});

  // Transition: color image undefined → color attachment optimal
  transitionImageLayout(swapChain->images()[imageIndex], frameIndex,
                        vk::ImageLayout::eUndefined,
                        vk::ImageLayout::eColorAttachmentOptimal, {},
                        vk::AccessFlagBits2::eColorAttachmentWrite,
                        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                        vk::ImageAspectFlagBits::eColor);

  // Transition: depth image undefined → depth attachment optimal
  transitionImageLayout(swapChain->depthImage(), frameIndex,
                        vk::ImageLayout::eUndefined,
                        vk::ImageLayout::eDepthAttachmentOptimal,
                        vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
                        vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
                        vk::PipelineStageFlagBits2::eEarlyFragmentTests |
                            vk::PipelineStageFlagBits2::eLateFragmentTests,
                        vk::PipelineStageFlagBits2::eEarlyFragmentTests |
                            vk::PipelineStageFlagBits2::eLateFragmentTests,
                        vk::ImageAspectFlagBits::eDepth);

  vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
  vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);

  vk::RenderingAttachmentInfo attachmentInfo = {
      .imageView = swapChain->imageViews()[imageIndex],
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue = clearColor};

  vk::RenderingAttachmentInfo depthAttachmentInfo = {
      .imageView = swapChain->depthImageView(),
      .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eDontCare,
      .clearValue = clearDepth};

  vk::RenderingInfo renderingInfo = {
      .renderArea = {.offset = {0, 0}, .extent = swapChain->extent()},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &attachmentInfo,
      .pDepthAttachment = &depthAttachmentInfo};

  commandBuffers[frameIndex].beginRendering(renderingInfo);

  commandBuffers[frameIndex].bindPipeline(vk::PipelineBindPoint::eGraphics,
                                          *pipeline->getPipeline());

  std::array<vk::DescriptorSet, 1> descriptorSets = {
      *pipeline->getDescriptorSets()[frameIndex]};
  commandBuffers[frameIndex].bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, *pipeline->getLayout(), 0,
      descriptorSets, {});

  vk::Viewport viewport{.x = 0.0f,
                        .y = 0.0f,
                        .width = static_cast<float>(swapChain->extent().width),
                        .height =
                            static_cast<float>(swapChain->extent().height),
                        .minDepth = 0.0f,
                        .maxDepth = 1.0f};
  commandBuffers[frameIndex].setViewport(0, viewport);

  vk::Rect2D scissor{.offset = {0, 0}, .extent = swapChain->extent()};
  commandBuffers[frameIndex].setScissor(0, scissor);

  commandBuffers[frameIndex].bindVertexBuffers(0, *buffer->getVertexBuffer(),
                                               {0});
  commandBuffers[frameIndex].bindIndexBuffer(*buffer->getIndexBuffer(), 0,
                                             vk::IndexType::eUint16);
  commandBuffers[frameIndex].drawIndexed(buffer->indices.size(), 1, 0, 0, 0);

  commandBuffers[frameIndex].endRendering();

  // Transition: color attachment → present
  transitionImageLayout(swapChain->images()[imageIndex], frameIndex,
                        vk::ImageLayout::eColorAttachmentOptimal,
                        vk::ImageLayout::ePresentSrcKHR,
                        vk::AccessFlagBits2::eColorAttachmentWrite, {},
                        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                        vk::PipelineStageFlagBits2::eBottomOfPipe,
                        vk::ImageAspectFlagBits::eColor);

  commandBuffers[frameIndex].end();
}

// ---------------------------------------------------------------------------
// Image layout transitions
// ---------------------------------------------------------------------------

void Renderer::transitionImageLayout(
    vk::Image image, uint32_t frameIndex, vk::ImageLayout oldLayout,
    vk::ImageLayout newLayout, vk::AccessFlags2 srcAccessMask,
    vk::AccessFlags2 dstAccessMask, vk::PipelineStageFlags2 srcStageMask,
    vk::PipelineStageFlags2 dstStageMask, vk::ImageAspectFlags aspectFlags) {
  vk::ImageMemoryBarrier2 barrier = {
      .srcStageMask = srcStageMask,
      .srcAccessMask = srcAccessMask,
      .dstStageMask = dstStageMask,
      .dstAccessMask = dstAccessMask,
      .oldLayout = oldLayout,
      .newLayout = newLayout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image,
      .subresourceRange = {.aspectMask = aspectFlags,
                           .baseMipLevel = 0,
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1}};
  vk::DependencyInfo dependencyInfo = {.dependencyFlags = {},
                                       .imageMemoryBarrierCount = 1,
                                       .pImageMemoryBarriers = &barrier};
  commandBuffers[frameIndex].pipelineBarrier2(dependencyInfo);
}
