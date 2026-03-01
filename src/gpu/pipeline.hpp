#pragma once

#include "context.hpp"
#include <vector>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

class GraphicsPipeline {
private:
  void createGraphicsPipeline();
  [[nodiscard]] vk::raii::ShaderModule
  createShaderModule(const std::vector<char> &code) const;

  Context *device;

  vk::raii::PipelineLayout pipelineLayout = nullptr;
  vk::raii::Pipeline graphicsPipeline = nullptr;

public:
  void init(Context &device);
  const vk::raii::Pipeline &getGraphicsPipeline() const {
    return graphicsPipeline;
  }
};
