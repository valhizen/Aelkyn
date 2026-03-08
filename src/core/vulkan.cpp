#include "vulkan.hpp"
#include <iostream>

void Vulkan::initVulkan() {
  device.init();
  std::cout << "Vulkan Device Created" << std::endl;
}
