#include "application.hpp"
#include <GLFW/glfw3.h>

void Application::run() {
  vulkan.initVulkan();
  while (!window.shouldClose()) {
    window.pollEvents();
  }
}
