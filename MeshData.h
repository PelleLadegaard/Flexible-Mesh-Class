#pragma once

// Uncomment to remove all asserts to squeeze out some additional performance (only recommended for release builds)
//#define MESHDATA_UNSAFE

#include <map>
#include <assert.h>

// The available attribute types
enum MeshAttribute {
	ATTR_POS,
	ATTR_NORM,
	ATTR_COL,
	ATTR_UV
};

// Global class that holds data type hash ID's and sizes, indexed by the vertex attribute enum
class AttributeData {
public:
	// Store hash ID and size of given data type
	template <class T> static void set_data(MeshAttribute attr) {
		get_sizes()[attr] = sizeof(T);
		get_types()[attr] = typeid(T).hash_code();
	}
	static size_t get_size(MeshAttribute attr) { return get_sizes()[attr]; }
	static size_t get_type(MeshAttribute attr) { return get_types()[attr]; }
private:
	static std::map<MeshAttribute, size_t>& get_sizes(){
		static std::map<MeshAttribute, size_t> m_sizes;
		return m_sizes;
	}
	static std::map<MeshAttribute, size_t>& get_types(){
		static std::map<MeshAttribute, size_t> m_sizes;
		return m_sizes;
	}
};

class Vertex;
// Mesh data class, storing all the data of a mesh, which attributes define the mesh, and the size of a single vertex in bytes
class MeshData {
public:
	// Create a mesh and define which attributes it's made out of
	MeshData(std::initializer_list<MeshAttribute> attributes);
	// Copy constructor
	MeshData(const MeshData& other);
	// Frees mesh data on destruction
	~MeshData();
	// Copy assignment operator
	MeshData& operator=(const MeshData& other);
	// Move operator
	MeshData& operator=(MeshData&& other);
	// When accessing the model using the [] operator (which specifies what vertex you want to access), return an object holding the data address of said vertex
	Vertex operator[](size_t idx);
	// Add a vertex to the mesh
	template <class T, class... Ts> void push_back(T const& first, Ts const&... rest);
	// Retrieve a void pointer to the start of the data. This can be used to copy the data over to another mesh object, or to assign it to an OpenGL VBO
	const void* data();
	// Retrieve the amount of vertices found in the model
	size_t size();
	// Retrieve the size in bytes of a single vertex
	size_t get_vertex_size();
	// Return the attribute count of each vertex
	size_t get_attributeCount();
	// Retrieve an array that holds the attributes that define the mesh
	MeshAttribute* get_attributes();
	// Shrink the buffer to fit the vertices
	void shrink_to_fit();
	// Extends the data container to fit the requested amount of vertices
	void reserve(size_t vertexCount);

private:
	char* m_data = nullptr;
	MeshAttribute* m_attributes;
	std::map<MeshAttribute, size_t> m_attributeOffsets;
	size_t m_attributeCount;
	size_t m_vertexSize;
	size_t m_vertexCount;
	size_t m_capacity;

	// Add a vertex element to the mesh
	template <class T, class... Ts> void push_back_rest(char* address, size_t attribute, T const& first, Ts const&... rest);
};

class VertexElement;
// Vertex class, holding a pointer that points towards the starting position of its data in the mesh's buffer
class Vertex {
	friend MeshData;
public:
	Vertex(char* data, std::map<MeshAttribute, size_t>* attributeOffsets, size_t vertexSize) : m_data(data), m_attributeOffsets(attributeOffsets), m_vertexSize(vertexSize) {}
	~Vertex() {}
	// Copy assignment operator
	Vertex& operator=(const Vertex& other);
	// When accessing the model using the [] operator (which specifies what vertex element you want to access), return an object holding the data address of said vertex element
	VertexElement operator[](MeshAttribute attrib);

private:
	char* m_data;
	std::map<MeshAttribute, size_t>* m_attributeOffsets;
	size_t m_vertexSize;
};

// Vertex element class, holding a pointer that points towards the starting position of its data in the mesh's buffer
class VertexElement {
	friend Vertex;
public:
	VertexElement(char* data, MeshAttribute attribute) : m_data(data), m_attribute(attribute) {}
	~VertexElement() {}
	// Copy assignment operator
	VertexElement& operator=(const VertexElement& other);
	// When assigning a value to the vertex element, store the value inside of the mesh's buffer
	template <class T> VertexElement& operator=(const T& value);
	// When assigning the vertex element to an object, cast the vertex element's data to the object's type and assign it to the object
	template <class T> operator T() const;
	// A getter to cast the data that is hold by the vertex element to the class it represents, this is useful when making use of the vertex element directly as function parameter
	template <class T> T& get() const;

private:
	char* m_data;
	MeshAttribute m_attribute;
};









///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Meshdata function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MeshData::MeshData(std::initializer_list<MeshAttribute> attributes) : m_vertexCount(0), m_capacity(128), m_vertexSize(0), m_attributeCount(0) {
	m_attributes = new MeshAttribute[attributes.size()];
	for (auto attribute : attributes) {
		m_attributes[m_attributeCount++] = attribute;
		m_attributeOffsets[attribute] = m_vertexSize;
		m_vertexSize += AttributeData::get_size(attribute);
	}

	m_data = (char*)malloc(m_capacity * m_vertexSize);
}
MeshData::MeshData(const MeshData& other) : m_attributeCount(other.m_attributeCount), m_attributeOffsets(other.m_attributeOffsets), m_vertexSize(other.m_vertexSize),
m_vertexCount(other.m_vertexCount), m_capacity(m_vertexCount) {
	m_attributes = new MeshAttribute[m_attributeCount];
	memcpy((void*)m_attributes, (void*)other.m_attributes, m_attributeCount * sizeof(MeshAttribute));
	m_data = (char*)malloc(m_vertexCount * m_vertexSize);
	memcpy((void*)m_data, (void*)other.m_data, m_vertexCount * m_vertexSize);
}
MeshData::~MeshData() {
	free(m_data);
	delete[] m_attributes;
}
MeshData& MeshData::operator=(const MeshData& other) {
	if (this != &other) {
		m_attributeOffsets = other.m_attributeOffsets;
		m_attributeCount = other.m_attributeCount;
		m_vertexSize = other.m_vertexSize;
		m_vertexCount = other.m_vertexCount;
		m_capacity = m_vertexCount;

		delete[] m_attributes;
		m_attributes = new MeshAttribute[m_attributeCount];
		memcpy((void*)m_attributes, (void*)other.m_attributes, m_attributeCount * sizeof(MeshAttribute));
		free(m_data);
		m_data = (char*)malloc(m_vertexCount * m_vertexSize);
		memcpy((void*)m_data, (void*)other.m_data, m_vertexCount * m_vertexSize);
	}
	return *this;
}
MeshData& MeshData::operator=(MeshData&& other) {
	m_attributeOffsets = std::move(other.m_attributeOffsets);
	m_attributeCount = std::move(other.m_attributeCount);
	m_vertexSize = std::move(other.m_vertexSize);
	m_vertexCount = std::move(other.m_vertexCount);
	m_capacity = m_vertexCount;

	delete[] m_attributes;
	m_attributes = other.m_attributes;
	free(m_data);
	m_data = other.m_data;

	other.m_attributes = nullptr;
	other.m_data = nullptr;
	other.m_attributeOffsets.clear();
	other.m_attributeCount = 0;
	other.m_vertexSize = 0;
	other.m_vertexCount = 0;
	other.m_capacity = 0;

	return *this;
}
Vertex MeshData::operator[](size_t idx) {
#ifndef MESHDATA_UNSAFE
	assert(("Index out of range", idx < m_vertexCount));
#endif

	return Vertex(m_data + m_vertexSize * idx, &m_attributeOffsets, m_vertexSize);
}
template <class T, class... Ts>
void MeshData::push_back(T const& first, Ts const&... rest) {
#ifndef MESHDATA_UNSAFE
	assert(("The argument count does not match the attribute count", sizeof...(rest) == m_attributeCount - 1));
	assert(("Attribute mismatch at pushback", typeid(T).hash_code() == AttributeData::get_type(m_attributes[0])));
#endif

	// Allocate more space when necessary
	if (m_vertexCount == m_capacity) {
		// Double the size of the capacity
		m_capacity *= 2;

		// Copy the data over to the new and larger buffer
		char* newBuffer = (char*)malloc(m_capacity * m_vertexSize);
		memcpy((void*)newBuffer, (void*)m_data, m_vertexCount * m_vertexSize);

		// Delete the previous buffer, and point to the new one
		free(m_data);
		m_data = newBuffer;
	}

	// Write the first vertex element to the mesh's buffer
	char* address = m_data + m_vertexCount * m_vertexSize;
	*(T*)address = first;

	// Write the remaining vertex elements to the mesh's buffer
	if constexpr (sizeof...(rest) > 0) {
		push_back_rest(address + sizeof(T), 1, rest...);
	}

	m_vertexCount++;
}
const void* MeshData::data() {
	return (const void*)m_data;
}
size_t MeshData::size() {
	return m_vertexCount;
}
size_t MeshData::get_vertex_size() {
	return m_vertexSize;
}
size_t MeshData::get_attributeCount() {
	return m_attributeCount;
}
MeshAttribute* MeshData::get_attributes() {
	return m_attributes;
}
void MeshData::shrink_to_fit() {
	// When there is more data allocated than there are vertices stored, shrink the buffer to fit the size of the set vertices
	if (m_capacity == m_vertexCount) {
		return;
	}

	// Copy the data over to the new buffer that perfectly fits the vertex data
	char* newBuffer = (char*)malloc(m_vertexCount * m_vertexSize);
	memcpy((void*)newBuffer, (void*)m_data, m_vertexCount * m_vertexSize);

	// Delete the previous buffer, and point to the new one
	free(m_data);
	m_data = newBuffer;

	m_capacity = m_vertexCount;
}
void MeshData::reserve(size_t vertexCount) {
	// When you want to reserve more memory than already available, grow the buffer to fit the requested size
	if (m_capacity >= vertexCount) {
		return;
	}

	// Copy the data over to the new and larger buffer
	char* newBuffer = (char*)malloc(vertexCount * m_vertexSize);
	memcpy((void*)newBuffer, (void*)m_data, m_vertexCount * m_vertexSize);

	// Delete the previous buffer, and point to the new one
	free(m_data);
	m_data = newBuffer;

	m_capacity = vertexCount;
}
template <class T, class... Ts>
void MeshData::push_back_rest(char* address, size_t attribute, T const& first, Ts const&... rest) {
#ifndef MESHDATA_UNSAFE
	assert(("Attribute mismatch at pushback", typeid(T).hash_code() == AttributeData::get_type(m_attributes[attribute])));
#endif

	// Write the next vertex element to the mesh's buffer
	* (T*)address = first;

	// Write the remaining vertex elements to the mesh's buffer
	if constexpr (sizeof...(rest) > 0) {
		push_back_rest(address + sizeof(T), attribute + 1, rest...);
	}
}





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Vertex function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vertex& Vertex::operator=(const Vertex& other) {
#ifndef MESHDATA_UNSAFE
	assert(("Vertices don't have the same amount of vertex attributes.", other.m_attributeOffsets->size() == m_attributeOffsets->size()));

	std::map<MeshAttribute, size_t>::iterator iteratorSelf = m_attributeOffsets->begin();
	std::map<MeshAttribute, size_t>::iterator iteratorOther = other.m_attributeOffsets->begin();
	for (int i = 0; i < m_attributeOffsets->size(); ++i) {
		assert(("Vertex attributes do not align", iteratorSelf++->first == iteratorOther++->first));
	}
#endif

	memcpy((void*)m_data, (void*)other.m_data, m_vertexSize);

	return *this;
}
VertexElement Vertex::operator[](MeshAttribute attrib) {
#ifndef MESHDATA_UNSAFE
	assert(("Non-existing attribute type", m_attributeOffsets->count(attrib) == 1));
#endif

	return VertexElement(m_data + (*m_attributeOffsets)[attrib], attrib);
}





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// VertexElement function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VertexElement& VertexElement::operator=(const VertexElement& other) {
#ifndef MESHDATA_UNSAFE
	assert(("Type mismatch at attribute copying", other.m_attribute == m_attribute));
#endif

	memcpy((void*)m_data, (void*)other.m_data, AttributeData::get_size(m_attribute));
	return *this;
}
template <class T>
VertexElement& VertexElement::operator=(const T& value) {
#ifndef MESHDATA_UNSAFE
	assert(("Incorrect type in attribute writing", typeid(T).hash_code() == AttributeData::get_type(m_attribute)));
#endif

	// Write the element to the mesh's buffer
	if (this != (VertexElement*)&value) {
		*(T*)m_data = value;
	}
	return *this;
}
template <class T>
VertexElement::operator T() const {
#ifndef MESHDATA_UNSAFE
	assert(("Incorrect type in attribute reading", typeid(T).hash_code() == AttributeData::get_type(m_attribute)));
#endif

	return *(T*)(m_data);
}
template <class T>
T& VertexElement::get() const {
#ifndef MESHDATA_UNSAFE
	assert(("Incorrect type in attribute reading", typeid(T).hash_code() == AttributeData::get_type(m_attribute)));
#endif

	return *(T*)(m_data);
}