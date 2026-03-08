#include "window.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>

Window::Window(const WindowConfig &config)
    : currentWidht(config.width), currentHeight(config.height) {
  if (!glfwInit()) {
    throw std::runtime_error("Failed to Initialize GLFW");
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(currentWidht, currentHeight, config.name.c_str(),
                            nullptr, nullptr);

  if (!window) {
    glfwTerminate();
    throw std::runtime_error("Dailed to Initialize Windoe");
  }
  std::cout << "Window Created with Configuration : " << currentWidht
            << currentHeight;
}

Window::~Window() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

bool Window::shouldClose() { return glfwWindowShouldClose(window); }
void Window::pollEvents() { return glfwPollEvents(); }
