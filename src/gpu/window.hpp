
#include <GLFW/glfw3.h>
class Window {

private:
  GLFWwindow *window;

public:
  void init();
  GLFWwindow *getWindow() const { return window; }
};
