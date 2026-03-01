
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
class Window {

private:
  GLFWwindow *window;

public:
  Window();
  ~Window();
  GLFWwindow *getWindow() const { return window; }
  bool shouldClose();
  void pollEvents();
};
