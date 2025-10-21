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

- default directory CMake looks into is `./external`, but can be configured to look in other locations.
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

## Future Work

- [ ] Modules
