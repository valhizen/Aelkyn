#include <string>

#include <GLFW/glfw3.h>

struct WindowConfig {
  int width = 1720;
  int height = 800;
  std::string name = "Aelkyn";
  bool resizeable = true;
};

class Window {
public:
  Window(const WindowConfig &config = {});
  ~Window();

  bool shouldClose();
  void pollEvents();

private:
  GLFWwindow *window;
  int currentHeight = 0;
  int currentWidht = 0;
};
