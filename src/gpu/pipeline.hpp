#pragma once

#include "../core/constants.hpp"
#include "buffer.hpp"
#include "context.hpp"
#include <vector>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

class GraphicsPipeline {
private:
  void createDescriptorSetLayout();
  void createGraphicsPipeline();

  void createDescriptorSets();
  void createDescriptorPool();

  [[nodiscard]] vk::raii::ShaderModule
  createShaderModule(const std::vector<char> &code) const;

  Context *device;
  Buffer *buffer;

  vk::raii::PipelineLayout pipelineLayout = nullptr;
  vk::raii::Pipeline graphicsPipeline = nullptr;
  vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
  vk::raii::DescriptorPool descriptorPool = nullptr;
  std::vector<vk::raii::DescriptorSet> descriptorSets;

public:
  void init(Context &device, Buffer &buffer);
  const vk::raii::Pipeline &getGraphicsPipeline() const {
    return graphicsPipeline;
  }
  const vk::raii::PipelineLayout &getPipelineLayout() const {
    return pipelineLayout;
  }
  const std::vector<vk::raii::DescriptorSet> &getDescriptorSets() const {
    return descriptorSets;
  }
};
