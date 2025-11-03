# C++ Wrappers

## Dependencies

- C++23 compiler
  - only tested with GCC 13
- CMake (_CMake-GUI for easier configuration_)
- Node.js/NPM (_for the NAPI wrapper only_)
- [SIMD Json](https://github.com/simdjson/simdjson)
- [Nlohmann Json](https://github.com/nlohmann/json)
- [Google Test](https://google.github.io/googletest/)

## Building

### C++ static library

Add wrapped libraries, at least one is needed:

- _SIMD Json_ must be under the `simd_json` directory, using the single header file version.
- _Nlohmann Json_ must be under the `nlohmann` directory, using the single include files.

For a basic build, with both _SIMD Json_ and _Nlohmann Json_, run:

```bash
mkdir -p build
cd build
cmake .. -DENABLE_SIMD_JSON=On -DENABLE_NLOH_JSON=On
```

Extra options:

- `-DBUILD_WRITER=On` to build a serializer from the any of the external library
- `-DBUILD_TEST=Off` to disable the tests
- `-DBUILD_READER=Off` to disable the deserializer

Built libraries are located in `lib/<CMAKE_BUILD_TYPE>`.

### Nix

Using the Nix shell is a simple way to install the tested version of `gcc`, `cmake`, `simdjson`, and `nlohmann_json`.
Once in the shell, it's the same building step from `C++ static library`.

### Node.js NAPI wrapper

In the `cpp` directory:

```bash
npm install
npm run build-writer
```

This creates the static library `libjson_wrapper_napi_static.a`.

Extra scripts/options:

- `build` to only include the deserializer part.
- `test` to run some quick tests.
- `-dev` is there to build in debug mode.

## Example

Using the same example as in the base README (_`example.jtd.json`_):

```json
{
  "properties": {
    "name": { "type": "string" },
    "isAdmin": { "type": "boolean" },
    "favoriteNumbers": { "elements": { "type": "float64" } }
  }
}
```

and this C++ configuration file (_`cpp_config.json`_):

```json
{
  "namespace": "Test",
  "guard": "pragma",
  "output": "both"
}
```

You can generate this C++ interface in a header file `example.hpp`:

```cpp
#pragma once

#include <json_data.hpp>
#include <json_reader.hpp>
#include <json_writer.hpp>
#include <string>
#include <vector>

namespace Test {

// forward declarations
struct Example;

// declarations
struct Example {
  std::vector<double> favorite_numbers;
  bool is_admin;
  std::string name;
};

// prototypes
JsonTypedefCodeGen::ExpType<Example> deserialize_Example(const JsonTypedefCodeGen::Reader::JsonValue& value);
JsonTypedefCodeGen::ExpType<void> serialize_Example(JsonTypedefCodeGen::Writer::Serializer& serializer, const Example& value);

} // namespace Test
```

The CLI to create this C++ interface in the current directory is:

```bash
./jtd-codegen example.jtd.json --cpp-out . --cpp-props cpp_config.json
```

## C++ Configuration file

Because there are multiple ways to write C++, a configuration file is necessary to support them all instead of relying on more arguments on the CLI.
This configuration file is optional, in that case, the default values of the various properties is used.

### Properties

#### guard (optional)

This is to control the header guards.
There's two values:

- `"pragma"` which puts a `#pragma once` at the top of every header file
- `{"name": "GUARD_NAME"}` which creates a header guard like this, with the matching `#endif` at the bottom:

```cpp
#ifndef GUARD_NAME
#define GUARD_NAME

// ...

#endif
```

#### include_data, include_reader, include_writer

How header files should be included in the generated code. `include_data` refers to `"json_data.hpp"`, `include_reader` to `"json_reader.hpp`, and `include_writer` to `"json_writer.hpp"`.

Here are the possible values:

- `"ignore"`: don't include them
- `"local"`: use `#include "header.hpp"` (relative to the header file)
- `"system"` (_default_): use `#include <header.hpp>`
- `{"path": "where/it/is"}`: use `where/it/is` as a prefix to the file name, `#include "where/it/is/header.hpp`

#### namespace (optional)

Wraps the generated code in a `namespace`, see example above with the namespace `Test`.

#### output

Which operations should be generated:

- `"serialize"` - generate serialization code only
- `"deserialize"` - generate deserialization code only
- `"both"` (_default_) - generate both serialization and deserialization code

## Future Work

- [ ] Single header file
- [ ] C++ Modules
- [ ] CMake function to help compile a list of `*.jtd.json` files
- [ ] Include comments from `*.jtd.json` files
- [ ] Refine the quality of the generated C++ code
