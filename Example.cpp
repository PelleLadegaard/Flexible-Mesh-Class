#include <iostream>

#include "MeshData.h"

struct vec2 {
	vec2(float tX, float tY) : x(tX), y(tY) {}
	float x;
	float y;
};
std::ostream& operator<<(std::ostream& os, const vec2& vector) {
	return os << "x: " << vector.x << " y: " << vector.y;
}

struct vec3 {
	vec3(float tX, float tY, float tZ) : x(tX), y(tY), z(tZ) {}
	float x;
	float y;
	float z;
};
std::ostream& operator<<(std::ostream& os, const vec3& vector) {
	return os << "x: " << vector.x << " y: " << vector.y << " z: " << vector.z;
}

int main() {
    // Define what data types are used for every vertex attribute type
    AttributeData::set_data<vec3>(ATTR_POS);
    AttributeData::set_data<vec3>(ATTR_NORM);
    AttributeData::set_data<vec3>(ATTR_COL);
    AttributeData::set_data<vec2>(ATTR_UV);

	MeshData meshTest({ATTR_POS, ATTR_NORM, ATTR_UV});
	meshTest.push_back(vec3(0.f, 0.f, 0.f), vec3(0.f, 0.f, 0.f), vec2(0.f, 0.f));
	meshTest.push_back(vec3(0.f, 1.f, 2.f), vec3(2.f, 1.f, 0.f), vec2(64.f, 256.f));
	meshTest.push_back(vec3(1000.f, 1000.f, 1000.f), vec3(1000.f, 1000.f, 1000.f), vec2(1000.f, 1000.f));
	std::cout << meshTest[0][ATTR_POS].get<vec3>() << " | " << meshTest[0][ATTR_NORM].get<vec3>() << " | " << meshTest[0][ATTR_UV].get<vec2>() << "\n";
	std::cout << meshTest[1][ATTR_POS].get<vec3>() << " | " << meshTest[1][ATTR_NORM].get<vec3>() << " | " << meshTest[1][ATTR_UV].get<vec2>() << "\n";
	std::cout << meshTest[2][ATTR_POS].get<vec3>() << " | " << meshTest[2][ATTR_NORM].get<vec3>() << " | " << meshTest[2][ATTR_UV].get<vec2>() << "\n\n";

	meshTest[0] = meshTest[2];
	std::cout << meshTest[0][ATTR_POS].get<vec3>() << " | " << meshTest[0][ATTR_NORM].get<vec3>() << " | " << meshTest[0][ATTR_UV].get<vec2>() << "\n\n";

	meshTest[0][ATTR_UV] = meshTest[1][ATTR_UV];
	std::cout << meshTest[0][ATTR_UV].get<vec2>() << "\n\n";

	meshTest[0][ATTR_POS].get<vec3>().x += 32.f;
	std::cout << meshTest[0][ATTR_POS].get<vec3>() << "\n\n";

	vec2 vecTest = meshTest[0][ATTR_UV];
	std::cout << vecTest << "\n\n";

	return 0;
}
