#include <iostream>

#define FMC_IMPLEMENTATION
#include "fmc.h"

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
    fmc::AttributeInfo::set_data<vec3>(fmc::ATTR_POS);
    fmc::AttributeInfo::set_data<vec3>(fmc::ATTR_NORM);
    fmc::AttributeInfo::set_data<vec3>(fmc::ATTR_COL);
    fmc::AttributeInfo::set_data<vec2>(fmc::ATTR_UV);

	// Owned vertex testing
	{
		fmc::Vertex vertexOwned0({fmc::ATTR_POS, fmc::ATTR_UV}, vec3(10.f, 100.f, 1000.f), vec2(64.f, 256.f));
		assert(("Vertex initialization failed", vertexOwned0[fmc::ATTR_POS].get<vec3>().x == 10.f));
		assert(("Vertex initialization failed", vertexOwned0[fmc::ATTR_UV].get<vec2>().y == 256.f));

		vertexOwned0.set(vec3(-10.f, -100.f, -1000.f), vec2(-64.f, -256.f));
		assert(("Vertex setting failed", vertexOwned0[fmc::ATTR_POS].get<vec3>().z == -1000.f));
		assert(("Vertex setting failed", vertexOwned0[fmc::ATTR_UV].get<vec2>().x == -64.f));

		vec2& vec2Ref = vertexOwned0[fmc::ATTR_UV].get<vec2>();
		vertexOwned0[fmc::ATTR_UV].get<vec2>().x = 128.f;
		assert(("Vertex attribute setting failed", vertexOwned0[fmc::ATTR_UV].get<vec2>().x == 128.f));
		assert(("Vertex attribute reference getting failed", vec2Ref.x == 128.f));

		vec2Ref.x = 512.f;
		assert(("Vertex attribute reference setting failed", vertexOwned0[fmc::ATTR_UV].get<vec2>().x == 512.f));
		assert(("Vertex attribute reference getting failed", vec2Ref.x == 512.f));

		fmc::Vertex vertexOwned1({fmc::ATTR_POS, fmc::ATTR_UV}, vec3(3.f, 9.f, 81.f), vec2(1.f, 2.f));
		vertexOwned0[fmc::ATTR_POS] = vertexOwned1[fmc::ATTR_POS];
		assert(("Vertex attribute copying failed", vertexOwned0[fmc::ATTR_POS].get<vec3>().y == 9.f));
		
		vertexOwned0 = vertexOwned1;
		assert(("Vertex copying failed", vertexOwned0[fmc::ATTR_UV].get<vec2>().y == 2.f));

		fmc::Vertex vertexOwned2(vertexOwned0);
		assert(("Vertex copy construction failed", vertexOwned2[fmc::ATTR_POS].get<vec3>().z == 81.f));
		assert(("Vertex copy construction failed", vertexOwned2[fmc::ATTR_UV].get<vec2>().y == 2.f));

		vertexOwned2.set(vec3(0.f, 1.f, 1.f), vec2(2.f, 3.f));
		vertexOwned0 = vertexOwned1 = vertexOwned2;
		assert(("Vertex chained copying failed", vertexOwned0[fmc::ATTR_POS].get<vec3>().z == 1.f));
		assert(("Vertex chained copying failed", vertexOwned1[fmc::ATTR_UV].get<vec2>().y == 3.f));

		fmc::Vertex vertexOwned3 = std::move(vertexOwned0);
		assert(("Vertex move construction failed", vertexOwned3[fmc::ATTR_POS].get<vec3>().z == 1.f));
		assert(("Vertex move construction failed", vertexOwned3[fmc::ATTR_UV].get<vec2>().y == 3.f));
	}

	// Mesh testing
	{
		fmc::Mesh mesh0({fmc::ATTR_POS, fmc::ATTR_UV});
		mesh0.push_back(vec3(10.f, 100.f, 1000.f), vec2(64.f, 256.f));
		mesh0.push_back(vec3(3.f, 9.f, 81.f), vec2(1.f, 2.f));
		assert(("Mesh push back or reallocation failed", mesh0[0][fmc::ATTR_POS].get<vec3>().x == 10.f));
		assert(("Mesh push back or reallocation failed", mesh0[1][fmc::ATTR_UV].get<vec2>().y == 2.f));

		fmc::Vertex& vertexRef = mesh0[0];
		vec2& vec2Ref = mesh0[0][fmc::ATTR_UV].get<vec2>();
		mesh0[0][fmc::ATTR_UV].get<vec2>().x = 128.f;
		assert(("Mesh vertex attribute setting failed", mesh0[0][fmc::ATTR_UV].get<vec2>().x == 128.f));
		assert(("Mesh vertex reference getting failed", vertexRef[fmc::ATTR_UV].get<vec2>().x == 128.f));
		assert(("Mesh vertex attribute reference getting failed", vec2Ref.x == 128.f));

		vertexRef[fmc::ATTR_UV].get<vec2>().x = 512.f;
		assert(("Mesh vertex reference setting failed", mesh0[0][fmc::ATTR_UV].get<vec2>().x == 512.f));
		assert(("Mesh vertex reference getting failed", vertexRef[fmc::ATTR_UV].get<vec2>().x == 512.f));
		assert(("Mesh vertex attribute reference getting failed", vec2Ref.x == 512.f));

		vec2Ref.x = 1024.f;
		assert(("Mesh vertex attribute reference setting failed", mesh0[0][fmc::ATTR_UV].get<vec2>().x == 1024.f));
		assert(("Mesh vertex attribute reference getting failed", vertexRef[fmc::ATTR_UV].get<vec2>().x == 1024.f));
		assert(("Mesh vertex attribute reference getting failed", vec2Ref.x == 1024.f));

		fmc::Mesh mesh1({fmc::ATTR_POS, fmc::ATTR_UV});
		mesh1.push_back(vec3(0.f, 1.f, 1.f), vec2(2.f, 3.f));
		mesh0 = mesh1;
		assert(("Mesh copying failed", mesh0[0][fmc::ATTR_POS].get<vec3>().x == 0.f));
		assert(("Mesh copying failed", mesh0[0][fmc::ATTR_UV].get<vec2>().y == 3.f));

		fmc::Mesh mesh2(mesh0);
		assert(("Mesh copy construction failed", mesh2[0][fmc::ATTR_POS].get<vec3>().x == 0.f));
		assert(("Mesh copy construction failed", mesh2[0][fmc::ATTR_UV].get<vec2>().y == 3.f));
		assert(("Mesh copy construction failed", mesh2.size() == 1));

		mesh2.clear();
		mesh2.push_back(vec3(5.f, 8.f, 13.f), vec2(21.f, 34.f));
		mesh2.push_back(vec3(55.f, 89.f, 144.f), vec2(233.f, 377.f));
		mesh2.push_back(vec3(5.f, 4.f, 3.f), vec2(2.f, 1.f));
		mesh0 = mesh1 = mesh2;
		assert(("Mesh chained copying or clearing failed", mesh0[0][fmc::ATTR_POS].get<vec3>().z == 13.f));
		assert(("Mesh chained copying or clearing failed", mesh1[1][fmc::ATTR_UV].get<vec2>().y == 377.f));
		assert(("Mesh chained copying or clearing failed", mesh0.size() == 3));
		assert(("Mesh chained copying or clearing failed", mesh1.size() == 3));

		fmc::Mesh mesh3 = std::move(mesh0);
		assert(("Mesh move construction failed", mesh3[0][fmc::ATTR_POS].get<vec3>().z == 13.f));
		assert(("Mesh move construction failed", mesh3[1][fmc::ATTR_UV].get<vec2>().y == 377.f));
		assert(("Mesh move construction failed", mesh3.size() == 3));
		assert(("Mesh move construction failed", mesh0.size() == 0));
	}

	return 0;
}
