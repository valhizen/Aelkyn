#include "Application.hpp"

#include <GLFW/glfw3.h>

void Application::run() {

  glfwSetKeyCallback(window.window, Input::keyCallback);
  glfwSetMouseButtonCallback(window.window, Input::mouseButtonCallback);

  while (!window.shouldClose()) {
    window.pollEvents();
    glfwSwapBuffers(window.window);
  }
}
