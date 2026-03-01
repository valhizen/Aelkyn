#include "window.hpp"
#include <GLFW/glfw3.h>

Window::Window() {
  glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(1000, 1000, "Aelkyn", nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

Window::~Window() {
  glfwDestroyWindow(window);
  glfwTerminate();
}
bool Window::shouldClose() { return glfwWindowShouldClose(window); }
void Window::pollEvents() { glfwPollEvents(); }
