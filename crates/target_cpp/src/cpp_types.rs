use jtd_codegen::target;

use crate::props::CppProps;
use crate::state::CppState;

type TypeIndex = usize;

fn create_entry_array(entries: &str, size: usize) -> String {
    include_str!("./cpp_snippets/entries_array.cpp")
        .replace("$SIZE$", &size.to_string())
        .replace("$ENTRIES$", entries)
}

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

fn create_visited_array(sz: usize) -> String {
    let falses = (0..sz)
        .map(|i| {
            let prefix = if i == 0 { "" } else { ", " };
            format!("{}false", prefix)
        })
        .collect::<String>();
    format!(r#"std::array<bool, {}> visited = {{ {} }};"#, sz, falses)
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
        include_str!("./cpp_snippets/internal_code_enum_idx.cpp")
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
        create_entry_array(&items, self.members.len())
    }

    fn create_switch_clauses(&self) -> String {
        self.members
            .iter()
            .enumerate()
            .map(|(i, m)| {
                format!(
                    r#"            case {}: return Enum::{};
"#,
                    i, m.name
                )
            })
            .collect()
    }

    fn get_internal_code(&self, cpp_props: &CppProps) -> String {
        let fullname = cpp_props.get_namespaced_name(&self.name);
        let entries = self.create_entry_array();
        let clauses = self.create_switch_clauses();
        include_str!("./cpp_snippets/enum_from_json.cpp")
            .replace("$FULL_NAME$", &fullname)
            .replace("$ENTRIES$", &entries)
            .replace("$ENUM_NAME$", &self.name)
            .replace("$CLAUSES$", &clauses)
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

    fn declare(&self) -> String {
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
        create_entry_array(&items, self.fields.len())
    }

    fn create_switch_clauses(&self) -> String {
        self.fields
            .iter()
            .enumerate()
            .map(|(i, f)| {
                format!(
                    r#"
                case {}: return convert_and_set(result.{}, val);"#,
                    i, f.name
                )
            })
            .collect::<String>()
    }

    fn get_internal_code(&self, cpp_props: &CppProps) -> String {
        let fullname = cpp_props.get_namespaced_name(&self.name);
        let clauses = self.create_switch_clauses();
        let entries = self.create_entry_array();
        let visited = create_visited_array(self.fields.len());
        include_str!("./cpp_snippets/struct_from_json.cpp")
            .replace("$FULL_NAME$", &fullname)
            .replace("$ENTRIES$", &entries)
            .replace("$VISITED$", &visited)
            .replace("$STRUCT_NAME$", &self.name)
            .replace("$CLAUSES$", &clauses)
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
        format!("using {} = {};\n", self.name, self.sub_type)
    }
    pub fn sub_matches(&self, sub_type: &str) -> bool {
        self.sub_type == sub_type
    }
    pub fn get_name<'a>(&'a self) -> &'a str {
        &self.name
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

    fn declare(&self) -> String {
        let variant_types = self.get_variante_types();
        let enum_items = self.get_enum_type_items();
        include_str!("./cpp_snippets/disc_declare.cpp")
            .replace("$DISCRIMINATOR_NAME$", &self.name)
            .replace("$VARIANT_NAMES$", &variant_types)
            .replace("$TYPE_VALUES$", &enum_items)
    }

    fn prototype(&self) -> String {
        prototype_name(&self.name)
    }

    fn create_entry_array(&self) -> String {
        let items = self
            .variants
            .iter()
            .enumerate()
            .map(|(i, v)| {
                let prefix = if i == 0 {
                    ""
                } else if (i % 4) == 0 {
                    ",\n          "
                } else {
                    ", "
                };
                format!("{}\"{}\"sv", prefix, v.tag_value)
            })
            .collect::<String>();
        create_entry_array(&items, self.variants.len())
    }

    fn create_switch_clauses(&self) -> String {
        self.variants
            .iter()
            .enumerate()
            .map(|(i, v)| {
                format!(
                    r#"
                case {}: return FromJson<{}>::convert(object).transform(toDisc);"#,
                    i, v.type_name
                )
            })
            .collect::<String>()
    }

    fn get_internal_code(&self, cpp_props: &CppProps) -> String {
        let fullname = cpp_props.get_namespaced_name(&self.name);
        let entries = self.create_entry_array();
        let clauses = self.create_switch_clauses();
        include_str!("./cpp_snippets/disc_from_json.cpp")
            .replace("$FULL_NAME$", &fullname)
            .replace("$ENTRIES$", &entries)
            .replace("$TAG_KEY$", &self.tag_field_name)
            .replace("$DISC_NAME$", &self.name)
            .replace("$CLAUSES$", &clauses)
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

    fn declare(&self) -> String {
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

    fn create_entry_array(&self) -> String {
        let items = self
            .fields
            .iter()
            .enumerate()
            .map(|(i, f)| {
                let prefix = if i == 0 {
                    ""
                } else if (i % 5) == 0 {
                    ",\n        "
                } else {
                    ", "
                };
                format!("{}\"{}\"sv", prefix, f.json_name)
            })
            .collect::<String>();
        format!(
            r#"static constexpr std::array<std::string_view, {}> entries = {{{{
          "{}"sv, {}
        }}}};"#,
            self.fields.len() + 1,
            self.tag_json_name,
            items
        )
    }

    fn create_switch_clauses(&self) -> String {
        self.fields
            .iter()
            .enumerate()
            .map(|(i, f)| {
                format!(
                    r#"
                  case {}: convert_and_set(result.{}, val); break;"#,
                    i + 1,
                    f.name
                )
            })
            .collect::<String>()
    }

    fn get_internal_code(&self, cpp_props: &CppProps) -> String {
        let fullname = cpp_props.get_namespaced_name(&self.name);
        let entries = self.create_entry_array();
        let visited = create_visited_array(self.fields.len() + 1);
        let clauses = self.create_switch_clauses();
        include_str!("./cpp_snippets/vary_from_json.cpp")
            .replace("$FULL_NAME$", &fullname)
            .replace("$ENTRIES$", &entries)
            .replace("$VISITED$", &visited)
            .replace("$VARY_NAME$", &self.name)
            .replace("$CLAUSES$", &clauses)
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

    fn get_internal_function(&self) -> String {
        let typename = self.cpp_name();
        let read_prim = self.read_primitive("value");
        let read_xform = self.cpp_reader_transform();
        let data_xform = self.cpp_data_transform();
        let exp_type = self.cpp_name();
        include_str!("./cpp_snippets/prim_from_json.cpp")
            .replace("$FULL_NAME$", &typename)
            .replace("$READ_PRIM_VALUE$", &read_prim)
            .replace("$READER_XFORM$", &read_xform)
            .replace("$EXP_TYPE$", &exp_type)
            .replace("$DATA_XFORM$", &data_xform)
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
pub struct CppDict(Option<TypeIndex>, String);

impl CppDict {
    pub fn new(opt_idx: Option<TypeIndex>, name: String) -> CppDict {
        CppDict(opt_idx, name)
    }

    pub fn get_set_internal_function(cpp_props: &CppProps) -> String {
        let settype = cpp_props.get_dictionary_info("").1;
        // TODO: use a simple "using" statement to define the SetType then full static code
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
pub struct CppNullable(TypeIndex, String);

impl CppNullable {
    pub fn new(idx: TypeIndex, name: String) -> CppNullable {
        CppNullable(idx, name)
    }

    fn function_name(&self) -> String {
        format!("fromJsonNullable{}", self.1)
    }

    fn get_full_name(&self, cpp_state: &CppState) -> String {
        let uptr_name = format!("std::unique_ptr<{}>", self.1);
        cpp_state.get_aliased_name(&uptr_name).to_string()
    }

    fn prototype(&self, cpp_state: &CppState) -> String {
        let full_name = self.get_full_name(cpp_state);
        format!(
            r#"
JsonTypedefCodeGen::ExpType<{}> {}(const JsonTypedefCodeGen::Reader::JsonValue& value);"#,
            full_name,
            self.function_name()
        )
    }

    fn define(&self, cpp_state: &CppState) -> String {
        let full_name = self.get_full_name(cpp_state);
        format!(
            r#"
ExpType<{}> {}(const Reader::JsonValue& value) {{
  return FromJson<{}>::convert(value);
}}
"#,
            full_name,
            self.function_name(),
            full_name
        )
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

    pub fn declare(&self, _cpp_state: &CppState, _cpp_props: &CppProps) -> Option<String> {
        match &self {
            CppTypes::Enum(_enum) => Some(_enum.declare()),
            CppTypes::Struct(_struct) => Some(_struct.declare()),
            CppTypes::Discriminator(disc) => Some(disc.declare()),
            CppTypes::DiscriminatorVariant(vary) => Some(vary.declare()),
            _ => None,
        }
    }

    pub fn prototype(&self, cpp_state: &CppState, _cpp_props: &CppProps) -> Option<String> {
        match &self {
            CppTypes::Enum(_enum) => Some(_enum.prototype()),
            CppTypes::Struct(_struct) => Some(_struct.prototype()),
            CppTypes::Discriminator(disc) => Some(disc.prototype()),
            CppTypes::DiscriminatorVariant(vary) => Some(vary.prototype()),
            CppTypes::Nullable(null) => Some(null.prototype(cpp_state)),
            _ => None,
        }
    }

    pub fn define(&self, cpp_state: &CppState, _cpp_props: &CppProps) -> Option<String> {
        match &self {
            CppTypes::Enum(_enum) => Some(get_complete_definition(&_enum.name)),
            CppTypes::Struct(_struct) => Some(get_complete_definition(&_struct.name)),
            CppTypes::Discriminator(disc) => Some(get_complete_definition(&disc.name)),
            CppTypes::DiscriminatorVariant(vary) => Some(get_complete_definition(&vary.name)),
            CppTypes::Nullable(null) => Some(null.define(cpp_state)),
            _ => None,
        }
    }

    pub fn get_internal_code(&self, _cpp_state: &CppState, cpp_props: &CppProps) -> Option<String> {
        match self {
            CppTypes::Primitive(p) => Some(p.get_internal_function()),
            CppTypes::Enum(_enum) => Some(_enum.get_internal_code(cpp_props)),
            CppTypes::Struct(_struct) => Some(_struct.get_internal_code(cpp_props)),
            CppTypes::Discriminator(disc) => Some(disc.get_internal_code(cpp_props)),
            CppTypes::DiscriminatorVariant(vary) => Some(vary.get_internal_code(cpp_props)),
            _ => None,
        }
    }
}
