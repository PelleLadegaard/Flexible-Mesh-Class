#pragma once

#include <cassert>
#include <typeinfo>
#include <vector>

namespace fmc {
	// The available attribute types
	enum Attribute {
		ATTR_POS,
		ATTR_NORM,
		ATTR_COL,
		ATTR_UV,

		ATTRIBUTE_COUNT
	};

	// Global class that holds data type hash ID's and sizes, indexed by the vertex attribute enum
	class AttributeInfo {
	public:
		// Store hash ID and size of given data type
		static void initialize(size_t _attributeCount) {
			get_sizes().resize(_attributeCount);
			get_types().resize(_attributeCount);
		}
		template <class T> static void set_data(Attribute _attribute) {
			get_sizes()[_attribute] = sizeof(T);
			get_types()[_attribute] = typeid(T).hash_code();
		}
		static size_t get_size(Attribute _attribute) { return get_sizes()[_attribute]; }
		static size_t get_type(Attribute _attribute) { return get_types()[_attribute]; }
	private:
		static std::vector<size_t>& get_sizes() {
			static std::vector<size_t> m_sizes;
			return m_sizes;
		}
		static std::vector<size_t>& get_types() {
			static std::vector<size_t> m_types;
			return m_types;
		}
	};

	class Vertex;
	// Mesh class, storing the data of a mesh, which attributes define the mesh, and the size of a single vertex in bytes
	class Mesh {
	public:
		Mesh(std::initializer_list<Attribute> _attributes);
		Mesh(const std::vector<Attribute>& _attributes);
		~Mesh();
		Mesh(const Mesh& _other);
		Mesh& operator=(const Mesh& _other);
		Mesh(Mesh&& _other);
		Mesh& operator=(Mesh&& _other);
		// When accessing the model using the [] operator (which specifies what vertex you want to access),
		// return an object holding the address of said vertex
		Vertex& operator[](size_t _index);
		// Add a vertex to the mesh
		template <class T, class... Ts> void push_back(T const& _first, Ts const&... _rest);
		void push_back(const Vertex& _vertex);
		// Retrieve a void pointer to the start of the data, which is useful for GPU uploading
		const void* data() const;
		// Retrieve the amount of vertices found in the model
		size_t size() const;
		// Retrieve the size in bytes of a single vertex
		size_t get_vertex_size() const;
		// Retrieve an array that holds the attributes that define the mesh
		const std::vector<Attribute>& get_attributes() const;
		// Shrink the buffer to fit the vertex data
		void shrink_to_fit();
		// Extends the data container to fit the requested amount of vertices
		void reserve(size_t _vertexCount);
		void clear();

		Mesh() = delete;

	private:
		char* m_data = nullptr;
		Vertex* m_vertices;
		std::vector<Attribute> m_attributes;
		size_t m_vertexSize;
		size_t m_vertexCount;
		size_t m_capacity;

		// Add a vertex element to the mesh
		template <class T, class... Ts> void push_back_rest(char* _address, size_t _attribute, T const& _first, Ts const&... _rest);
		// Reallocates the vertex buffer
		void reallocate(size_t _capacity);
	};

	// Vertex class, holding either an address that points towards the starting position of its data in the mesh's buffer,
	// or an address that points towards its owned data
	class Vertex {
		friend Mesh;

		// Vertex element class, holding an address that points towards the starting position of its data in the vertex's buffer
		class Element {
			friend Vertex;
		public:
			~Element() {};
			// To assign one Element to another a move is used instead of a copy, read below for an explanation (*)
			Element& operator=(Element&& _other);
			// When assigning a value to the vertex element, store the value inside of the vertex's buffer
			template <class T> void operator=(const T& _value);
			// When assigning the element to an object, cast the element's data to the object's class
			template <class T> operator T() const;
			// A getter to cast the data that is held by the element to the class it represents,
			// this is useful when making use of the element directly as a function parameter
			template <class T> T& get();
			template <class T> const T& get() const;

			Element() = delete;
			Element(const Element& _other) = delete;
			Element& operator=(const Element& _other) = delete;
			Element(Element&& _other) = delete;

		private:
			char* m_data;
			Attribute m_attribute;

			Element(char* _data, Attribute _attribute);
		};

	public:
		Vertex(std::initializer_list<Attribute> _attributes);
		Vertex(const std::vector<Attribute>& _attributes);
		template <class T, class... Ts> Vertex(std::initializer_list<Attribute> _attributes, T const& _first, Ts const&... _rest);
		template <class T, class... Ts> Vertex(const std::vector<Attribute>& _attributes, T const& _first, Ts const&... _rest);
		~Vertex();
		Vertex(const Vertex& _other);
		Vertex& operator=(const Vertex& _other);
		Vertex(Vertex&& _other);
		Vertex& operator=(Vertex&& _other);
		// When accessing the vertex using the [] operator (which specifies what vertex element you want to access),
		// return an object holding the address of said vertex element
		Element operator[](Attribute _attribute);
		const Element operator[](Attribute _attribute) const;
		template <class T, class... Ts> void set(T const& _first, Ts const&... _rest);

		Vertex() = delete;

	private:
		char* m_data;
		std::vector<Attribute>* m_attributes;

		void initialize(char* _data, std::vector<Attribute>* _attributes);
		template <class T, class... Ts> void set_rest(char* _address, size_t _attribute, T const& _first, Ts const&... _rest);
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Templated functions definitions
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template <class T, class... Ts>
	void Mesh::push_back(T const& _first, Ts const&... _rest) {
		assert(("The argument count does not match the attribute count", sizeof...(_rest) == m_attributes.size() - 1));
		assert(("Attribute mismatch", typeid(T).hash_code() == AttributeInfo::get_type(m_attributes[0])));

		// Allocate more space when necessary
		if (m_vertexCount == m_capacity) {
			reserve(m_capacity * 2);
		}

		// Write the first vertex element to the mesh's buffer
		char* address = m_data + m_vertexCount * m_vertexSize;
		*(T*)address = _first;

		// Write the remaining vertex elements to the mesh's buffer
		if constexpr (sizeof...(_rest) > 0) {
			push_back_rest(address + sizeof(T), 1, _rest...);
		}

		m_vertices[m_vertexCount].initialize(m_data + m_vertexCount * m_vertexSize, &m_attributes);
		m_vertexCount++;
	}
	template <class T, class... Ts>
	void Mesh::push_back_rest(char* _address, size_t _attribute, T const& _first, Ts const&... _rest) {
		assert(("Attribute mismatch", typeid(T).hash_code() == AttributeInfo::get_type(m_attributes[_attribute])));

		// Write the current vertex element to the mesh's buffer
		*(T*)_address = _first;

		// Write the remaining vertex elements to the mesh's buffer
		if constexpr (sizeof...(_rest) > 0) {
			push_back_rest(_address + sizeof(T), _attribute + 1, _rest...);
		}
	}

	template <class T, class... Ts>
	Vertex::Vertex(std::initializer_list<Attribute> _attributes, T const& _first, Ts const&... _rest) {
		assert(("The argument count does not match the attribute count", sizeof...(_rest) == _attributes.size() - 1));
		assert(("Attribute mismatch", typeid(T).hash_code() == AttributeInfo::get_type(*_attributes.begin())));

		size_t vertexSize = 0;
		m_attributes = new std::vector<Attribute>;
		for (const auto& attribute : _attributes) {
			(*m_attributes).push_back(attribute);
			vertexSize += AttributeInfo::get_size(attribute);
		}

		m_data = (char*)malloc(vertexSize);
		*(T*)m_data = _first;

		// Write the remaining vertex elements to the vertex's buffer
		if constexpr (sizeof...(_rest) > 0) {
			set_rest(m_data + sizeof(T), 1, _rest...);
		}
	}
	template <class T, class... Ts>
	Vertex::Vertex(const std::vector<Attribute>& _attributes, T const& _first, Ts const&... _rest) {
		assert(("The argument count does not match the attribute count", sizeof...(_rest) == _attributes.size() - 1));
		assert(("Attribute mismatch", typeid(T).hash_code() == AttributeInfo::get_type(_attributes[0])));

		size_t vertexSize = 0;
		m_attributes = new std::vector<Attribute>;
		for (const auto& attribute : _attributes) {
			(*m_attributes).push_back(attribute);
			vertexSize += AttributeInfo::get_size(attribute);
		}

		m_data = (char*)malloc(vertexSize);
		*(T*)m_data = _first;

		// Write the remaining vertex elements to the vertex's buffer
		if constexpr (sizeof...(_rest) > 0) {
			set_rest(m_data + sizeof(T), 1, _rest...);
		}
	}
	template <class T, class... Ts>
	void Vertex::set(T const& _first, Ts const&... _rest) {
		assert(("The argument count does not match the attribute count", sizeof...(_rest) == m_attributes->size() - 1));
		assert(("Attribute mismatch", typeid(T).hash_code() == AttributeInfo::get_type((*m_attributes)[0])));

		*(T*)m_data = _first;

		// Write the remaining vertex elements to the vertex's buffer
		if constexpr (sizeof...(_rest) > 0) {
			set_rest(m_data + sizeof(T), 1, _rest...);
		}
	}
	template <class T, class... Ts>
	void Vertex::set_rest(char* _address, size_t _attribute, T const& _first, Ts const&... _rest) {
		assert(("Attribute mismatch", typeid(T).hash_code() == AttributeInfo::get_type((*m_attributes)[_attribute])));

		// Write the current vertex element to the mesh's buffer
		*(T*)_address = _first;

		// Write the remaining vertex elements to the mesh's buffer
		if constexpr (sizeof...(_rest) > 0) {
			push_back_rest(_address + sizeof(T), _attribute + 1, _rest...);
		}
	}

	template <class T>
	void Vertex::Element::operator=(const T& _value) {
		assert(("Incorrect type in element writing", typeid(T).hash_code() == AttributeInfo::get_type(m_attribute)));

		*(T*)m_data = _value;
	}
	template <class T>
	Vertex::Element::operator T() const {
		assert(("Incorrect type in element reading", typeid(T).hash_code() == AttributeInfo::get_type(m_attribute)));

		return *(T*)(m_data);
	}
	template <class T>
	T& Vertex::Element::get() {
		assert(("Incorrect type in element reading", typeid(T).hash_code() == AttributeInfo::get_type(m_attribute)));

		return *(T*)(m_data);
	}
	template <class T>
	const T& Vertex::Element::get() const {
		assert(("Incorrect type in element reading", typeid(T).hash_code() == AttributeInfo::get_type(m_attribute)));

		return *(T*)(m_data);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Non-templated functions definitions
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef FMC_IMPLEMENTATION
	Mesh::Mesh(std::initializer_list<Attribute> _attributes) : m_vertexCount(0), m_capacity(1), m_vertexSize(0) {
		for (auto attribute : _attributes) {
			m_attributes.push_back(attribute);
			m_vertexSize += AttributeInfo::get_size(attribute);
		}

		m_data = (char*)malloc(m_capacity * m_vertexSize);
		m_vertices = (Vertex*)malloc(m_capacity * sizeof(Vertex));
	}
	Mesh::Mesh(const std::vector<Attribute>& _attributes) : m_vertexCount(0), m_capacity(1), m_vertexSize(0) {
		for (auto attribute : _attributes) {
			m_attributes.push_back(attribute);
			m_vertexSize += AttributeInfo::get_size(attribute);
		}

		m_data = (char*)malloc(m_capacity * m_vertexSize);
		m_vertices = (Vertex*)malloc(m_capacity * sizeof(Vertex));
	}
	Mesh::~Mesh() {
		free(m_data);
		free(m_vertices);
	}
	Mesh::Mesh(const Mesh& _other) : m_attributes(_other.m_attributes), m_vertexSize(_other.m_vertexSize), m_vertexCount(_other.m_vertexCount), m_capacity(m_vertexCount) {
		m_data = (char*)malloc(m_vertexCount * m_vertexSize);
		memcpy((void*)m_data, (void*)_other.m_data, m_vertexCount * m_vertexSize);

		m_vertices = (Vertex*)malloc(m_vertexCount * sizeof(Vertex));

		for (size_t index = 0; index < m_vertexCount; ++index) {
			m_vertices[index].initialize(m_data + index * m_vertexSize, &m_attributes);
		}
	}
	Mesh& Mesh::operator=(const Mesh& _other) {
		if (this != &_other) {
#ifndef NDEBUG
			assert(("Vertices don't have the same amount of vertex attributes.", m_attributes.size() == _other.m_attributes.size()));

			std::vector<Attribute>::iterator iteratorSelf = m_attributes.begin();
			std::vector<Attribute>::const_iterator iteratorOther = _other.m_attributes.begin();
			for (size_t i = 0; i < m_attributes.size(); ++i) {
				assert(("Vertex attributes do not align", *iteratorSelf++ == *iteratorOther++));
			}
#endif

			if (_other.m_vertexCount < m_vertexCount) {
				m_vertexCount = _other.m_vertexCount;
			}
			reallocate(_other.m_vertexCount);

			for (; m_vertexCount < _other.m_vertexCount; ++m_vertexCount) {
				m_vertices[m_vertexCount].initialize(m_data + m_vertexCount * m_vertexSize, &m_attributes);
			}

			m_attributes = _other.m_attributes;
			m_vertexSize = _other.m_vertexSize;

			memcpy((void*)m_data, (void*)_other.m_data, m_vertexCount * m_vertexSize);
		}
		return *this;
	}
	Mesh::Mesh(Mesh&& _other) {
		m_data = _other.m_data;
		m_vertices = _other.m_vertices;
		m_attributes = std::move(_other.m_attributes);
		m_vertexSize = std::move(_other.m_vertexSize);
		m_vertexCount = std::move(_other.m_vertexCount);
		m_capacity = std::move(_other.m_capacity);

		for (size_t index = 0; index < m_vertexCount; ++index) {
			m_vertices[index].m_attributes = &m_attributes;
		}

		_other.m_data = nullptr;
		_other.m_vertices = nullptr;
		_other.m_attributes.clear();
		_other.m_vertexSize = 0;
		_other.m_vertexCount = 0;
		_other.m_capacity = 0;
	}
	Mesh& Mesh::operator=(Mesh&& _other) {
		if (this != &_other) {
#ifndef NDEBUG
			assert(("Vertices don't have the same amount of vertex attributes.", m_attributes.size() == _other.m_attributes.size()));

			std::vector<Attribute>::iterator iteratorSelf = m_attributes.begin();
			std::vector<Attribute>::const_iterator iteratorOther = _other.m_attributes.begin();
			for (size_t i = 0; i < m_attributes.size(); ++i) {
				assert(("Vertex attributes do not align", *iteratorSelf++ == *iteratorOther++));
			}
#endif

			free(m_data);
			free(m_vertices);

			m_data = _other.m_data;
			m_vertices = _other.m_vertices;
			m_attributes = std::move(_other.m_attributes);
			m_vertexSize = std::move(_other.m_vertexSize);
			m_vertexCount = std::move(_other.m_vertexCount);
			m_capacity = std::move(_other.m_capacity);

			for (size_t index = 0; index < m_vertexCount; ++index) {
				m_vertices[index].m_attributes = &m_attributes;
			}

			_other.m_data = nullptr;
			_other.m_vertices = nullptr;
			_other.m_attributes.clear();
			_other.m_vertexSize = 0;
			_other.m_vertexCount = 0;
			_other.m_capacity = 0;
		}

		return *this;
	}
	Vertex& Mesh::operator[](size_t _index) {
		assert(("Index out of range", _index < m_vertexCount));

		return m_vertices[_index];
	}
	void Mesh::push_back(const Vertex& _vertex) {
		// Allocate more space when necessary
		if (m_vertexCount == m_capacity) {
			reserve(m_capacity * 2);
		}

		m_vertices[m_vertexCount].initialize(m_data + m_vertexCount * m_vertexSize, &m_attributes);
		m_vertices[m_vertexCount++] = _vertex;
	}
	const void* Mesh::data() const {
		return (const void*)m_data;
	}
	size_t Mesh::size() const {
		return m_vertexCount;
	}
	size_t Mesh::get_vertex_size() const {
		return m_vertexSize;
	}
	const std::vector<Attribute>& Mesh::get_attributes() const {
		return m_attributes;
	}
	void Mesh::shrink_to_fit() {
		if (m_capacity == m_vertexCount) {
			return;
		}

		reallocate(m_vertexCount);
	}
	void Mesh::reserve(size_t _capacity) {
		if (m_capacity >= _capacity) {
			return;
		}

		reallocate(_capacity);
	}
	void Mesh::clear() {
		m_vertexCount = 0;
		reallocate(1);
	}
	void Mesh::reallocate(size_t _capacity) {
		m_capacity = _capacity;

		char* oldAddress = m_data;
		m_data = (char*)realloc(m_data, m_capacity * m_vertexSize);
		m_vertices = (Vertex*)realloc(m_vertices, m_capacity * sizeof(Vertex));

		// SLOW! Should be avoided by (excessive) use of reserve
		if (m_data != oldAddress) {
			for (size_t index = 0; index < m_vertexCount; ++index) {
				m_vertices[index].m_data = m_data + index * m_vertexSize;
			}
		}
	}

	Vertex::Vertex(std::initializer_list<Attribute> _attributes) {
		size_t vertexSize = 0;
		m_attributes = new std::vector<Attribute>;
		for (const auto& attribute : _attributes) {
			(*m_attributes).push_back(attribute);
			vertexSize += AttributeInfo::get_size(attribute);
		}

		m_data = (char*)malloc(vertexSize);
	}
	Vertex::Vertex(const std::vector<Attribute>& _attributes) {
		size_t vertexSize = 0;
		m_attributes = new std::vector<Attribute>;
		for (const auto& attribute : _attributes) {
			(*m_attributes).push_back(attribute);
			vertexSize += AttributeInfo::get_size(attribute);
		}

		m_data = (char*)malloc(vertexSize);
	}
	Vertex::~Vertex() {
		// The destructor is never called on vertices owned by a mesh
		free(m_data);
		delete m_attributes;
	}
	Vertex::Vertex(const Vertex& _other) {
		size_t vertexSize = 0;
		m_attributes = new std::vector<Attribute>;
		for (const auto& attribute : *_other.m_attributes) {
			(*m_attributes).push_back(attribute);
			vertexSize += AttributeInfo::get_size(attribute);
		}

		m_data = (char*)malloc(vertexSize);
		memcpy((void*)m_data, (void*)_other.m_data, vertexSize);
	}
	Vertex& Vertex::operator=(const Vertex& _other) {
		if (this != &_other) {
			size_t vertexSize = 0;
#ifndef NDEBUG
			assert(("Vertices don't have the same amount of vertex attributes.", m_attributes->size() == _other.m_attributes->size()));

			std::vector<Attribute>::iterator iteratorSelf = m_attributes->begin();
			std::vector<Attribute>::iterator iteratorOther = _other.m_attributes->begin();
			for (size_t i = 0; i < m_attributes->size(); ++i) {
				vertexSize += AttributeInfo::get_size(*iteratorSelf);
				assert(("Vertex attributes do not align", *iteratorSelf++ == *iteratorOther++));
			}
#else
			for (const auto& attribute : *m_attributes) {
				vertexSize += AttributeInfo::get_size(attribute);
			}
#endif

			memcpy((void*)m_data, (void*)_other.m_data, vertexSize);
		}

		return *this;
	}
	Vertex::Vertex(Vertex&& _other) {
		m_data = _other.m_data;
		m_attributes = _other.m_attributes;

		_other.m_data = nullptr;
		_other.m_attributes = nullptr;
	}
	Vertex& Vertex::operator=(Vertex&& _other) {
		if (this != &_other) {
			size_t vertexSize = 0;
#ifndef NDEBUG
			assert(("Vertices don't have the same amount of vertex attributes.", m_attributes->size() == _other.m_attributes->size()));

			std::vector<Attribute>::iterator iteratorSelf = m_attributes->begin();
			std::vector<Attribute>::iterator iteratorOther = _other.m_attributes->begin();
			for (size_t i = 0; i < m_attributes->size(); ++i) {
				vertexSize += AttributeInfo::get_size(*iteratorSelf);
				assert(("Vertex attributes do not align", *(iteratorSelf++) == *(iteratorOther++)));
			}
#else
			for (const auto& attribute : *m_attributes) {
				vertexSize += AttributeInfo::get_size(attribute);
			}
#endif

			memcpy((void*)m_data, (void*)_other.m_data, vertexSize);

			_other.m_data = nullptr;
			_other.m_attributes = nullptr;
		}

		return *this;
	}
	Vertex::Element Vertex::operator[](Attribute _attribute) {
		size_t vertexOffset = 0;
		for (const auto& attribute : *m_attributes) {
			if (attribute == _attribute) {
				return Element(m_data + vertexOffset, _attribute);
			}

			vertexOffset += AttributeInfo::get_size(attribute);
		}

		assert(("Unused attribute type", false));
		return Element(nullptr, (Attribute)0);
	}
	const Vertex::Element Vertex::operator[](Attribute _attribute) const {
		size_t vertexOffset = 0;
		for (const auto& attribute : *m_attributes) {
			if (attribute == _attribute) {
				return Element(m_data + vertexOffset, _attribute);
			}

			vertexOffset += AttributeInfo::get_size(attribute);
		}

		assert(("Unused attribute type", false));
		return Element(nullptr, (Attribute)0);
	}
	void Vertex::initialize(char* _data, std::vector<Attribute>* _attributes) {
		m_data = _data;
		m_attributes = _attributes;
	}

	Vertex::Element& Vertex::Element::operator=(Element&& _other) {
		assert(("Type mismatch at element copying", AttributeInfo::get_type(_other.m_attribute) == AttributeInfo::get_type(m_attribute)));

		memcpy((void*)m_data, (void*)_other.m_data, AttributeInfo::get_size(m_attribute));
		return *this;
	}
	Vertex::Element::Element(char* data, Attribute attribute) : m_data(data), m_attribute(attribute) {}
#endif
}

// (*)
// A move operation is used instead of a copy operation when assigning an Element to another Element.
// This is done due to the fact that it should only be possible to assign a temporary Element variable to another,
// which calls a move operation instead of a copy operation.
