#include "renderer.hpp"
#include "context.hpp"
#include <GLFW/glfw3.h>

void Renderer::run() {
  window.init();

  context.init();
  while (!glfwWindowShouldClose(window.getWindow())) {
    glfwPollEvents();
  }
}
