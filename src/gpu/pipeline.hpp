#pragma once

#include "../core/constants.hpp"
#include "../core/rendertypes.hpp"
#include "buffer.hpp"
#include "device.hpp"
#include "texture.hpp"
#include <vector>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

class GraphicsPipeline {
public:
  void init(Device &device, const vk::SurfaceFormatKHR &surfaceFormat,
            vk::Format depthFormat, Buffer &buffer, Texture &texture);

  const vk::raii::Pipeline &getPipeline() const { return pipeline_; }
  const vk::raii::PipelineLayout &getLayout() const { return pipelineLayout; }
  const std::vector<vk::raii::DescriptorSet> &getDescriptorSets() const {
    return descriptorSets;
  }

private:
  void createDescriptorSetLayout();
  void createPipeline();
  void createDescriptorPool();
  void createDescriptorSets();

  [[nodiscard]] vk::raii::ShaderModule
  createShaderModule(const std::vector<char> &code) const;

  Device *device = nullptr;
  Buffer *buffer = nullptr;
  Texture *texture = nullptr;
  vk::SurfaceFormatKHR surfaceFormat;
  vk::Format depthFormat_;

  vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
  vk::raii::PipelineLayout pipelineLayout = nullptr;
  vk::raii::Pipeline pipeline_ = nullptr;
  vk::raii::DescriptorPool descriptorPool = nullptr;
  std::vector<vk::raii::DescriptorSet> descriptorSets;
};
