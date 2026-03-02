#include "frames.hpp"
#include "commands.hpp"
#include "context.hpp"

void Frames::init(Context &device, Commands &commands, Window &window) {
  this->device = &device;
  this->commands = &commands;
  this->window = &window;
  createSyncObjects();
}

void Frames::createSyncObjects() {
  for (size_t i = 0; i < device->getSwapChainImageCount(); i++) {
    renderFinishedSemaphores.emplace_back(device->getLogicalDevice(),
                                          vk::SemaphoreCreateInfo());
  }

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    presentCompleteSemaphores.emplace_back(device->getLogicalDevice(),
                                           vk::SemaphoreCreateInfo());
    inFlightFences.emplace_back(
        device->getLogicalDevice(),
        vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
  }
}

void Frames::drawFrame() {

  auto fenceResult = device->getLogicalDevice().waitForFences(
      *inFlightFences[currentFrame], vk::True, UINT64_MAX);
  if (fenceResult != vk::Result::eSuccess) {
    throw std::runtime_error("failed to wait for fence!");
  }

  auto [acquireResult, imageIndex] = device->getSwapChain().acquireNextImage(
      UINT64_MAX, *presentCompleteSemaphores[currentFrame], nullptr);

  if (acquireResult == vk::Result::eErrorOutOfDateKHR) {
    device->recreateSwapChain();
    return;
  }
  if (acquireResult != vk::Result::eSuccess &&
      acquireResult != vk::Result::eSuboptimalKHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  // Only reset fence after we know we're submitting work
  device->getLogicalDevice().resetFences(*inFlightFences[currentFrame]);

  commands->recordCommandBuffer(imageIndex, currentFrame);

  vk::PipelineStageFlags waitDestinationStageMask(
      vk::PipelineStageFlagBits::eColorAttachmentOutput);

  const vk::SubmitInfo submitInfo{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &*presentCompleteSemaphores[currentFrame],
      .pWaitDstStageMask = &waitDestinationStageMask,
      .commandBufferCount = 1,
      .pCommandBuffers = &*commands->getCommandBuffer(currentFrame),
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &*renderFinishedSemaphores[imageIndex]};
  device->getQueue().submit(submitInfo, *inFlightFences[currentFrame]);

  const vk::PresentInfoKHR presentInfoKHR{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &*renderFinishedSemaphores[imageIndex],
      .swapchainCount = 1,
      .pSwapchains = &*device->getSwapChain(),
      .pImageIndices = &imageIndex};
  auto presentResult = device->getQueue().presentKHR(presentInfoKHR);

  if (presentResult == vk::Result::eSuboptimalKHR ||
      presentResult == vk::Result::eErrorOutOfDateKHR || window->wasResized()) {
    window->resetResizedFlag();
    device->recreateSwapChain();
  } else {
    assert(presentResult == vk::Result::eSuccess);
  }

  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
