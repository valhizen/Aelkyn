#include "aelkyn.hpp"

void Aelkyn::init() {
  renderer.init(window);
  while (!window.shouldClose()) {
    window.pollEvents();
    renderer.drawFrame();
  }
  renderer.waitIdle();
}
