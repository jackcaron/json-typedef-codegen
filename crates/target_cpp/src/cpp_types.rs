use jtd_codegen::target;

use crate::props::CppProps;
use crate::state::CppState;

type TypeIndex = usize;

fn deserialize_name(name: &str) -> String {
    format!("fromJson{}", name)
}

fn function_name(name: &str, full_ns: bool) -> String {
    format!(
        "ExpType<{}> {}(const {}Reader::JsonValue& value)",
        name,
        deserialize_name(name),
        if full_ns { "JsonTypedefCodeGen::" } else { "" }
    )
}

fn prototype_name(name: &str) -> String {
    format!("\nJsonTypedefCodeGen::{};", function_name(name, true))
}

#[derive(Debug, PartialEq)]
pub struct CppEnum {
    name: String,
    members: Vec<target::EnumMember>,
}

impl CppEnum {
    pub fn new(name: String, members: Vec<target::EnumMember>) -> CppEnum {
        CppEnum { name, members }
    }

    fn get_prefix() -> &'static str {
        "enum class"
    }

    pub fn get_internal_code() -> &'static str {
        r#"
ExpType<int> getEnumIndex(const Reader::JsonValue &value,
                          const std::span<const std::string_view> entries,
                          const std::string_view enumName) {
  if (const auto str = value.read_str(); str.has_value()) {
    const auto val = std::move(str.value());
    for (int index = 0; const auto entry : entries) {
      if (val == entry) {
        return index;
      }
      ++index;
    }
    const auto err = std::format("Invalid value \"{}\" for {}", val, enumName);
    return makeJsonError(JsonErrorTypes::Invalid, err);
  } else {
    return std::unexpected(str.error());
  }
}
"#
    }

    fn declare(&self) -> String {
        format!(
            "\n{} {} {{\n{}}};\n",
            CppEnum::get_prefix(),
            self.name,
            self.members
                .iter()
                .map(|m| format!("  {},\n", m.name))
                .collect::<String>()
        )
    }

    fn prototype(&self) -> String {
        prototype_name(&self.name)
    }

    fn create_json_str_items(&self) -> String {
        self.members
            .iter()
            .enumerate()
            .map(|(i, m)| {
                let prefix = if i == 0 {
                    ""
                } else if (i % 4) == 0 {
                    ",\n        "
                } else {
                    ", "
                };
                format!("{}\"{}\"sv", prefix, m.json_value)
            })
            .collect()
    }

    fn create_switch_clauses(&self) -> String {
        self.members
            .iter()
            .enumerate()
            .map(|(i, m)| format!("        case {}: return {}::{};\n", i, self.name, m.name))
            .collect()
    }

    fn define(&self) -> String {
        format!(
            r#"
{} {{
  constexpr std::array<std::string_view, {}> entries = {{{{
        {}
      }}}};
  return getEnumIndex(value, entries, "{}"sv)
    .transform([](auto idx) {{
      switch(idx) {{
        default:
{}     }}
    }});
}}

"#,
            function_name(&self.name, false),
            self.members.len(),
            self.create_json_str_items(),
            self.name,
            self.create_switch_clauses()
        )
    }
}

#[derive(Debug, PartialEq)]
pub struct CppStruct {
    name: String,
    fields: Vec<target::Field>,
    cpp_type_indices: Vec<TypeIndex>,
}

impl CppStruct {
    pub fn new(
        name: String,
        fields: Vec<target::Field>,
        cpp_type_indices: Vec<TypeIndex>,
    ) -> CppStruct {
        CppStruct {
            name,
            fields,
            cpp_type_indices,
        }
    }

    fn get_prefix() -> &'static str {
        "struct"
    }

    fn declare(&self, cpp_state: &CppState, cpp_props: &CppProps) -> String {
        let _ = cpp_props;
        let _ = cpp_state;
        format!(
            "\n{} {} {{\n{}}};\n",
            CppStruct::get_prefix(),
            self.name,
            (&self.fields)
                .iter()
                .map(|f| format!("  {} {};\n", f.type_, f.name))
                .collect::<String>()
        )
    }

    fn prototype(&self) -> String {
        prototype_name(&self.name)
    }

    fn define(&self) -> String {
        format!("")
    }
}

#[derive(Debug, PartialEq)]
pub struct CppAlias {
    name: String,
    sub_type: String,
}

impl CppAlias {
    pub fn new(name: String, sub_type: String) -> CppAlias {
        CppAlias { name, sub_type }
    }
    pub fn format(&self) -> String {
        format!("using {} = {}", self.name, self.sub_type)
    }

    fn get_internal_function_name(&self, cpp_state: &CppState) -> String {
        match cpp_state.get_index_from_name(&self.sub_type) {
            Some(sidx) => cpp_state
                .get_cpp_type_from_index(sidx)
                .get_internal_function_name(cpp_state),
            None => unreachable!(),
        }
    }

    fn get_internal_function(&self, cpp_state: &CppState, visited: &mut Vec<bool>) -> String {
        match cpp_state.get_index_from_name(&self.sub_type) {
            Some(sidx) => {
                if !visited[sidx] {
                    visited[sidx] = true;
                    let stype = cpp_state.get_cpp_type_from_index(sidx);
                    stype.get_internal_code(cpp_state, visited)
                } else {
                    String::new()
                }
            }
            None => unreachable!(),
        }
    }

    fn formatted_name(&self, cpp_state: &CppState) -> String {
        match cpp_state.get_index_from_name(&self.sub_type) {
            Some(sidx) => cpp_state
                .get_cpp_type_from_index(sidx)
                .get_formatted_name(cpp_state),
            None => unreachable!(),
        }
    }
}

#[derive(Debug, PartialEq)]
pub struct CppDiscriminator {
    name: String,
    variants: Vec<target::DiscriminatorVariantInfo>,
    tag_json_name: String,
    tag_field_name: String,
    cpp_type_indices: Vec<TypeIndex>,
}

impl CppDiscriminator {
    pub fn new(
        name: String,
        variants: Vec<target::DiscriminatorVariantInfo>,
        tag_json_name: String,
        tag_field_name: String,
        cpp_type_indices: Vec<TypeIndex>,
    ) -> CppDiscriminator {
        CppDiscriminator {
            name,
            variants,
            tag_json_name,
            tag_field_name,
            cpp_type_indices,
        }
    }
    fn get_prefix() -> &'static str {
        "struct"
    }

    fn get_variante_types(&self) -> String {
        let mut line_size = 4;
        self.variants
            .iter()
            .enumerate()
            .map(|(i, v)| {
                let prefix = if i == 0 {
                    "\n    "
                } else if (line_size + v.type_name.len()) > 78 {
                    // 78, to include ", "
                    line_size = 4; // reset with proper spaces
                    ",\n    "
                } else {
                    line_size += 2;
                    ", "
                };
                line_size += v.type_name.len();
                format!("{}{}", prefix, v.type_name)
            })
            .collect()
    }

    fn get_enum_type_items(&self) -> String {
        let cut = self.name.len();
        self.variants
            .iter()
            .enumerate()
            .map(|(i, v)| {
                let prefix = match i {
                    0 => "",
                    _ => ",\n    ",
                };
                format!("{}{} = {}", prefix, &v.type_name[cut..], i)
            })
            .collect()
    }

    fn get_constructors(&self) -> String {
        format!(
            r#"
  constexpr {}() = default;
  template<typename U> constexpr {}(U& t): m_value(t) {{}}"#,
            self.name, self.name
        )
    }

    fn declare(&self, cpp_state: &CppState, cpp_props: &CppProps) -> String {
        const GETTER_DEF: &'static str = r#"
  template<Types Tp> constexpr auto get()"#;
        const GETTER_INTERNAL: &'static str = r#"{
    namespace DT = JsonTypedefCodeGen::Data;
    if (auto ptr = std::get_if<size_t(Tp)>(&m_value); ptr != nullptr) {
      return DT::OptRefW(DT::RefW(*ptr));
    }
    return DT::OptRefW();
  }"#;
        let _ = cpp_props;
        let _ = cpp_state;
        let variant_types = self.get_variante_types();
        let enum_items = self.get_enum_type_items();
        let constructors = self.get_constructors();
        format!(
            r#"
class {} {{
private:
  using variant_t = std::variant<{}>;
  variant_t m_value;

public:
  enum class Types: size_t {{
    {}
  }};
{}

  template<typename U> constexpr {}& operator=(U& t) {{
    m_value = t;
    return *this;
  }}

  constexpr Types type() const {{ return Types(m_value.index()); }}
{} {}
{} const {}
}};
"#,
            self.name,
            variant_types,
            enum_items,
            constructors,
            self.name, // template assign
            GETTER_DEF,
            GETTER_INTERNAL,
            GETTER_DEF,
            GETTER_INTERNAL
        )
    }

    fn prototype(&self) -> String {
        prototype_name(&self.name)
    }

    fn define(&self) -> String {
        format!("")
    }
}

#[derive(Debug, PartialEq)]
pub struct CppDiscriminatorVariant {
    name: String,
    fields: Vec<target::Field>,
    tag_field_name: String,
    tag_json_name: String,
    tag_value: String,
    parent_name: String,
    cpp_type_indices: Vec<TypeIndex>,
}

impl CppDiscriminatorVariant {
    pub fn new(
        name: String,
        fields: Vec<target::Field>,
        tag_field_name: String,
        tag_json_name: String,
        tag_value: String,
        parent_name: String,
        cpp_type_indices: Vec<TypeIndex>,
    ) -> CppDiscriminatorVariant {
        CppDiscriminatorVariant {
            name,
            fields,
            tag_field_name,
            tag_json_name,
            tag_value,
            parent_name,
            cpp_type_indices,
        }
    }
    fn get_prefix() -> &'static str {
        "struct"
    }

    fn declare(&self, cpp_state: &CppState, cpp_props: &CppProps) -> String {
        let _ = cpp_props;
        let _ = cpp_state;
        format!(
            "\n{} {} {{\n{}}};\n",
            CppDiscriminatorVariant::get_prefix(),
            self.name,
            (&self.fields)
                .iter()
                .map(|f| format!("  {} {};\n", f.type_, f.name))
                .collect::<String>()
        )
    }

    fn prototype(&self) -> String {
        prototype_name(&self.name)
    }

    fn define(&self) -> String {
        format!("")
    }
}

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

    fn formatted_name(&self) -> &'static str {
        match self {
            Primitives::Bool => "Bool",
            Primitives::Int8 => "I8",
            Primitives::Uint8 => "U8",
            Primitives::Int16 => "I16",
            Primitives::Uint16 => "U16",
            Primitives::Int32 => "I32",
            Primitives::Uint32 => "U32",
            Primitives::Float32 => "Float",
            Primitives::Float64 => "Double",
            Primitives::String => "String",
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

    fn cpp_transform(&self) -> String {
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

    fn get_internal_function_name(&self) -> String {
        format!("fromJson{}", self.formatted_name())
    }

    fn get_internal_function(&self) -> String {
        format!(
            r#"
inline ExpType<{}> {}(const Reader::JsonValue& value) {{
  return {}{};
}}
"#,
            self.cpp_name(),
            self.get_internal_function_name(),
            self.read_primitive("value"),
            self.cpp_transform()
        )
    }

    // internal_visit(&self, cpp_state, visited_indices)
}

#[derive(Debug, PartialEq)]
pub struct CppArray(TypeIndex, String);

impl CppArray {
    pub fn new(idx: TypeIndex, name: String) -> CppArray {
        CppArray(idx, name)
    }

    fn formatted_name(&self, cpp_state: &CppState) -> String {
        format!(
            "ArrayOf{}",
            cpp_state
                .get_cpp_type_from_index(self.0)
                .get_formatted_name(cpp_state)
        )
    }

    fn get_internal_function_name(&self, cpp_state: &CppState) -> String {
        format!("fromJson{}", self.formatted_name(cpp_state))
    }

    fn get_internal_function(&self, cpp_state: &CppState) -> String {
        let sub_type_fn = cpp_state
            .get_cpp_type_from_index(self.0)
            .get_internal_function_name(cpp_state);
        format!(
            r#"
ExpType<{}> {}(const Reader::JsonValue &value) {{
  {} result;
  return Reader::json_array_for_each(
      value, [&result](const auto &item) {{
        if (auto exp_res = {}(item); exp_res.has_value()) {{
          result.emplace_back(std::move(exp_res.value()));
          return ExpType<void>();
        }}
        else {{
          std::unexpected(exp_res.error());
        }}
      }}).transform([&result]() {{ return result; }});
}}
"#,
            self.1,
            self.get_internal_function_name(cpp_state),
            self.1,
            sub_type_fn
        )
    }
}

#[derive(Debug, PartialEq)]
pub struct CppDict(Option<TypeIndex>);

impl CppDict {
    pub fn new(opt_idx: Option<TypeIndex>) -> CppDict {
        CppDict(opt_idx)
    }

    fn formatted_name(&self, cpp_state: &CppState) -> String {
        match &self.0 {
            Some(idx) => format!(
                "MapOf{}",
                cpp_state
                    .get_cpp_type_from_index(*idx)
                    .get_formatted_name(cpp_state)
            ),
            None => "Set".to_string(),
        }
    }

    fn get_internal_function_name(&self, cpp_state: &CppState) -> String {
        format!("fromJson{}", self.formatted_name(cpp_state))
    }

    fn get_internal_function(&self, cpp_state: &CppState) -> String {
        match &self.0 {
            Some(idx) => todo!(),
            None => String::new(), // handled by cpp_state
        }
    }

    pub fn get_set_internal_function(cpp_props: &CppProps) -> String {
        let stype = cpp_props.get_dictionary_info("").1;
        format!(
            r#"
ExpType<{}> fromJsonSet(const Reader::JsonValue &value) {{
  {} result;
  return makeJsonError(JsonErrorTypes::Unknown, "not ready yet"sv); // probably just reading the keys, ignore the data
}}
        "#,
            stype, stype
        )
    }
}

#[derive(Debug, PartialEq)]
pub struct CppNullable(TypeIndex);

impl CppNullable {
    pub fn new(idx: TypeIndex) -> CppNullable {
        CppNullable(idx)
    }

    fn formatted_name(&self, cpp_state: &CppState) -> String {
        format!(
            "Nullable{}",
            cpp_state
                .get_cpp_type_from_index(self.0)
                .get_formatted_name(cpp_state)
        )
    }

    fn get_internal_function_name(&self, cpp_state: &CppState) -> String {
        format!("fromJson{}", self.formatted_name(cpp_state))
    }

    fn get_internal_function(&self, cpp_state: &CppState) -> String {
        todo!()
    }
}

#[derive(Debug, PartialEq)]
pub enum CppTypes {
    Incomplete,
    Primitive(Primitives),
    Array(CppArray),
    Dictionary(CppDict),
    Nullable(CppNullable),
    Enum(CppEnum),
    Struct(CppStruct),
    Alias(CppAlias),
    Discriminator(CppDiscriminator),
    DiscriminatorVariant(CppDiscriminatorVariant),
}

impl CppTypes {
    pub fn needs_forward_declaration(&self) -> bool {
        match &self {
            Self::Enum(_)
            | Self::Struct(_)
            | Self::Discriminator(_)
            | Self::DiscriminatorVariant(_) => true,
            _ => false,
        }
    }

    pub fn type_prefix(&self) -> &'static str {
        match &self {
            Self::Enum(_) => CppEnum::get_prefix(),
            Self::Struct(_) => CppStruct::get_prefix(),
            Self::Discriminator(_) => CppDiscriminator::get_prefix(),
            Self::DiscriminatorVariant(_) => CppDiscriminatorVariant::get_prefix(),
            _ => panic!("type doesn't have a prefix"),
        }
    }

    pub fn declare(&self, cpp_state: &CppState, cpp_props: &CppProps) -> String {
        match &self {
            CppTypes::Enum(cpp_enum) => cpp_enum.declare(),
            CppTypes::Struct(cpp_struct) => cpp_struct.declare(cpp_state, cpp_props),
            CppTypes::Discriminator(cpp_dist) => cpp_dist.declare(cpp_state, cpp_props),
            CppTypes::DiscriminatorVariant(cpp_var) => cpp_var.declare(cpp_state, cpp_props),
            _ => String::new(),
        }
    }

    pub fn prototype(&self, cpp_state: &CppState, cpp_props: &CppProps) -> String {
        match &self {
            CppTypes::Enum(cpp_enum) => cpp_enum.prototype(),
            CppTypes::Struct(cpp_struct) => cpp_struct.prototype(),
            CppTypes::Discriminator(cpp_dist) => cpp_dist.prototype(),
            CppTypes::DiscriminatorVariant(cpp_var) => cpp_var.prototype(),
            _ => String::new(),
        }
    }

    pub fn define(&self, cpp_state: &CppState, cpp_props: &CppProps) -> String {
        match &self {
            CppTypes::Enum(cpp_enum) => cpp_enum.define(),
            CppTypes::Struct(cpp_struct) => cpp_struct.define(),
            CppTypes::Discriminator(cpp_dist) => cpp_dist.define(),
            CppTypes::DiscriminatorVariant(cpp_var) => cpp_var.define(),
            _ => String::new(),
        }
    }

    fn get_formatted_name(&self, cpp_state: &CppState) -> String {
        match self {
            CppTypes::Incomplete => unreachable!(),
            CppTypes::Primitive(p) => p.formatted_name().to_string(),
            CppTypes::Array(cpp_array) => cpp_array.formatted_name(cpp_state),
            CppTypes::Dictionary(cpp_dict) => cpp_dict.formatted_name(cpp_state),
            CppTypes::Nullable(cpp_null) => cpp_null.formatted_name(cpp_state),
            CppTypes::Enum(cpp_enum) => cpp_enum.name.clone(),
            CppTypes::Struct(cpp_struct) => cpp_struct.name.clone(),
            CppTypes::Alias(cpp_alias) => cpp_alias.formatted_name(cpp_state),
            CppTypes::Discriminator(cpp_discriminator) => cpp_discriminator.name.clone(),
            CppTypes::DiscriminatorVariant(cpp_discriminator_variant) => {
                cpp_discriminator_variant.name.clone()
            }
        }
    }

    fn get_internal_function_name(&self, cpp_state: &CppState) -> String {
        match self {
            CppTypes::Incomplete => unreachable!(),
            CppTypes::Primitive(p) => p.get_internal_function_name(),
            CppTypes::Array(cpp_array) => cpp_array.get_internal_function_name(cpp_state),
            CppTypes::Dictionary(cpp_dict) => cpp_dict.get_internal_function_name(cpp_state),
            CppTypes::Nullable(cpp_null) => cpp_null.get_internal_function_name(cpp_state),
            CppTypes::Enum(cpp_enum) => deserialize_name(&cpp_enum.name),
            CppTypes::Struct(cpp_struct) => deserialize_name(&cpp_struct.name),
            CppTypes::Alias(cpp_alias) => cpp_alias.get_internal_function_name(cpp_state),
            CppTypes::Discriminator(cpp_discriminator) => deserialize_name(&cpp_discriminator.name),
            CppTypes::DiscriminatorVariant(cpp_discriminator_variant) => {
                deserialize_name(&cpp_discriminator_variant.name)
            }
        }
    }

    pub fn get_internal_code(&self, cpp_state: &CppState, visited: &mut Vec<bool>) -> String {
        match self {
            CppTypes::Primitive(p) => p.get_internal_function(),
            CppTypes::Array(cpp_array) => cpp_array.get_internal_function(cpp_state),
            CppTypes::Dictionary(cpp_dict) => cpp_dict.get_internal_function(cpp_state),
            CppTypes::Nullable(cpp_null) => cpp_null.get_internal_function(cpp_state),
            CppTypes::Alias(cpp_alias) => cpp_alias.get_internal_function(cpp_state, visited),
            _ => String::new(),
        }
    }
}
