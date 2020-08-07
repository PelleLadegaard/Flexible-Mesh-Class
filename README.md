# Flexible-Mesh-Class
This single-header library makes it possible write and store vertex data with arbitrary atributes in a unified mesh class. This allows for quick experimentation, and meshes with different attributes can easily be processed by functions as long as each mesh own the required attributes. The vertex data is interleaving, and stored in a single sequential buffer, which makes the library optimal to store mesh data in graphics engines.

The obvious substitution for this libary is making use of vectors of objects, each different set of attributes being defined by a unique struct/class. By doing this however, it is difficult to make a generalized process that can be applied to every set of attributes. For example, if you have a set of attributes to define regular meshes made up out of position, normal, and UV attributes, then define terrains using position, normal, and color attributes, and lastly wireframe meshes using position and color attributes, then the only attribute that is shared between all three the mesh types is the position attribute. If you want to process models that have a position and normal attributes, you can't write a base parent struct that supports these attributes, due to the fact that the wireframe mesh doesn't need the normal attribute.

This library makes it possible to process any mesh with any arbritrary set of attributes, and it's up to the user to make sure that only meshes with the required attributes are processed. Asserts are in place to notify the user when mistakes are made, but these can be ignored if improved performance is required.

## Code examples
Attribute types are defined in an enum, to assign variable types to each attribute type you make use of the global `set_data()` function:
```cxx
AttributeData::set_data<vec3>(ATTR_POS);
```

When creating a mesh, you need to define which attributes it uses in the constructor:
```cxx
MeshData meshTest({ATTR_POS, ATTR_NORM, ATTR_UV});
```

Meshes behave very similarly to STL vectors, the MeshData class supports functions like `push_back()`, `shrink_to_fit()`, `reserve()` , and `data()` to make its functionality intuitive to understand for the user. Internally the MeshData class also operates in a similar fastion to STL vectors. The object holds a buffer that stores the vertex data, and when `push_back()` is called at full capacity, the object will reallocate its buffer to one with twice the capacity to allow the user to work with variable vertex counts. When pushing back data to the mesh, you'll need to define each attribute of the new vertex as a unique parameter:
```cxx
meshTest.push_back(vec3(0.f, 0.f, 0.f), vec3(0.f, 0.f, 0.f), vec2(0.f, 0.f));
```

Vertex data can easily be accessed using the provided operator overloads:
```cxx
vec2 vecTest = meshTest[0][ATTR_UV];
```

It's also easy to write vertex data of different meshes to each other using the same operators:
```cxx
meshTest[0] = meshTest[2];
meshTest[0][ATTR_UV] = meshTest[1][ATTR_UV];
```

Internally, data is stored in a unified, type-less buffer. Therefore the accessors do not cast the vertex data to the original variable type without a hint. The hint is acquired when assigning the vertex data to a variable by making use of the variable's type, this does however mean that you can't make use of the vertex data directly as function parameters, or to increment it's values. For these use cases there's a getter function that requires the user to tell what variable type is used for the specified vertex data:
```cxx
meshTest[0][ATTR_POS].get<vec3>().x += 32.f;
std::cout << meshTest[0][ATTR_POS].get<vec3>() << "\n\n";
```

## Important information
This library is heavily dependent on RTTR, and it requires a variable type's `hash_code()` to be unique. This is the case for when you compile using Visual Studio, if you use a different compiler please look up its behaviour with `hash_code()` before making use of this library.

This library makes use of C++14 functionality, so make sure that you compile with C++14 at a minimum.
