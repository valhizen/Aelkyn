#include <string>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Window {
 public:
  Window();
  ~Window();

  // Healper
  bool shouldClose() { return glfwWindowShouldClose(window); }
  void pollEvents() { glfwPollEvents(); }

  GLFWwindow* window;

 private:
  // Window

  // Window Settings
  static constexpr int WIDTH = 800;
  static constexpr int HEIGHT = 800;
  std::string NAME = "Aelkyn";
};
