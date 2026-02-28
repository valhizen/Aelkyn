#include "context.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <iostream>
#include <map>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

void Context::init() {
  createInstance();
  pickPhysicalDevice();
}

void Context::createInstance() {
  constexpr vk::ApplicationInfo appInfo{
      .pApplicationName = "Aelkyn",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "No Engie",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = vk::ApiVersion14,
  };
  auto requiredExtensions = getRequiredInstanceExtensions();
  vk::InstanceCreateInfo createInfo = {
      .pApplicationInfo = &appInfo,
      .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
      .ppEnabledExtensionNames = requiredExtensions.data()};
  instance = vk::raii::Instance(context, createInfo);
};

std::vector<const char *> Context::getRequiredInstanceExtensions() {
  uint32_t glfwExtensionCount{0};
  auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
  if (enableValidationLayer) {
    extensions.push_back(vk::EXTDebugUtilsExtensionName);
  }
  return extensions;
}

void Context::pickPhysicalDevice() {
  auto devices = vk::raii::PhysicalDevices(instance);

  if (devices.empty()) {
    throw std::runtime_error("failed to fing GPU with vulkan Support");
  }
  std::multimap<int, vk::raii::PhysicalDevice> canidates;

  for (const auto &device : devices) {
    auto deviceProperties = device.getProperties();
    auto deviceFeatures = device.getFeatures();
    uint32_t score = 0;

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
      score += 1000;
    }
    score += deviceProperties.limits.maxImageDimension2D;

    if (!deviceFeatures.geometryShader) {
      continue;
    }
    canidates.insert(std::make_pair(score, device));
  }
  // Check if the best candidate is suitable at all
  if (canidates.rbegin()->first > 0) {
    physicalDevice = canidates.rbegin()->second;
  } else {
    throw std::runtime_error("failed to find a suitable GPU!");
  }

  // Get The device that we are using
  auto props2 = physicalDevice.getProperties().deviceName;
  std::cout << props2 << std::endl;
}

void Context::createLogicalDevice() {
  vk::StructureChain<vk::PhysicalDeviceFeatures2,
                     vk::PhysicalDeviceVulkan11Features,
                     vk::PhysicalDeviceVulkan13Features,
                     vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
      featureChain = {
          {}, // vk::PhysicalDeviceFeatures2
          {.shaderDrawParameters = true},
          {.dynamicRendering = true}, // vk::PhysicalDeviceVulkan13Features
          {.extendedDynamicState =
               true} // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
      };

  float queuePriority = 0.5f;
  vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
      .queueFamilyIndex = graphicsQueueFamilyIndex,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority};

  vk::DeviceCreateInfo deviceCreateInfo{
      .pNext = &featureChain,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &deviceQueueCreateInfo,
      .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
      .ppEnabledExtensionNames = deviceExtensions.data()};

  logicalDevice = vk::raii::Device(physicalDevice, deviceCreateInfo);
  graphicsQueue = vk::raii::Queue(logicalDevice, graphicsQueueFamilyIndex, 0);
}
