#pragma once

#include "../window.hpp"
#include <cstdint>
#include <vector>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

class Context {
public:
  void init(Window &window);
  void recreateSwapChain();
  void cleanupSwapChain();

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
  const vk::raii::PhysicalDevice &getPhysicalDevice() const {
    return physicalDevice;
  }

  uint32_t getSwapChainImageCount() const {
    return static_cast<uint32_t>(swapChainImages.size());
  }

  void copyBuffer(vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer,
                  vk::DeviceSize size);

private:
  void createInstance();
  void setupDebugMessenger();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createTransferCommandPool();
  void findQueueFamilies();
  void createSurface();
  void createSwapChain();
  void createImageViews();

  uint32_t chooseSwapMinImageCount(
      vk::SurfaceCapabilitiesKHR const &surfaceCapabilities);

  Window *window;

  vk::raii::Context context;
  vk::raii::Instance instance = nullptr;
  vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
  vk::raii::PhysicalDevice physicalDevice = nullptr;
  vk::raii::Device logicalDevice = nullptr;
  vk::raii::Queue graphicsQueue = nullptr;
  vk::raii::CommandPool transferCommandPool = nullptr;
  uint32_t graphicsQueueFamilyIndex = ~0u;
  vk::raii::SurfaceKHR surface = nullptr;
  std::vector<vk::Image> swapChainImages;
  vk::SurfaceFormatKHR swapChainSurfaceFormat;
  vk::Extent2D swapChainExtent;
  std::vector<vk::raii::ImageView> swapChainImageViews;
  vk::raii::SwapchainKHR swapChain = nullptr;

  std::vector<const char *> getRequiredInstanceExtensions();

#ifdef NDEBUG
  static constexpr bool enableValidationLayers = false;
#else
  static constexpr bool enableValidationLayers = true;
#endif

  const std::vector<char const *> validationLayers = {
      "VK_LAYER_KHRONOS_validation"};

  const std::vector<const char *> deviceExtensions = {
      vk::KHRSwapchainExtensionName};

  vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<vk::SurfaceFormatKHR> &availableFormats);

  vk::PresentModeKHR chooseSwapPresentMode(
      const std::vector<vk::PresentModeKHR> &availablePresentModes);
  vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);

  static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
      vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
      vk::DebugUtilsMessageTypeFlagsEXT type,
      const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *);
};
