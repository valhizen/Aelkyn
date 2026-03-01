#pragma once

#include "../window.hpp"
#include <cstdint>
#include <sys/types.h>
#include <vector>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

class Context {
public:
  void init(Window &window);
  void drawFrame();
  void recreateSwapChain();
  void cleanupSwapChain();
  // bool enableValidationLayer = true;
  // Helper Functions
  const vk::raii::Device &getLogicalDevice() const { return logicalDevice; }
  const vk::SurfaceFormatKHR &getSurfaceFormat() const {
    return swapChainSurfaceFormat;
  }
  const uint32_t &getQueueFamilyIndex() const {
    return graphicsQueueFamilyIndex;
  }

  const std::vector<vk::Image> &getSwapChainImage() const {
    return swapChainImages;
  }

  const vk::raii::Queue &getQueue() const { return graphicsQueue; }
  const vk::raii::SwapchainKHR &getSwapChain() const { return swapChain; }

  const std::vector<vk::raii::ImageView> &getSwapChainImageView() const {
    return swapChainImageViews;
  }

  const vk::Extent2D &getSwapChainExtent() const { return swapChainExtent; }

private:
  void createInstance();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void findQueueFamilies(const vk::raii::SurfaceKHR surface);
  void createSurface();
  void createSwapChain();
  void createImageViews();

  uint32_t chooseSwapMinImageCount(
      vk::SurfaceCapabilitiesKHR const &surfaceCapabilities);
  Window *window;
  // GraphicsPipeline *graphicsPipeline;

  vk::raii::Context context;
  vk::raii::Instance instance = nullptr;
  vk::raii::PhysicalDevice physicalDevice = nullptr;
  vk::raii::Device logicalDevice = nullptr;
  vk::raii::Queue graphicsQueue = nullptr;
  uint32_t graphicsQueueFamilyIndex = 0;
  vk::raii::SurfaceKHR surface = nullptr;
  vk::raii::Queue presentQueue = nullptr;
  std::vector<vk::Image> swapChainImages;
  vk::SurfaceFormatKHR swapChainSurfaceFormat;
  vk::Extent2D swapChainExtent;
  std::vector<vk::raii::ImageView> swapChainImageViews;
  vk::raii::SwapchainKHR swapChain = nullptr;

  std::vector<const char *> getRequiredInstanceExtensions();
  const std::vector<char const *> validationLayers = {
      "VK_LAYER_KHRONOS_validation"};

  const std::vector<const char *> deviceExtensions = {
      vk::KHRSwapchainExtensionName};

  vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<vk::SurfaceFormatKHR> &availableFormats);

  vk::PresentModeKHR chooseSwapPresentMode(
      const std::vector<vk::PresentModeKHR> &availablePresentModes);
  vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);

  // Helper Functions
};
