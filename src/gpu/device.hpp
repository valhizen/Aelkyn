#pragma once

#include "../window.hpp"
#include <cstdint>
#include <vector>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

// Device owns the core Vulkan handles that live for the entire application
// lifetime: instance, physical device, logical device, queues, and surface

class Device {
public:
  void init(Window &window);

  const vk::raii::Device &logical() const { return logicalDevice; }
  const vk::raii::PhysicalDevice &physical() const { return physicalDevice; }
  const vk::raii::Queue &queue() const { return graphicsQueue; }
  const vk::raii::SurfaceKHR &surface() const { return surface_; }
  uint32_t queueFamily() const { return queueFamilyIndex; }

  uint32_t findMemoryType(uint32_t typeFilter,
                          vk::MemoryPropertyFlags properties) const;

  void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                    vk::MemoryPropertyFlags properties,
                    vk::raii::Buffer &buffer,
                    vk::raii::DeviceMemory &bufferMemory);

  void copyBuffer(vk::raii::Buffer &src, vk::raii::Buffer &dst,
                  vk::DeviceSize size);

  void createImage(uint32_t width, uint32_t height, vk::Format format,
                   vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                   vk::MemoryPropertyFlags properties, vk::raii::Image &image,
                   vk::raii::DeviceMemory &imageMemory);

  void copyBufferToImage(vk::raii::Buffer &buffer, vk::raii::Image &image,
                         uint32_t width, uint32_t height);

  [[nodiscard]] vk::raii::CommandBuffer beginSingleTimeCommands();

  void endSingleTimeCommands(vk::raii::CommandBuffer &cmd);

  bool hasStencilComponent(vk::Format format) {
    return format == vk::Format::eD32SfloatS8Uint ||
           format == vk::Format::eD24UnormS8Uint;
  }

  vk::Format findDepthFormat();
  vk::raii::ImageView createImageView(vk::raii::Image &image, vk::Format format,
                                      vk::ImageAspectFlags aspectFlags);

private:
  void createInstance();
  void setupDebugMessenger();
  void createSurface(Window &window);
  void pickPhysicalDevice();
  void findQueueFamilies();
  void createLogicalDevice();
  void createUtilityCommandPool();
  vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates,
                                 vk::ImageTiling tiling,
                                 vk::FormatFeatureFlags features);

  vk::raii::Context context;
  vk::raii::Instance instance = nullptr;
  vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
  vk::raii::SurfaceKHR surface_ = nullptr;
  vk::raii::PhysicalDevice physicalDevice = nullptr;
  vk::raii::Device logicalDevice = nullptr;
  vk::raii::Queue graphicsQueue = nullptr;
  vk::raii::CommandPool utilityCommandPool = nullptr;
  uint32_t queueFamilyIndex = ~0u;

  std::vector<const char *> getRequiredInstanceExtensions();

#ifdef NDEBUG
  static constexpr bool enableValidationLayers = false;
#else
  static constexpr bool enableValidationLayers = true;
#endif

  const std::vector<const char *> validationLayers = {
      "VK_LAYER_KHRONOS_validation"};
  const std::vector<const char *> deviceExtensions = {
      vk::KHRSwapchainExtensionName};

  static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
      vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
      vk::DebugUtilsMessageTypeFlagsEXT type,
      const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *);
};
