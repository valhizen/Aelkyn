#include "Window.hpp"

#include <stdexcept>

Window::Window() {
  // NOTE: This is to allow RenderDoc, which without this doesn't work
  // glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);

  glfwInit();
  //  TEMP: Just to Check Window so nothing to do with vulkan right now
  // glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  // Resiziable will be handeled by vulkan so no need here
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(WIDTH, HEIGHT, NAME.c_str(), nullptr, nullptr);
  if (!window) {
    throw std::runtime_error("Failed to create window");
    return;
  }
  glfwMakeContextCurrent(window);
}

Window::~Window() {
  glfwDestroyWindow(window);
  glfwTerminate();
}
