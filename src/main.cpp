#include "aelkyn.hpp"
#include <cstdlib>
#include <iostream>

int main() {
  try {
    Aelkyn application;
    application.init();
    application.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
