use jtd_codegen::target;
use jtd_codegen::Error as JtdError;

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

impl TryFrom<&target::Expr> for Primitives {
    type Error = JtdError;

    fn try_from(value: &target::Expr) -> Result<Self, Self::Error> {
        match value {
            target::Expr::Boolean => Ok(Primitives::Bool),
            target::Expr::Int8 => Ok(Primitives::Int8),
            target::Expr::Uint8 => Ok(Primitives::Uint8),
            target::Expr::Int16 => Ok(Primitives::Int16),
            target::Expr::Uint16 => Ok(Primitives::Uint16),
            target::Expr::Int32 => Ok(Primitives::Int32),
            target::Expr::Uint32 => Ok(Primitives::Uint32),
            target::Expr::Float32 => Ok(Primitives::Float32),
            target::Expr::Float64 => Ok(Primitives::Float64),
            _ => Err(JtdError::Conversion(
                "cannot create primitive from expression".into(),
            )),
        }
    }
}
