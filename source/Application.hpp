#include "game/Input.hpp"
#include "gpu/Device.hpp"

class Application {
public:
  void run();

private:
  Device device;
  Window window;
  Input input;
};
