#include "renderer.hpp"

#include <GLFW/glfw3.h>

void Renderer::init(Window &window) {
  context.init(window);
  graphicsPipeline.init(context);
  buffer.init(context);
  commands.init(context, graphicsPipeline, buffer);
  frames.init(context, commands, window);
}

void Renderer::drawFrame() { frames.drawFrame(); }

void Renderer::waitIdle() { context.getLogicalDevice().waitIdle(); }
