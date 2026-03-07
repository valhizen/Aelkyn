#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Device {
public:
  Device();

private:
  void createInstance();
  void pickPhysicalDevice();
  void createLogicalDevice();

  std::vector<const char *> getRequiredInstanceExtensions();

#ifdef NDEBUG
  static constexpr bool enableValidationLayers = false;
#else
  static constexpr bool enableValidationLayers = true;
#endif

  vk::raii::Context context;
  vk::raii::Instance instance = nullptr;
  vk::raii::PhysicalDevice physicalDevice = nullptr;
  vk::raii::Device logicalDevice = nullptr;

  const std::vector<const char *> validationLayers = {
      "VK_LAYER_KHRONOS_validation"};
  const std::vector<const char *> deviceExtensions = {
      vk::KHRSwapchainExtensionName};
};
