#include "Globals.h"

size_t AttributeData::get_size(Attribute attr) {
	return m_sizes[attr];
}
size_t AttributeData::get_type(Attribute attr) {
	return m_types[attr];
}