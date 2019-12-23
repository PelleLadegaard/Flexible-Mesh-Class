#pragma once

#include <map>

#include "Enums.h"

// Global class that holds data type hash ID's and sizes, indexed by the vertex attribute enum
class AttributeData {
public:
	// Store hash ID and size of given data type
	template <class T>
	static void set_data(Attribute attr) {
		m_sizes[attr] = sizeof(T);
		m_types[attr] = typeid(T).hash_code();
	}
	static size_t get_size(Attribute attr);
	static size_t get_type(Attribute attr);

private:
	inline static std::map<Attribute, size_t> m_sizes;
	inline static std::map<Attribute, size_t> m_types;
};