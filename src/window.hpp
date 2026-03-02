#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Window {

private:
  GLFWwindow *window;
  bool framebufferResized = false;
  static void framebufferResizeCallback(GLFWwindow *window, int width,
                                        int height) {
    auto self = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    self->framebufferResized = true;
  }

public:
  Window();
  ~Window();
  GLFWwindow *getWindow() const { return window; }
  bool shouldClose();
  void pollEvents();

  bool wasResized() const { return framebufferResized; }
  void resetResizedFlag() { framebufferResized = false; }
};
