#include "aelkyn.hpp"

void Aelkyn::init() {
  model.load("../models/viking_room.obj");
  device.init(window);
  swapChain.init(device, window);
  buffer.init(device, model.getVertices(), model.getIndices());
  texture.init(device, "../models/viking_room.png");
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
