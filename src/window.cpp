#include "window.hpp"
#include <GLFW/glfw3.h>

Window::Window() {

  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(800, 900, "Aelkyn", nullptr, nullptr);
}

Window::~Window() {
  glfwDestroyWindow(window);
  glfwTerminate();
}
bool Window::shouldClose() { return glfwWindowShouldClose(window); }
void Window::pollEvents() { glfwPollEvents(); }
