Simple serialization library for modern C++ (C++20).

## Basic Usage

A full basic example can be seen in `examples/basic.cpp`.
```
#include "deserialize.hpp"
#include "serialize.hpp"
#include "simple.hpp"

// Serialization
MyType object = ...;
std::ofstream fos{<file>};
serialize::SimpleStreamSerializer s{fos};
serialize::serialize(s, object);
s.flush();

// Deserialization
std::ifstream fis{<file>};
serialize::SimpleStreamDeserializer d{fis};
auto secondObject = serialize::deserialize<MyType>(d);
```

## Supported Types

- All standard C++ fundamental types (bool, char, float, etc.)
- Common STL container types (std::vector, std::map, etc.)
- Other common STL types (std::chrono::duration, std::atomic, std::optional, std::variant, etc.)
- Any aggregate type containing only supported types (see `examples/aggregate.cpp`)
- Any other type (via custom serialization/deserialization functions, see below)

## Custom Type Support

Additional types can be supported by one of the ways listed below.

The implementation for the serialization and deserialization do not have to match,
e.g. serialization can be supported via a member function and deserialization via a static class function.

### Member Function

For serialization, needs to provide a publicly accessible member function with following signature:

    <unused> Type::serialize(concept::Serializer&) const;

For deserialization, the required publicly accessible function has following signature:

    <unused> Type::deserialize(concept::Deserializer&);

An example type providing member function (de-)serialization can be seen in `examples/member.cpp`.

### Static Class Function

The signature for the publicly accessible serialization function is as follows:

    static <unused> Type::serialize(concept::Serializer&, const Type&);

The signature for the publicly accessible deserialization function is as follows:

    static <unused> Type::deserialize(concept::Deserializer&, Type&);

An example type providing static type function (de-)serialization can be seen in `examples/static.cpp`.

### Structured Bindings

Any type which can be assigned from/to a tuple of only (de-)serializable types can automatically be (de-)serialized, see `examples/aggregate.cpp`.

## Supported Serializers

- Simple (`simple.hpp`): Uses the underlying iostreams `read` and `write` functions.
- BitPacking (`bit_packing.hpp`): Compresses integral values with exponential Golomb.
- BytePacking (`byte_packing.hpp`): Uses a custom byte-based compression algorithm.
- TypeSafe (`type_safe.hpp`): Wrapper around other (de-)serializers adding and verifying type-information in the serialized stream (see `examples/type_safe.cpp`).

## Custom Serializers

Any type which adheres to the `serialize::Serializer` concept can be used as serializer.
A `Serializer` type needs to provide member `write(T)` functions accepting all fundamental C++ types and additionally a `flush()` function.

Adhering to the additional `serialize::ByteSerializer` concept can improve serialization performance for larger buffers.
To fulfill the `ByteSerializer` concept, an additional publicly accessible member function `write(size_t, std::span<std::byte>)` needs to be implemented.

Any type which adheres to the `serialize::Deserializer` concept can be used as deserializer.
A `Deserializer` type needs to implement publicly accessible `read(T&)` member functions accepting all fundamental C++ types.

See `examples/custom.cpp` for an example on how to implement custom (de-)serializers.
