#include "context.hpp"
#include <cstdint>
#include <iostream>
#include <map>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>

void Context::init(Window &window) {
  this->window = &window;

  createInstance();
  createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  createSwapChain();
  createImageViews();
}

void Context::createInstance() {
  constexpr vk::ApplicationInfo appInfo{
      .pApplicationName = "Aelkyn",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "No Engie",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = vk::ApiVersion13,
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
  // if (enableValidationLayer) {
  //   extensions.push_back(vk::EXTDebugUtilsExtensionName);
  // }
  return extensions;
}

void Context::pickPhysicalDevice() {
  auto devices = vk::raii::PhysicalDevices(instance);

  if (devices.empty()) {
    throw std::runtime_error("failed to fing GPU with vulkan Support");
  }
  std::multimap<int, vk::raii::PhysicalDevice> canidates;

  // auto props3 = devices[2].getProperties();
  // std::cout << props3.deviceName;

  for (const auto &device : devices) {
    auto deviceProperties = device.getProperties();
    auto deviceFeatures = device.getFeatures();
    uint32_t score = 0;

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
      score += 1000;
    }
    score += deviceProperties.limits.maxImageDimension2D;

    // if (!deviceFeatures.geometryShader) {
    //   continue;
    // }
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
          // {.dynamicRendering = true}, // vk::PhysicalDeviceVulkan13Features
          {.synchronization2 = true, .dynamicRendering = true},
          {.extendedDynamicState =
               true} // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
      };

  float queuePriority = 0.5f;
  vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
      .queueFamilyIndex = graphicsQueueFamilyIndex,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority};

  vk::DeviceCreateInfo deviceCreateInfo{
      .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &deviceQueueCreateInfo,
      .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
      .ppEnabledExtensionNames = deviceExtensions.data()};

  logicalDevice = vk::raii::Device(physicalDevice, deviceCreateInfo);
  graphicsQueue = vk::raii::Queue(logicalDevice, graphicsQueueFamilyIndex, 0);
}

void Context::findQueueFamilies(const vk::raii::SurfaceKHR surface) {
  // find the index of the first queue family that supports graphics
  std::vector<vk::QueueFamilyProperties> queueFamilyProperties =
      physicalDevice.getQueueFamilyProperties();

  // get the first index into queueFamilyProperties which supports graphics
  auto graphicsQueueFamilyProperty =
      std::ranges::find_if(queueFamilyProperties, [](auto const &qfp) {
        return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) !=
               static_cast<vk::QueueFlags>(0);
      });

  auto graphicsIndex = static_cast<uint32_t>(std::distance(
      queueFamilyProperties.begin(), graphicsQueueFamilyProperty));

  // determine a queueFamilyIndex that supports present
  // first check if the graphicsIndex is good enough
  auto presentIndex =
      physicalDevice.getSurfaceSupportKHR(graphicsIndex, *surface)
          ? graphicsIndex
          : static_cast<uint32_t>(queueFamilyProperties.size());
  if (presentIndex == queueFamilyProperties.size()) {
    // the graphicsIndex doesn't support present -> look for another family
    // index that supports both graphics and present
    for (size_t i = 0; i < queueFamilyProperties.size(); i++) {
      if ((queueFamilyProperties[i].queueFlags &
           vk::QueueFlagBits::eGraphics) &&
          physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i),
                                              *surface)) {
        graphicsIndex = static_cast<uint32_t>(i);
        presentIndex = graphicsIndex;
        break;
      }
    }
    if (presentIndex == queueFamilyProperties.size()) {
      // there's nothing like a single family index that supports both graphics
      // and present -> look for another family index that supports present
      for (size_t i = 0; i < queueFamilyProperties.size(); i++) {
        if (physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i),
                                                *surface)) {
          presentIndex = static_cast<uint32_t>(i);
          break;
        }
      }
    }
  }
  if ((graphicsIndex == queueFamilyProperties.size()) ||
      (presentIndex == queueFamilyProperties.size())) {
    throw std::runtime_error(
        "Could not find a queue for graphics or present -> terminating");
  }
}

void Context::createSurface() {

  VkSurfaceKHR _surface;
  if (glfwCreateWindowSurface(*instance, window->getWindow(), nullptr,

                              &_surface) != 0) {
    throw std::runtime_error("failed to create window surface!");
  }
  surface = vk::raii::SurfaceKHR(instance, _surface);
}

vk::SurfaceFormatKHR Context::chooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
        availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

vk::PresentModeKHR Context::chooseSwapPresentMode(
    const std::vector<vk::PresentModeKHR> &availablePresentModes) {
  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
      return availablePresentMode;
    }
  }
  return vk::PresentModeKHR::eFifo;
}

vk::Extent2D
Context::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) {
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

uint32_t Context::chooseSwapMinImageCount(
    vk::SurfaceCapabilitiesKHR const &surfaceCapabilities) {
  auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
  if ((0 < surfaceCapabilities.maxImageCount) &&
      (surfaceCapabilities.maxImageCount < minImageCount)) {
    minImageCount = surfaceCapabilities.maxImageCount;
  }
  return minImageCount;
}

void Context::createSwapChain() {

  auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
  swapChainExtent = chooseSwapExtent(surfaceCapabilities);
  swapChainSurfaceFormat =
      chooseSwapSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(*surface));
  vk::SwapchainCreateInfoKHR swapChainCreateInfo{
      .surface = *surface,
      .minImageCount = chooseSwapMinImageCount(surfaceCapabilities),
      .imageFormat = swapChainSurfaceFormat.format,
      .imageColorSpace = swapChainSurfaceFormat.colorSpace,
      .imageExtent = swapChainExtent,
      .imageArrayLayers = 1,
      .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
      .imageSharingMode = vk::SharingMode::eExclusive,
      .preTransform = surfaceCapabilities.currentTransform,
      .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
      .presentMode = chooseSwapPresentMode(
          physicalDevice.getSurfacePresentModesKHR(*surface)),
      .clipped = true};

  swapChain = vk::raii::SwapchainKHR(logicalDevice, swapChainCreateInfo);
  swapChainImages = swapChain.getImages();
}

void Context::createImageViews() {

  assert(swapChainImageViews.empty());

  vk::ImageViewCreateInfo imageViewCreateInfo{
      .viewType = vk::ImageViewType::e2D,
      .format = swapChainSurfaceFormat.format,
      .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
  for (auto &image : swapChainImages) {
    imageViewCreateInfo.image = image;
    swapChainImageViews.emplace_back(logicalDevice, imageViewCreateInfo);
  }
}

void Context::recreateSwapChain() {

  int width = 0, height = 0;
  glfwGetFramebufferSize(window->getWindow(), &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(window->getWindow(), &width, &height);
    glfwWaitEvents();
  }

  logicalDevice.waitIdle();

  cleanupSwapChain();

  createSwapChain();
  createImageViews();
}

void Context::cleanupSwapChain() {
  swapChainImageViews.clear();
  swapChain = nullptr;
}
