#include "Application.hpp"
#include <cstdlib>
#include <exception>
#include <iostream>

int main() {

  Application app;
  try {
    app.run();
  } catch (std::exception &e) {
    std::cerr << "Exception Caught" << e.what();
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
