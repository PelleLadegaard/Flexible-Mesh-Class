#include "Model.h"

Model::Model(const std::vector<Attribute>& attributes, uint vertCount) {
	m_attributes = attributes;

	m_vertexSize = 0;
	for (int i = 0; i < attributes.size(); i++) {
		m_attrLocs[attributes[i]] = m_vertexSize;
		m_vertexSize += AttributeData::get_size(attributes[i]);
	}

	m_data = (char*)malloc(vertCount*m_vertexSize);
}
Model::~Model() {
	free(m_data);
}
Vertex Model::operator[](size_t idx) {
	Vertex vert;
	vert.m_attrLocs = &m_attrLocs;
	vert.m_data = m_data + m_vertexSize * idx;

	return vert;
}

VertexElement Vertex::operator[](Attribute attrib) {
	assertm((*m_attrLocs).count(attrib) == 1, "Non-existing attribute type");
	VertexElement attr;
	attr.m_data = m_data + (*m_attrLocs)[attrib];
	attr.m_attr = attrib;
	return attr;
}