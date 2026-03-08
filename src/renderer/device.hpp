#pragma once
#include <vector>
#include <vulkan/vulkan.h>

class Device {
public:
  void init();
  void cleanup();

private:
  void createInstance();
  void pickPhysicalDevice();
  void createLogicalDevice();
  std::vector<const char *> getRequiredGlfwExtensions();

  VkInstance instance = VK_NULL_HANDLE;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice device = VK_NULL_HANDLE;

#ifdef NDEBUG
  static constexpr bool enableValidationLayers = false;
#else
  static constexpr bool enableValidationLayers = true;
#endif

  const std::vector<const char *> validationLayers = {
      "VK_LAYER_KHRONOS_validation"};
};
