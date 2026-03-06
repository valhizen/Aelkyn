#include "texture.hpp"
#include <cstring>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void Texture::init(Device &device, const std::string &filepath) {
  this->device = &device;
  createTextureImage(filepath);
  createImageView();
  createSampler();
}

// Image loading and upload

void Texture::createTextureImage(const std::string &filepath) {
  int texWidth, texHeight, texChannels;
  stbi_uc *pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight,
                              &texChannels, STBI_rgb_alpha);
  if (!pixels) {
    throw std::runtime_error("failed to load texture image: " + filepath);
  }

  vk::DeviceSize imageSize =
      static_cast<vk::DeviceSize>(texWidth) * texHeight * 4;

  // Upload pixels through a staging buffer
  vk::raii::Buffer stagingBuffer({});
  vk::raii::DeviceMemory stagingMemory({});
  device->createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc,
                       vk::MemoryPropertyFlagBits::eHostVisible |
                           vk::MemoryPropertyFlagBits::eHostCoherent,
                       stagingBuffer, stagingMemory);

  void *data = stagingMemory.mapMemory(0, imageSize);
  memcpy(data, pixels, imageSize);
  stagingMemory.unmapMemory();

  stbi_image_free(pixels);

  // Create the device-local image
  device->createImage(
      static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight),
      vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
      vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
      vk::MemoryPropertyFlagBits::eDeviceLocal, image, imageMemory);

  // Transition → transfer dst, copy, transition → shader read
  transitionImageLayout(vk::ImageLayout::eUndefined,
                        vk::ImageLayout::eTransferDstOptimal);
  device->copyBufferToImage(stagingBuffer, image,
                            static_cast<uint32_t>(texWidth),
                            static_cast<uint32_t>(texHeight));
  transitionImageLayout(vk::ImageLayout::eTransferDstOptimal,
                        vk::ImageLayout::eShaderReadOnlyOptimal);
}

// Layout transitions

void Texture::transitionImageLayout(vk::ImageLayout oldLayout,
                                    vk::ImageLayout newLayout) {
  auto cmd = device->beginSingleTimeCommands();

  vk::ImageMemoryBarrier barrier{
      .oldLayout = oldLayout,
      .newLayout = newLayout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image,
      .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};

  vk::PipelineStageFlags srcStage;
  vk::PipelineStageFlags dstStage;

  if (oldLayout == vk::ImageLayout::eUndefined &&
      newLayout == vk::ImageLayout::eTransferDstOptimal) {
    barrier.srcAccessMask = {};
    barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
    srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
    dstStage = vk::PipelineStageFlagBits::eTransfer;
  } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
             newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    srcStage = vk::PipelineStageFlagBits::eTransfer;
    dstStage = vk::PipelineStageFlagBits::eFragmentShader;
  } else {
    throw std::invalid_argument("unsupported layout transition!");
  }

  cmd.pipelineBarrier(srcStage, dstStage, {}, {}, nullptr, barrier);
  device->endSingleTimeCommands(cmd);
}

// Image view

void Texture::createImageView() {
  vk::ImageViewCreateInfo viewInfo{
      .image = image,
      .viewType = vk::ImageViewType::e2D,
      .format = vk::Format::eR8G8B8A8Srgb,
      .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
  imageView = vk::raii::ImageView(device->logical(), viewInfo);
}

// Sampler

void Texture::createSampler() {
  auto properties = device->physical().getProperties();

  vk::SamplerCreateInfo samplerInfo{
      .magFilter = vk::Filter::eLinear,
      .minFilter = vk::Filter::eLinear,
      .mipmapMode = vk::SamplerMipmapMode::eLinear,
      .addressModeU = vk::SamplerAddressMode::eRepeat,
      .addressModeV = vk::SamplerAddressMode::eRepeat,
      .addressModeW = vk::SamplerAddressMode::eRepeat,
      .mipLodBias = 0.0f,
      .anisotropyEnable = vk::True,
      .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
      .compareEnable = vk::False,
      .compareOp = vk::CompareOp::eAlways,
      .minLod = 0.0f,
      .maxLod = 0.0f,
      .borderColor = vk::BorderColor::eIntOpaqueBlack,
      .unnormalizedCoordinates = vk::False};
  sampler = vk::raii::Sampler(device->logical(), samplerInfo);
}
