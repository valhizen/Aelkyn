#pragma once

#include "device.hpp"
#include <string>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

class Texture {
public:
  void init(Device &device, const std::string &filepath);

  const vk::raii::ImageView &getImageView() const { return imageView; }
  const vk::raii::Sampler &getSampler() const { return sampler; }

private:
  void createTextureImage(const std::string &filepath);
  void createImageView();
  void createSampler();

  void transitionImageLayout(vk::ImageLayout oldLayout,
                             vk::ImageLayout newLayout);

  Device *device = nullptr;

  vk::raii::Image image = nullptr;
  vk::raii::DeviceMemory imageMemory = nullptr;
  vk::raii::ImageView imageView = nullptr;
  vk::raii::Sampler sampler = nullptr;
};
