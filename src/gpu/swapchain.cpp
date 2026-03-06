#include "swapchain.hpp"
#include <algorithm>
#include <cassert>
#include <limits>

void SwapChain::init(Device &device, Window &window) {
  this->device = &device;
  this->window = &window;
  create();
  createImageViews();
}

void SwapChain::recreate() {
  int width = 0, height = 0;
  glfwGetFramebufferSize(window->getWindow(), &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(window->getWindow(), &width, &height);
    glfwWaitEvents();
  }

  device->logical().waitIdle();

  cleanup();
  create();
  createImageViews();
}

void SwapChain::cleanup() {
  imageViews_.clear();
  swapChain = nullptr;
}

// Swap chain creation

void SwapChain::create() {
  auto surfaceCapabilities =
      device->physical().getSurfaceCapabilitiesKHR(*device->surface());
  swapExtent = chooseExtent(surfaceCapabilities);
  format =
      chooseFormat(device->physical().getSurfaceFormatsKHR(*device->surface()));

  vk::SwapchainCreateInfoKHR swapChainCreateInfo{
      .surface = *device->surface(),
      .minImageCount = chooseMinImageCount(surfaceCapabilities),
      .imageFormat = format.format,
      .imageColorSpace = format.colorSpace,
      .imageExtent = swapExtent,
      .imageArrayLayers = 1,
      .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
      .imageSharingMode = vk::SharingMode::eExclusive,
      .preTransform = surfaceCapabilities.currentTransform,
      .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
      .presentMode = choosePresentMode(
          device->physical().getSurfacePresentModesKHR(*device->surface())),
      .clipped = true};

  swapChain = vk::raii::SwapchainKHR(device->logical(), swapChainCreateInfo);
  images_ = swapChain.getImages();
}

void SwapChain::createImageViews() {
  assert(imageViews_.empty());

  vk::ImageViewCreateInfo imageViewCreateInfo{
      .viewType = vk::ImageViewType::e2D,
      .format = format.format,
      .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
  for (auto &image : images_) {
    imageViewCreateInfo.image = image;
    imageViews_.emplace_back(device->logical(), imageViewCreateInfo);
  }
}

// Selection helpers

vk::SurfaceFormatKHR SwapChain::chooseFormat(
    const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
        availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return availableFormat;
    }
  }
  return availableFormats[0];
}

vk::PresentModeKHR SwapChain::choosePresentMode(
    const std::vector<vk::PresentModeKHR> &availablePresentModes) {
  for (const auto &mode : availablePresentModes) {
    if (mode == vk::PresentModeKHR::eMailbox) {
      return mode;
    }
  }
  return vk::PresentModeKHR::eFifo;
}

vk::Extent2D
SwapChain::chooseExtent(const vk::SurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  }
  int width, height;
  glfwGetFramebufferSize(window->getWindow(), &width, &height);

  return {std::clamp<uint32_t>(width, capabilities.minImageExtent.width,
                               capabilities.maxImageExtent.width),
          std::clamp<uint32_t>(height, capabilities.minImageExtent.height,
                               capabilities.maxImageExtent.height)};
}

uint32_t SwapChain::chooseMinImageCount(
    const vk::SurfaceCapabilitiesKHR &surfaceCapabilities) {
  auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
  if ((0 < surfaceCapabilities.maxImageCount) &&
      (surfaceCapabilities.maxImageCount < minImageCount)) {
    minImageCount = surfaceCapabilities.maxImageCount;
  }
  return minImageCount;
}
