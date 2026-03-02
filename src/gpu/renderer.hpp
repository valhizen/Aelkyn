#pragma once

#include "buffer.hpp"
#include "commands.hpp"
#include "context.hpp"
#include "frames.hpp"
#include "pipeline.hpp"

class Renderer {
public:
  void init(Window &window);
  void drawFrame();
  void waitIdle();

private:
  Context context;
  GraphicsPipeline graphicsPipeline;
  Buffer buffer;
  Commands commands;
  Frames frames;
};
