#include "window.hpp"
#include <GLFW/glfw3.h>

void Window::init() {
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(800, 900, "Aelkyn", nullptr, nullptr);
}
