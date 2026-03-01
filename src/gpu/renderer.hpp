#include "commands.hpp"
#include "context.hpp"
#include "frames.hpp"
#include "pipeline.hpp"

class Renderer {
public:
  void init(Window &window);

private:
  Context context;
  GraphicsPipeline graphicsPipeline;
  Commands commands;
  Frames frames;
};
