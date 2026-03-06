#include "device.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <map>
#include <stdexcept>
#include <vector>

void Device::init(Window &window) {
  createInstance();
  setupDebugMessenger();
  createSurface(window);
  pickPhysicalDevice();
  findQueueFamilies();
  createLogicalDevice();
  createUtilityCommandPool();
}

// Instance

void Device::createInstance() {
  constexpr vk::ApplicationInfo appInfo{
      .pApplicationName = "Aelkyn",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "No Engine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = vk::ApiVersion13,
  };

  std::vector<char const *> requiredLayers;
  if (enableValidationLayers) {
    requiredLayers.assign(validationLayers.begin(), validationLayers.end());

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

// Debug

void Device::setupDebugMessenger() {
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

VKAPI_ATTR vk::Bool32 VKAPI_CALL Device::debugCallback(
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

// Surface

void Device::createSurface(Window &window) {
  VkSurfaceKHR _surface;
  if (glfwCreateWindowSurface(*instance, window.getWindow(), nullptr,
                              &_surface) != 0) {
    throw std::runtime_error("failed to create window surface!");
  }
  surface_ = vk::raii::SurfaceKHR(instance, _surface);
}

// Physical device

void Device::pickPhysicalDevice() {
  auto devices = vk::raii::PhysicalDevices(instance);
  if (devices.empty()) {
    throw std::runtime_error("failed to find GPU with Vulkan support");
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
                                                     requiredExt) == 0;
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

// Queue families

void Device::findQueueFamilies() {
  auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

  for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
    if ((queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
        physicalDevice.getSurfaceSupportKHR(i, *surface_)) {
      queueFamilyIndex = i;
      return;
    }
  }

  throw std::runtime_error(
      "Could not find a queue for graphics and present -> terminating");
}

// Logical device

void Device::createLogicalDevice() {
  vk::StructureChain<vk::PhysicalDeviceFeatures2,
                     vk::PhysicalDeviceVulkan13Features,
                     vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
      featureChain = {{.features = {.samplerAnisotropy = true}},
                      {.synchronization2 = true, .dynamicRendering = true},
                      {.extendedDynamicState = true}};

  float queuePriority = 0.5f;
  vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
      .queueFamilyIndex = queueFamilyIndex,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority};

  vk::DeviceCreateInfo deviceCreateInfo{
      .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &deviceQueueCreateInfo,
      .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
      .ppEnabledExtensionNames = deviceExtensions.data()};

  logicalDevice = vk::raii::Device(physicalDevice, deviceCreateInfo);
  graphicsQueue = vk::raii::Queue(logicalDevice, queueFamilyIndex, 0);
}

// Utility command pool (for one-shot transfers, image transitions, etc.)

void Device::createUtilityCommandPool() {
  vk::CommandPoolCreateInfo poolInfo{
      .flags = vk::CommandPoolCreateFlagBits::eTransient,
      .queueFamilyIndex = queueFamilyIndex};
  utilityCommandPool = vk::raii::CommandPool(logicalDevice, poolInfo);
}

// Extensions

std::vector<const char *> Device::getRequiredInstanceExtensions() {
  uint32_t glfwExtensionCount{0};
  auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
  if (enableValidationLayers) {
    extensions.push_back(vk::EXTDebugUtilsExtensionName);
  }
  return extensions;
}

// Shared helpers

uint32_t Device::findMemoryType(uint32_t typeFilter,
                                vk::MemoryPropertyFlags properties) const {
  auto memProperties = physicalDevice.getMemoryProperties();

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags &
                                    properties) == properties) {
      return i;
    }
  }
  throw std::runtime_error("failed to find suitable memory type!");
}

void Device::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                          vk::MemoryPropertyFlags properties,
                          vk::raii::Buffer &buffer,
                          vk::raii::DeviceMemory &bufferMemory) {
  vk::BufferCreateInfo bufferInfo{
      .size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive};

  buffer = vk::raii::Buffer(logicalDevice, bufferInfo);
  vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
  vk::MemoryAllocateInfo allocInfo{
      .allocationSize = memRequirements.size,
      .memoryTypeIndex =
          findMemoryType(memRequirements.memoryTypeBits, properties)};
  bufferMemory = vk::raii::DeviceMemory(logicalDevice, allocInfo);
  buffer.bindMemory(*bufferMemory, 0);
}

void Device::copyBuffer(vk::raii::Buffer &srcBuffer,
                        vk::raii::Buffer &dstBuffer, vk::DeviceSize size) {
  auto cmd = beginSingleTimeCommands();
  cmd.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy(0, 0, size));
  endSingleTimeCommands(cmd);
}

void Device::createImage(uint32_t width, uint32_t height, vk::Format format,
                         vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                         vk::MemoryPropertyFlags properties,
                         vk::raii::Image &image,
                         vk::raii::DeviceMemory &imageMemory) {
  vk::ImageCreateInfo imageInfo{.imageType = vk::ImageType::e2D,
                                .format = format,
                                .extent = {width, height, 1},
                                .mipLevels = 1,
                                .arrayLayers = 1,
                                .samples = vk::SampleCountFlagBits::e1,
                                .tiling = tiling,
                                .usage = usage,
                                .sharingMode = vk::SharingMode::eExclusive};

  image = vk::raii::Image(logicalDevice, imageInfo);

  vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
  vk::MemoryAllocateInfo allocInfo{
      .allocationSize = memRequirements.size,
      .memoryTypeIndex =
          findMemoryType(memRequirements.memoryTypeBits, properties)};
  imageMemory = vk::raii::DeviceMemory(logicalDevice, allocInfo);
  image.bindMemory(imageMemory, 0);
}

void Device::copyBufferToImage(vk::raii::Buffer &buffer, vk::raii::Image &image,
                               uint32_t width, uint32_t height) {
  auto cmd = beginSingleTimeCommands();
  vk::BufferImageCopy region{
      .bufferOffset = 0,
      .bufferRowLength = 0,
      .bufferImageHeight = 0,
      .imageSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1},
      .imageOffset = {0, 0, 0},
      .imageExtent = {width, height, 1}};
  cmd.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal,
                        {region});
  endSingleTimeCommands(cmd);
}

vk::raii::CommandBuffer Device::beginSingleTimeCommands() {
  vk::CommandBufferAllocateInfo allocInfo{.commandPool = utilityCommandPool,
                                          .level =
                                              vk::CommandBufferLevel::ePrimary,
                                          .commandBufferCount = 1};
  vk::raii::CommandBuffer cmd =
      std::move(logicalDevice.allocateCommandBuffers(allocInfo).front());
  cmd.begin(vk::CommandBufferBeginInfo{
      .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
  return cmd;
}

void Device::endSingleTimeCommands(vk::raii::CommandBuffer &cmd) {
  cmd.end();
  graphicsQueue.submit(
      vk::SubmitInfo{.commandBufferCount = 1, .pCommandBuffers = &*cmd},
      nullptr);
  logicalDevice.waitIdle();
}
