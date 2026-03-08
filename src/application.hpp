#include "core/vulkan.hpp"
#include "core/window.hpp"

class Application {
public:
  void run();

private:
  Window window;
  Vulkan vulkan;
};
