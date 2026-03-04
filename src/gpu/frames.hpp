#pragma once

#include "buffer.hpp"
#include "commands.hpp"
#include "context.hpp"
#include <vector>
#include <vulkan/vulkan_raii.hpp>

class Frames {

private:
  std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
  std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
  std::vector<vk::raii::Fence> inFlightFences;

  uint32_t currentFrame = 0;
  Context *device;
  Commands *commands;
  Window *window;
  Buffer *buffer;

  void createSyncObjects();

public:
  void init(Context &device, Commands &commands, Window &window,
            Buffer &buffer);
  void drawFrame();
};
