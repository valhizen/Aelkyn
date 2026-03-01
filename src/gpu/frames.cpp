#include "frames.hpp"
#include "commands.hpp"
#include "context.hpp"
#include <iostream>

void Frames::init(Context &device, Commands &commands) {
  this->device = &device;
  this->commands = &commands;
  createSyncObjects();
}

void Frames::createSyncObjects() {
  presentCompleteSemaphore = vk::raii::Semaphore(device->getLogicalDevice(),
                                                 vk::SemaphoreCreateInfo());
  renderFinishedSemaphore = vk::raii::Semaphore(device->getLogicalDevice(),
                                                vk::SemaphoreCreateInfo());
  drawFence = vk::raii::Fence(device->getLogicalDevice(),
                              {.flags = vk::FenceCreateFlagBits::eSignaled});
}

void Frames::drawFrame() {

  device->getQueue()
      .waitIdle(); // NOTE: for simplicity, wait for the queue to be idle
                   // before starting the frame In the next chapter you see how
                   // to use multiple frames in flight and fences to sync

  auto [result, imageIndex] = device->getSwapChain().acquireNextImage(
      UINT64_MAX, *presentCompleteSemaphore, nullptr);
  commands->recordCommandBuffer(imageIndex);

  device->getLogicalDevice().resetFences(*drawFence);
  vk::PipelineStageFlags waitDestinationStageMask(
      vk::PipelineStageFlagBits::eColorAttachmentOutput);
  const vk::SubmitInfo submitInfo{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &*presentCompleteSemaphore,
      .pWaitDstStageMask = &waitDestinationStageMask,
      .commandBufferCount = 1,
      .pCommandBuffers = &*commands->getCommandBuffer(),
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &*renderFinishedSemaphore};
  device->getQueue().submit(submitInfo, *drawFence);
  result = device->getLogicalDevice().waitForFences(*drawFence, vk::True,
                                                    UINT64_MAX);
  if (result != vk::Result::eSuccess) {
    throw std::runtime_error("failed to wait for fence!");
  }

  const vk::PresentInfoKHR presentInfoKHR{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &*renderFinishedSemaphore,
      .swapchainCount = 1,
      .pSwapchains = &*device->getSwapChain(),
      .pImageIndices = &imageIndex};
  result = device->getQueue().presentKHR(presentInfoKHR);
  switch (result) {
  case vk::Result::eSuccess:
    break;
  case vk::Result::eSuboptimalKHR:
    std::cout
        << "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR !\n";
    break;
  default:
    break; // an unexpected result is returned!
  }
}
