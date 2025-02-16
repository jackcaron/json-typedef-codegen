use crate::cpp_snippets::INTERNAL_CODE_PRIM;

#[derive(Debug, PartialEq)]
pub enum Primitives {
    Bool,
    Int8,
    Uint8,
    Int16,
    Uint16,
    Int32,
    Uint32,
    Float32,
    Float64,
    String,
}

impl Primitives {
    pub fn cpp_name(&self) -> &'static str {
        match self {
            Primitives::Bool => "bool",
            Primitives::Int8 => "int8_t",
            Primitives::Uint8 => "uint8_t",
            Primitives::Int16 => "int16_t",
            Primitives::Uint16 => "uint16_t",
            Primitives::Int32 => "int32_t",
            Primitives::Uint32 => "uint32_t",
            Primitives::Float32 => "float",
            Primitives::Float64 => "double",
            Primitives::String => "std::string",
        }
    }

    fn read_primitive(&self, from: &str) -> String {
        let fn_name = match self {
            Primitives::Bool => "bool",
            Primitives::Int8 | Primitives::Int16 | Primitives::Int32 => "i64",
            Primitives::Uint8 | Primitives::Uint16 | Primitives::Uint32 => "u64",
            Primitives::Float32 | Primitives::Float64 => "double",
            Primitives::String => "str",
        };
        format!("{}.read_{}()", from, fn_name)
    }

    fn cpp_reader_transform(&self) -> String {
        match self {
            // no need to transform the value
            Primitives::Bool | Primitives::Float64 | Primitives::String => String::new(),
            _ => format!(
                r#".transform([](const auto val) {{
  return ({})val;
}})"#,
                self.cpp_name()
            ),
        }
    }

    fn cpp_data_transform(&self) -> String {
        match self {
            Primitives::String => r#"
        .transform([](auto v) { return std::string(v); })"#
                .to_string(),
            _ => String::new(),
        }
    }

    pub fn get_internal_function(&self) -> String {
        let typename = self.cpp_name();
        let read_prim = self.read_primitive("value");
        let read_xform = self.cpp_reader_transform();
        let data_xform = self.cpp_data_transform();
        let exp_type = self.cpp_name();
        INTERNAL_CODE_PRIM
            .replace("$FULL_NAME$", &typename)
            .replace("$READ_PRIM_VALUE$", &read_prim)
            .replace("$READER_XFORM$", &read_xform)
            .replace("$EXP_TYPE$", &exp_type)
            .replace("$DATA_XFORM$", &data_xform)
    }

    // internal_visit(&self, cpp_state, visited_indices)
}
