#include "context.hpp"
#include "window.hpp"

class Renderer {
public:
  void run();

private:
  Window window;
  Context context;
};
