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
}
