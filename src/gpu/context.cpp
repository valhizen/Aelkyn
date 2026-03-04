#include "context.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

void Context::init(Window &window) {
  this->window = &window;

  createInstance();
  setupDebugMessenger();
  createSurface();
  pickPhysicalDevice();
  findQueueFamilies();
  createLogicalDevice();
  createTransferCommandPool();
  createSwapChain();
  createImageViews();
}

void Context::createInstance() {
  constexpr vk::ApplicationInfo appInfo{
      .pApplicationName = "Aelkyn",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "No Engine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = vk::ApiVersion13,
  };

  // Get required layers
  std::vector<char const *> requiredLayers;
  if (enableValidationLayers) {
    requiredLayers.assign(validationLayers.begin(), validationLayers.end());

    // Check layer support
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

void Context::setupDebugMessenger() {
  if (!enableValidationLayers)
    return;

  vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
  vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
      vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
      vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
      vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
  vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
      .messageSeverity = severityFlags,
      .messageType = messageTypeFlags,
      .pfnUserCallback = &debugCallback};
  debugMessenger =
      instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL Context::debugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT type,
    const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *) {
  if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError ||
      severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
    std::cerr << "validation layer: type " << to_string(type)
              << " msg: " << pCallbackData->pMessage << std::endl;
  }
  return vk::False;
}

std::vector<const char *> Context::getRequiredInstanceExtensions() {
  uint32_t glfwExtensionCount{0};
  auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
  if (enableValidationLayers) {
    extensions.push_back(vk::EXTDebugUtilsExtensionName);
  }
  return extensions;
}

void Context::pickPhysicalDevice() {
  auto devices = vk::raii::PhysicalDevices(instance);

  if (devices.empty()) {
    throw std::runtime_error("failed to find GPU with Vulkan support");
  }

  // Score devices, preferring discrete GPUs
  std::multimap<int, const vk::raii::PhysicalDevice *> candidates;

  for (const auto &device : devices) {
    auto props = device.getProperties();

    // Check minimum requirements
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
                                                     requiredExt) == 0;
                                     });
        }))
      continue;

    auto features = device.template getFeatures2<
        vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features,
        vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    if (!features.template get<vk::PhysicalDeviceVulkan11Features>()
             .shaderDrawParameters ||
        !features.template get<vk::PhysicalDeviceVulkan13Features>()
             .dynamicRendering ||
        !features
             .template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>()
             .extendedDynamicState)
      continue;

    // Score: discrete GPUs get a big bonus
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

  auto props = physicalDevice.getProperties().deviceName;
  std::cout << "Selected GPU: " << props << std::endl;
}

void Context::findQueueFamilies() {
  std::vector<vk::QueueFamilyProperties> queueFamilyProperties =
      physicalDevice.getQueueFamilyProperties();

  for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
    if ((queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
        physicalDevice.getSurfaceSupportKHR(i, *surface)) {
      graphicsQueueFamilyIndex = i;
      return;
    }
  }

  if (graphicsQueueFamilyIndex == ~0u) {
    throw std::runtime_error(
        "Could not find a queue for graphics and present -> terminating");
  }
}

void Context::createLogicalDevice() {
  vk::StructureChain<vk::PhysicalDeviceFeatures2,
                     vk::PhysicalDeviceVulkan11Features,
                     vk::PhysicalDeviceVulkan13Features,
                     vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
      featureChain = {{},
                      {.shaderDrawParameters = true},
                      {.synchronization2 = true, .dynamicRendering = true},
                      {.extendedDynamicState = true}};

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

void Context::createTransferCommandPool() {
  vk::CommandPoolCreateInfo poolInfo{
      .flags = vk::CommandPoolCreateFlagBits::eTransient,
      .queueFamilyIndex = graphicsQueueFamilyIndex};
  transferCommandPool = vk::raii::CommandPool(logicalDevice, poolInfo);
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

void Context::copyBuffer(vk::raii::Buffer &srcBuffer,
                         vk::raii::Buffer &dstBuffer, vk::DeviceSize size) {
  vk::CommandBufferAllocateInfo allocInfo{
    .commandPool = transferCommandPool,
    .level = vk::CommandBufferLevel::ePrimary,
    .commandBufferCount = 1};
  vk::raii::CommandBuffer commandCopyBuffer =
      std::move(logicalDevice.allocateCommandBuffers(allocInfo).front());
  commandCopyBuffer.begin(vk::CommandBufferBeginInfo{
      .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
  commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer,
                               vk::BufferCopy(0, 0, size));
  commandCopyBuffer.end();
  graphicsQueue.submit(vk::SubmitInfo{.commandBufferCount = 1,
                                      .pCommandBuffers = &*commandCopyBuffer},
                       nullptr);
  logicalDevice.waitIdle();
}

void Context::cleanupSwapChain() {
  swapChainImageViews.clear();
  swapChain = nullptr;
}
