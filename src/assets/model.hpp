#include "../core/rendertypes.hpp"
#include <string>
#include <vector>

class Model {
public:
  void load(const std::string &path);

  const std::vector<Vertex> &getVertices() const { return vertices; }
  const std::vector<uint32_t> &getIndices() const { return indices; }

private:
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
};
