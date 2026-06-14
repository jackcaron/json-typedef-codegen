# Claude helper for json-typedef-codegen

Purpose

- Provide concise, actionable guidance for AI assistants (Claude) working in this repository.
- **For C++ development:** See [cpp/claude.md](cpp/claude.md) for detailed C++ library guidance, build options, and architecture.
- Respect `.gitignore`: avoid editing or recommending changes to files/dirs listed in the project's `.gitignore` (generated build artifacts, node_modules, single-header outputs, etc.).

- Do not create new files in the repository without asking the user for permission first.

Quick tasks

- Build Rust workspace: `cargo build` (root)
- Run Rust tests: `cargo test --package=<crate>` (e.g., `jtd_codegen_target_typescript`)
- Build C++: `mkdir -p cpp/build && cd cpp/build && cmake .. [options] && make`

Build & Test (examples)

```bash
# Rust (workspace)
cargo build
cargo test --package=jtd_codegen_target_typescript

# C++ (from repo root)
cd cpp
mkdir -p build && cd build
cmake .. -DENABLE_SIMD_JSON=On -DENABLE_NLOH_JSON=On
make
ctest
```

Key locations (link-first)

- **Overview:** [README.md](README.md)
- **Core library:** [crates/core/](crates/core/)
- **CLI:** [crates/cli/](crates/cli/)
- **Targets (generators):** [crates/target\_\*](crates/)
- **Tests & schemas:** [crates/test/](crates/test/)
- **C++ implementation:** [cpp/](cpp/) — see [cpp/claude.md](cpp/claude.md) for detailed C++ guidance

Conventions (short)

- Rust: snake_case functions/vars, PascalCase types, 4-space indent, use `Result<T, E>` + `thiserror` for errors. See [rustfmt.toml](rustfmt.toml) for formatting rules.
- C++: C++23, PascalCase classes, snake_case functions, `ExpType<T>` (std::expected) for errors, namespace `JsonTypedefCodeGen`.
- Naming: generator crates use `jtd_codegen_target_<lang>`.

Common pitfalls

- Do not modify or commit ignored/generated artifacts listed in the repo `.gitignore` files. Common ignored paths include:
  - `/target`
  - `cpp/build*`, `lib/*`, `bin/*`, `external/*`
  - `node_modules/*`, `package-lock.json`
  - `single-header/*` (generated single-header outputs)
  - `tests/gtests/generated/*`
  - `notes.txt` and editor-specific files like `.vscode/c_cpp_properties.json`
- Clean `target/` and `cpp/build/` when changing toolchain or templates.
- Generated headers live under `cpp/build/generated-headers/` during CMake runs — don't commit them.
- CMake: enable at least one backend flag (`ENABLE_SIMD_JSON` or `ENABLE_NLOH_JSON`) for C++ builds.
- NAPI builds require matching Node.js toolchain and may need cmake-js or npm scripts in `cpp/package.json`.

How to generate code (examples)

```bash
# Single target
./jtd-codegen example.jtd.json --typescript-out ./gen/ts

# Multiple targets
./jtd-codegen example.jtd.json --rust-out ./gen/rust --cpp-out ./gen/cpp
```

Why this file helps

- Gives a short, link-focused orientation so Claude agents can find build/test commands, key modules, and constraints quickly without copying large docs.

If you want, I can: create a companion `AGENTS.md` with role-played prompts, add language-specific skills, or extract more granular developer rules.
