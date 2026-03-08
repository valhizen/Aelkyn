#include <glm/glm.hpp>

enum class CameraMovement {

};

class Camera {
private:
  glm::vec3 position; // Camera's location in world coordinates
  glm::vec3 front;    // Forward direction (where camera is looking)
  glm::vec3 up;       // Camera's local up direction (for roll control)
  glm::vec3
      right; // Camera's local right direction (perpendicular to front and up)
  glm::vec3 worldUp; // Global up vector reference (typically Y-axis)
};
