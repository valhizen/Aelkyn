#pragma once

#include <sys/types.h>
#include <vector>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <GLFW/glfw3.h>

class Context {
public:
  void init();
  bool enableValidationLayer = true;

private:
  void createInstance();
  void pickPhysicalDevice();
  void createLogicalDevice();

  vk::raii::Context context;
  vk::raii::Instance instance = nullptr;
  vk::raii::PhysicalDevice physicalDevice = nullptr;
  vk::raii::Device logicalDevice = nullptr;
  vk::raii::Queue graphicsQueue = nullptr;
  uint32_t graphicsQueueFamilyIndex = 0;

  std::vector<const char *> getRequiredInstanceExtensions();
  const std::vector<char const *> validationLayers = {
      "VK_LAYER_KHRONOS_validation"};

  const std::vector<const char *> deviceExtensions = {
      vk::KHRSwapchainExtensionName};
};
