#include "aelkyn.hpp"

void Aelkyn::init() {

  device.init(window);
  swapChain.init(device, window);
  buffer.init(device);
  texture.init(device, "../textures/texture.jpg");
  pipeline.init(device, swapChain.surfaceFormat(), swapChain.depthFormat(),
                buffer, texture);
  renderer.init(device, swapChain, pipeline, buffer, window);
}

void Aelkyn::run() {
  while (!window.shouldClose()) {
    window.pollEvents();

    // TODO: game logic, physics, input and more

    renderer.drawFrame();
  }
  renderer.waitIdle();
}
