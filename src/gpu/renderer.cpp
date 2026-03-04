#include "renderer.hpp"

#include <GLFW/glfw3.h>

void Renderer::init(Window &window) {
  context.init(window);
  buffer.init(context);
  graphicsPipeline.init(context, buffer);
  commands.init(context, graphicsPipeline, buffer);
  frames.init(context, commands, window, buffer);
}

void Renderer::drawFrame() { frames.drawFrame(); }

void Renderer::waitIdle() { context.getLogicalDevice().waitIdle(); }
