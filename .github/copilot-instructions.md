# Project Guidelines

## Code Style
- **Rust**: Standard Rust conventions - snake_case for functions/variables, PascalCase for types/structs, 4-space indentation. Examples: [crates/core/src/codegen/mod.rs](crates/core/src/codegen/mod.rs), [crates/cli/src/main.rs](crates/cli/src/main.rs).
- **C++**: Modern C++23 with std::expected for error handling (aliased as ExpType), namespaces (e.g., JsonTypedefCodeGen), PascalCase classes, snake_case functions. Examples: [cpp/include/common.hpp](cpp/include/common.hpp), [cpp/include/json_data.hpp](cpp/include/json_data.hpp).

## Architecture
- **Core Components**: Rust workspace with core library ([crates/core](crates/core)) for AST/schema processing and codegen framework; target crates ([crates/target_*](crates/target_*)) for language-specific generation; CLI ([crates/cli](crates/cli)) for user interface.
- **C++ Implementation**: Separate high-performance JSON library ([cpp/](cpp/)) with SIMD/Nlohmann backends, providing alternative to Rust-generated code.
- **Data Flow**: Schemas parsed via jtd crate → AST conversion → target-specific code emission to output directories.
- **Design Rationale**: Modular targets enable easy addition of languages; C++ impl for performance-critical JSON ops; workspace structure isolates concerns.

## Build and Test
- **Rust**: `cargo build` (workspace), `cargo test --package=<crate>` (e.g., jtd_codegen_cli). CI: [test.yml](.github/workflows/test.yml).
- **C++**: `mkdir build && cd build && cmake .. -DENABLE_SIMD_JSON=On -DENABLE_NLOH_JSON=On && make`. Options: `-DBUILD_TEST=On` for tests. Nix shell for deps.

## Project Conventions
- **Naming**: Target crates prefixed `jtd_codegen_target_<lang>`; C++ classes use PascalCase, functions snake_case with ExpType returns.
- **Error Handling**: C++ uses std::expected (ExpType) instead of exceptions; Rust uses Result/thiserror.
- **Structure**: crates/ for Rust components, cpp/ for C++ impl; single-header generation via scripts.

## Integration Points
- **Dependencies**: jtd (0.2.1) for schema parsing; C++ links SIMD JSON, Nlohmann JSON, GoogleTest.
- **Cross-Component**: Core provides Target trait for codegen; CLI orchestrates targets; C++ exposes NAPI for Node.js integration.

## Security
- No authentication or network operations; codegen tool processes local schemas/files. No sensitive data handling.