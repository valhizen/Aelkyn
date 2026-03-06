#pragma once

#include "device.hpp"
#include <vector>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

class SwapChain {
public:
  void init(Device &device, Window &window);
  void recreate();

  const vk::raii::SwapchainKHR &handle() const { return swapChain; }
  const vk::SurfaceFormatKHR &surfaceFormat() const { return format; }
  const vk::Extent2D &extent() const { return swapExtent; }
  const std::vector<vk::Image> &images() const { return images_; }
  const std::vector<vk::raii::ImageView> &imageViews() const {
    return imageViews_;
  }
  uint32_t imageCount() const { return static_cast<uint32_t>(images_.size()); }

private:
  void create();
  void createImageViews();
  void cleanup();

  vk::SurfaceFormatKHR
  chooseFormat(const std::vector<vk::SurfaceFormatKHR> &formats);
  vk::PresentModeKHR
  choosePresentMode(const std::vector<vk::PresentModeKHR> &modes);
  vk::Extent2D chooseExtent(const vk::SurfaceCapabilitiesKHR &caps);
  uint32_t chooseMinImageCount(const vk::SurfaceCapabilitiesKHR &caps);

  Device *device = nullptr;
  Window *window = nullptr;

  vk::raii::SwapchainKHR swapChain = nullptr;
  std::vector<vk::Image> images_;
  std::vector<vk::raii::ImageView> imageViews_;
  vk::SurfaceFormatKHR format;
  vk::Extent2D swapExtent;
};
