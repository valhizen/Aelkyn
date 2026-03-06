
#pragma once

#include "gpu/buffer.hpp"
#include "gpu/device.hpp"
#include "gpu/pipeline.hpp"
#include "gpu/renderer.hpp"
#include "gpu/swapchain.hpp"
#include "gpu/texture.hpp"
#include "window.hpp"

class Aelkyn {
public:
  void init();
  void run();

private:
  Window window;
  Device device;
  SwapChain swapChain;
  Buffer buffer;
  Texture texture;
  GraphicsPipeline pipeline;
  Renderer renderer;
};
