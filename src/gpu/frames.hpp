#pragma once

#include "commands.hpp"
#include "context.hpp"
#include <vulkan/vulkan_raii.hpp>

class Frames {

private:
  vk::raii::Semaphore presentCompleteSemaphore = nullptr;
  vk::raii::Semaphore renderFinishedSemaphore = nullptr;
  vk::raii::Fence drawFence = nullptr;

  Context *device;
  Commands *commands;

  void createSyncObjects();

public:
  void init(Context &device, Commands &commands);
  void drawFrame();
};
