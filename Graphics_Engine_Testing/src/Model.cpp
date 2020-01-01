#include "Model.h"

Model::Model(const std::vector<Attribute>& attributes, uint vertCount) {
	m_attributes = attributes;
	m_vertexCount = vertCount;

	m_vertexSize = 0;
	for (int i = 0; i < attributes.size(); i++) {
		m_attrLocs[attributes[i]] = m_vertexSize;
		m_vertexSize += AttributeData::get_size(attributes[i]);
	}

	m_data = (char*)malloc(m_vertexCount*m_vertexSize);
}
Model::Model(const Model &other) {
	m_attributes = other.m_attributes;
	m_attrLocs = other.m_attrLocs;
	m_vertexSize = other.m_vertexSize;
	m_vertexCount = other.m_vertexCount;

	m_data = (char*)malloc(m_vertexCount*m_vertexSize);
	memcpy((void*)m_data, (void*)other.m_data, m_vertexCount*m_vertexSize);
}
Model::~Model() {
	free(m_data);
}
Model& Model::operator = (const Model &other) {
	if (this != &other) {
		free(m_data);

		m_attributes = other.m_attributes;
		m_attrLocs = other.m_attrLocs;
		m_vertexSize = other.m_vertexSize;
		m_vertexCount = other.m_vertexCount;

		m_data = (char*)malloc(m_vertexCount*m_vertexSize);
		memcpy((void*)m_data, (void*)other.m_data, m_vertexCount*m_vertexSize);
	}
	return *this;
}
Model& Model::operator=(Model&& other)
{
	free(m_data);

	m_attributes = std::move(other.m_attributes);
	m_attrLocs = std::move(other.m_attrLocs);
	m_vertexSize = std::move(other.m_vertexSize);
	m_vertexCount = std::move(other.m_vertexCount);

	m_data = (char*)malloc(m_vertexCount*m_vertexSize);
	memcpy((void*)m_data, (void*)other.m_data, m_vertexCount*m_vertexSize);

	free(other.m_data);
	other.m_data = nullptr;
	other.m_vertexSize = 0;
	other.m_vertexCount = 0;

	return *this;
}
Vertex Model::operator[](size_t idx) {
	Vertex vert;
	vert.m_attrLocs = &m_attrLocs;
	vert.m_data = m_data + m_vertexSize * idx;

	return vert;
}
void* Model::data() {
	return (void*)m_data;
}
size_t Model::get_size() {
	return m_vertexCount;
}
const std::vector<Attribute>& Model::get_attributes() {
	return m_attributes;
}

VertexElement Vertex::operator[](Attribute attrib) {
	assertm((*m_attrLocs).count(attrib) == 1, "Non-existing attribute type");
	VertexElement attr;
	attr.m_data = m_data + (*m_attrLocs)[attrib];
	attr.m_attr = attrib;
	return attr;
}