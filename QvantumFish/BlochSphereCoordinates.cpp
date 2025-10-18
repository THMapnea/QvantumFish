#include "BlochSphereCoordinates.h"
#include <array>
#include <glm/glm.hpp>





glm::vec3 BlochSphereCoordinates::convertToVec3() {
	glm::vec3 converted = { sphericalX(), sphericalY(), sphericalZ() };
	return converted;
}