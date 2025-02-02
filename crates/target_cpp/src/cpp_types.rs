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

fn get_complete_definition(name: &str) -> String {
    format!(
        r#"
{} {{
  return FromJson<{}>::convert(value);
}}
"#,
        function_name(name, false),
        name
    )
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

    pub fn get_enum_index_code() -> &'static str {
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
      const auto err = std::format("Invalid value \"{}\" for {}",
                                   value, enumName);
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

    fn create_entry_array(&self) -> String {
        let items = self
            .members
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
            .collect::<String>();
        format!(
            r#"constexpr std::array<std::string_view, {}> entries = {{{{
          {}
        }}}};"#,
            self.members.len(),
            items
        )
    }

    fn create_switch_clauses(&self) -> String {
        self.members
            .iter()
            .enumerate()
            .map(|(i, m)| {
                format!(
                    "            case {}: return {}::{};\n",
                    i, self.name, m.name
                )
            })
            .collect()
    }

    fn get_internal_code(&self, cpp_props: &CppProps) -> String {
        let fullname = cpp_props.get_namespaced_name(&self.name);
        format!(
            r#"
  template<>
  struct FromJson<{}> {{
    static ExpType<{}> convert(const Reader::JsonValue &value) {{
      using {};
      {}
      return getEnumIndex(value, entries, "{}"sv)
        .transform([](auto idx) {{
          switch(idx) {{
            default:
{}        }}
      }});
    }}
  }};
"#,
            fullname,
            fullname,
            fullname,
            self.create_entry_array(),
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

    fn create_entry_array(&self) -> String {
        let items = self
            .fields
            .iter()
            .enumerate()
            .map(|(i, f)| {
                let prefix = if i == 0 {
                    ""
                } else if (i % 4) == 0 {
                    ",\n        "
                } else {
                    ", "
                };
                format!("{}\"{}\"sv", prefix, f.json_name)
            })
            .collect::<String>();
        format!(
            r#"constexpr std::array<std::string_view, {}> entries = {{{{
          {}
        }}}};"#,
            self.fields.len(),
            items
        )
    }

    fn create_visited_array(&self) -> String {
        let falses = (0..self.fields.len())
            .map(|i| {
                let prefix = if i == 0 { "" } else { ", " };
                format!("{}false", prefix)
            })
            .collect::<String>();
        format!(
            r#"std::array<bool, {}> visited = {{ {} }};"#,
            self.fields.len(),
            falses
        )
    }

    fn create_switch_clauses(&self) -> String {
        self.fields
            .iter()
            .enumerate()
            .map(|(i, f)| {
                format!(
                    r#"
              case {}: return convertAndSet(result.{}, val);"#,
                    i, f.name
                )
            })
            .collect::<String>()
    }

    fn get_internal_code(&self, cpp_props: &CppProps) -> String {
        let fullname = cpp_props.get_namespaced_name(&self.name);
        let clauses = self.create_switch_clauses();
        format!(
            r#"
  template<>
  struct FromJson<{}> {{
    static ExpType<{}> convert(const Reader::JsonValue &value) {{
      {}

      {}
      {} result;

      auto feach = Reader::json_object_for_each(
        value,
        [&](const auto key, const auto &val) {{
          return flatten_expected(
            getValueIndex(key, entries, "{}"sv)
            .transform([&](const int idx) -> ExpType<void> {{
              if (visited[idx]) {{
                const auto err = std::format("Duplicated key {{}}", key);
                return makeJsonError(JsonErrorTypes::Invalid, err);
              }}
              visited[idx] = true;

              switch (idx) {{
              default:{}
              }}
            }}));
        }});
      return feach.transform([res = std::move(result)]() {{ return res; }});
    }}
  }};
"#,
            fullname,
            fullname,
            self.create_entry_array(),
            self.create_visited_array(),
            fullname,
            self.name,
            clauses
        )
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

    fn get_internal_code(&self, cpp_props: &CppProps) -> String {
        let fullname = cpp_props.get_namespaced_name(&self.name);
        format!(
            r#"
  template<>
  struct FromJson<{}> {{
    static ExpType<{}> convert(const Reader::JsonValue &value) {{
      return makeJsonError(JsonErrorTypes::Unknown, "not implemented"sv);
    }}
  }};
"#,
            fullname, fullname
        )
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

    fn get_internal_code(&self, cpp_props: &CppProps) -> String {
        let fullname = cpp_props.get_namespaced_name(&self.name);
        format!(
            r#"
  template<>
  struct FromJson<{}> {{
    static ExpType<{}> convert(const Reader::JsonValue &value) {{
      return makeJsonError(JsonErrorTypes::Unknown, "not implemented"sv);
    }}
  }};
"#,
            fullname, fullname
        )
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

    fn get_internal_function(&self) -> String {
        let typename = self.cpp_name();
        format!(
            r#"
  template<>
  struct FromJson<{}> {{
    static ExpType<{}> convert(const Reader::JsonValue& value) {{
      return {}{};
    }}
  }};
"#,
            typename,
            typename,
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
}

#[derive(Debug, PartialEq)]
pub struct CppDict(Option<TypeIndex>);

impl CppDict {
    pub fn new(opt_idx: Option<TypeIndex>) -> CppDict {
        CppDict(opt_idx)
    }

    pub fn get_set_internal_function(cpp_props: &CppProps) -> String {
        let settype = cpp_props.get_dictionary_info("").1;
        format!(
            r#"
  template<> struct FromJson<{}> {{
    static ExpType<{}> convert(const Reader::JsonValue &value) {{
      return makeJsonError(JsonErrorTypes::Unknown, "not ready yet"sv); // probably just reading the keys, ignore the data
    }}
  }};
"#,
            settype, settype
        )
    }
}

#[derive(Debug, PartialEq)]
pub struct CppNullable(TypeIndex);

impl CppNullable {
    pub fn new(idx: TypeIndex) -> CppNullable {
        CppNullable(idx)
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
            CppTypes::Enum(cpp_enum) => get_complete_definition(&cpp_enum.name),
            CppTypes::Struct(cpp_struct) => get_complete_definition(&cpp_struct.name),
            CppTypes::Discriminator(cpp_discriminator) => {
                get_complete_definition(&cpp_discriminator.name)
            }
            CppTypes::DiscriminatorVariant(cpp_discriminator_variant) => {
                get_complete_definition(&cpp_discriminator_variant.name)
            }
            _ => String::new(),
        }
    }

    pub fn get_internal_code(&self, cpp_state: &CppState, cpp_props: &CppProps) -> String {
        match self {
            CppTypes::Primitive(p) => p.get_internal_function(),
            CppTypes::Enum(cpp_enum) => cpp_enum.get_internal_code(cpp_props),
            CppTypes::Struct(cpp_struct) => cpp_struct.get_internal_code(cpp_props),
            CppTypes::Discriminator(cpp_discriminator) => {
                cpp_discriminator.get_internal_code(cpp_props)
            }
            CppTypes::DiscriminatorVariant(cpp_discriminator_variant) => {
                cpp_discriminator_variant.get_internal_code(cpp_props)
            }
            // Nullable!!!
            _ => String::new(),
        }
    }
}
