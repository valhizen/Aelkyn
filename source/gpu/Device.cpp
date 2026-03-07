#include "Device.hpp"
#include "vulkan/vulkan.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <map>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>

// Instance Need to be Created Once For the Application

Device::Device() {
  createInstance();
  pickPhysicalDevice();
}

void Device::createInstance() {
  vk::ApplicationInfo appInfo{.pApplicationName = "Aelkyn",
                              .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                              .pEngineName = "No Engine",
                              .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                              .apiVersion = vk::ApiVersion13};

  std::vector<char const *> requiredLayers;

  if (enableValidationLayers) {
    requiredLayers.assign(validationLayers.begin(), validationLayers.end());
  }

  auto layerProperties = context.enumerateInstanceLayerProperties();

  auto unsupportedLayerIt = std::ranges::find_if(
      requiredLayers, [&layerProperties](auto const &requiredLayer) {
        return std::ranges::none_of(
            layerProperties, [requiredLayer](auto const &layerProperty) {
              return strcmp(layerProperty.layerName, requiredLayer) == 0;
            });
      });
  if (unsupportedLayerIt != requiredLayers.end()) {
    throw std::runtime_error("Required layer not supported: " +
                             std::string(*unsupportedLayerIt));
  }

  auto requiredExtensions = getRequiredInstanceExtensions();

  vk::InstanceCreateInfo createInfo = {
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
      .ppEnabledLayerNames = requiredLayers.data(),
      .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
      .ppEnabledExtensionNames = requiredExtensions.data()};

  instance = vk::raii::Instance(context, createInfo);
}

std::vector<const char *> Device::getRequiredInstanceExtensions() {
  uint32_t glfwExtensionCount{0};
  auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
  if (enableValidationLayers) {
    extensions.push_back(vk::EXTDebugUtilsExtensionName);
  }

  return extensions;
}

void Device::pickPhysicalDevice() {
  // Get List of All Devices
  auto devices = vk::raii::PhysicalDevices(instance);
  if (devices.empty()) {
    throw std::runtime_error("failed to find GPU with Vulkan Support");
  }

  std::multimap<int, const vk::raii::PhysicalDevice *> candidates;

  for (const auto &device : devices) {
    auto props = device.getProperties();

    if (props.apiVersion < VK_API_VERSION_1_3)
      continue;

    auto queueFamilies = device.getQueueFamilyProperties();
    if (!std::ranges::any_of(queueFamilies, [](auto const &qfp) {
          return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics);
        }))
      continue;

    auto availableDeviceExtensions =
        device.enumerateDeviceExtensionProperties();
    if (!std::ranges::all_of(deviceExtensions, [&availableDeviceExtensions](
                                                   auto const &requiredExt) {
          return std::ranges::any_of(availableDeviceExtensions,
                                     [requiredExt](auto const &availableExt) {
                                       return strcmp(availableExt.extensionName,
                                                     requiredExt)

                                              == 0;
                                     });
        }))
      continue;

    auto features = device.template getFeatures2<
        vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    if (!features.template get<vk::PhysicalDeviceFeatures2>()
             .features.samplerAnisotropy ||
        !features.template get<vk::PhysicalDeviceVulkan13Features>()
             .dynamicRendering ||
        !features
             .template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>()
             .extendedDynamicState)
      continue;

    uint32_t score = 0;
    if (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
      score += 1000;
    }
    score += props.limits.maxImageDimension2D;
    candidates.insert(std::make_pair(score, &device));
  }

  if (candidates.empty() || candidates.rbegin()->first == 0) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }

  physicalDevice = *candidates.rbegin()->second;
  std::cout << "Selected GPU: " << physicalDevice.getProperties().deviceName
            << std::endl;
}

void Device::createLogicalDevice() {}
