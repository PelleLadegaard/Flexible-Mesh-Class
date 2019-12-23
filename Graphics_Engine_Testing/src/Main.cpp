#include <iostream>

#include "Globals.h"
#include "Enums.h"
#include "Model.h"
#include "Vectors.h"

// EXAMPLE IMPLEMENTATION CODE
int main(){
	// Define which vertex attributes use which data types
	AttributeData::set_data<vec3>(ATTR_POS);
	AttributeData::set_data<vec3>(ATTR_NORM);
	AttributeData::set_data<vec3>(ATTR_COL);
	AttributeData::set_data<vec2>(ATTR_UV);

	// Create model and specify which vertex attributes define the model, and how many vertices it should store
	Model model({ ATTR_POS, ATTR_UV, ATTR_NORM }, 2);
	
	// Fill the model data up
		// Vertex 0
	model[0][ATTR_POS] = vec3(1.f, 0.f, 0.f);
	model[0][ATTR_NORM] = vec3(0.f, 1.f, 0.f);
	model[0][ATTR_UV] = vec2(1.f, 0.f);
		// Vertex 1
	model[1][ATTR_POS] = vec3(0.f, 0.f, 1.f);
	model[1][ATTR_NORM] = vec3(1.f, 1.f, 1.f);
	model[1][ATTR_UV] = vec2(0.f, 1.f);

	// Read model data and print it out
	for (int i = 0; i < 2; i++) {
		vec3 pos = model[i][ATTR_POS];
		vec3 norm = model[i][ATTR_NORM];
		vec2 uv = model[i][ATTR_UV];
		std::cout << pos.x << ", "<< pos.y << ", " << pos.z << std::endl;
		std::cout << norm.x << ", " << norm.y << ", " << norm.z << std::endl;
		std::cout << uv.x << ", " << uv.y << std::endl;
	}



	while (true){}
	return 1;
}