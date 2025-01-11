use jtd_codegen::target;
use jtd_codegen::target::metadata::Metadata;
use std::collections::{BTreeMap, BTreeSet};

use crate::props::CppProps;

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
    fn get_prefix() -> &'static str {
        "enum class"
    }

    fn get_internal_code() -> &'static str {
        r#"using namespace std::string_view_literals;

namespace {

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
    const auto err =
        std::format("Invalid value \"{}\" for enum {}", val, enumName);
    return makeJsonError(JsonErrorTypes::Invalid, err);
  } else {
    return std::unexpected(str.error());
  }
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

    fn create_json_str_array(&self) -> String {
        let items = self
            .members
            .iter()
            .enumerate()
            .map(|(i, m)| {
                let prefix = if i == 0 { "" } else { ", " };
                format!("{}\n        \"{}\"sv", prefix, m.json_value)
            })
            .collect::<String>();
        format!(
            "constexpr std::array<std::string_view, {}> entries = {{{{ {}\n    }}}}",
            self.members.len(),
            items
        )
    }

    fn create_switch(&self) -> String {
        let clauses = self
            .members
            .iter()
            .enumerate()
            .map(|(i, m)| format!("        case {}: return {}::{};\n", i, self.name, m.name))
            .collect::<String>();
        format!(
            r#"switch(idx) {{
        default:
{}      }}"#,
            clauses
        )
    }

    fn define(&self) -> String {
        format!(
            r#"
{} {{
  {};
  return getEnumIndex(value, entries, "{}"sv)
    .transform([](auto idx) {{
      {}
    }});
}}

"#,
            function_name(&self.name, false),
            self.create_json_str_array(),
            self.name,
            self.create_switch()
        )
    }
}

#[derive(Debug, PartialEq)]
pub struct CppStruct {
    name: String,
    fields: Vec<target::Field>,
    cpp_type_indices: Vec<usize>,
}

impl CppStruct {
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

#[derive(Debug, PartialEq)]
pub struct CppDiscriminator {
    name: String,
    variants: Vec<target::DiscriminatorVariantInfo>,
    tag_json_name: String,
    tag_field_name: String,
    cpp_type_indices: Vec<usize>,
}

impl CppDiscriminator {
    fn get_prefix() -> &'static str {
        "struct"
    }

    fn declare(&self, cpp_state: &CppState, cpp_props: &CppProps) -> String {
        let _ = cpp_props;
        let _ = cpp_state;
        format!(
            "\n{} {} {{\n{}}};\n",
            CppDiscriminator::get_prefix(),
            self.name,
            "  // TODO discriminator\n"
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
    cpp_type_indices: Vec<usize>,
}

impl CppDiscriminatorVariant {
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
pub enum CppTypes {
    Incomplete,
    Primitive(String),
    Array(usize),
    Dictionary(Option<usize>),
    Nullable(usize),
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

    fn declare(&self, cpp_state: &CppState, cpp_props: &CppProps) -> String {
        match &self {
            CppTypes::Enum(cpp_enum) => cpp_enum.declare(),
            CppTypes::Struct(cpp_struct) => cpp_struct.declare(cpp_state, cpp_props),
            CppTypes::Discriminator(cpp_dist) => cpp_dist.declare(cpp_state, cpp_props),
            CppTypes::DiscriminatorVariant(cpp_var) => cpp_var.declare(cpp_state, cpp_props),
            _ => String::new(),
        }
    }

    fn prototype(&self, cpp_state: &CppState, cpp_props: &CppProps) -> String {
        match &self {
            CppTypes::Enum(cpp_enum) => cpp_enum.prototype(),
            CppTypes::Struct(cpp_struct) => cpp_struct.prototype(),
            CppTypes::Discriminator(cpp_dist) => cpp_dist.prototype(),
            CppTypes::DiscriminatorVariant(cpp_var) => cpp_var.prototype(),
            _ => String::new(),
        }
    }

    fn define(&self, cpp_state: &CppState, cpp_props: &CppProps) -> String {
        match &self {
            CppTypes::Enum(cpp_enum) => cpp_enum.define(),
            CppTypes::Struct(cpp_struct) => cpp_struct.define(),
            CppTypes::Discriminator(cpp_dist) => cpp_dist.define(),
            CppTypes::DiscriminatorVariant(cpp_var) => cpp_var.define(),
            _ => String::new(),
        }
    }
}

#[derive(Default)]
pub struct CppState {
    // - header files to include, ex.: optional, vector, string, ...
    include_files: BTreeSet<String>,
    src_include_files: BTreeSet<String>, // header needed for the source file to work

    metadatas: Vec<Metadata>,
    names: Vec<String>,
    cpp_types: Vec<CppTypes>,
    cpp_type_indices: BTreeMap<String, usize>, // type name to index in "cpp_types"

    requires_enum_internal_code: bool,
}

impl CppState {
    pub fn write_include_files(&self, cpp_props: &CppProps) -> String {
        cpp_props.get_codegen_includes()
            + &((&self.include_files)
                .iter()
                .map(|h| format!("#include {}\n", h))
                .collect::<String>())
    }

    pub fn write_src_include_files(&self) -> String {
        // but exclude files already included in the header
        self.src_include_files
            .iter()
            .filter_map(|h| {
                if self.include_files.contains(h) {
                    None
                } else {
                    Some(format!("#include {}\n", h))
                }
            })
            .collect::<String>()
    }

    pub fn write_internal_code(&self) -> String {
        let mut intern_code = "using namespace JsonTypedefCodeGen;\n".to_string();
        if self.requires_enum_internal_code {
            intern_code = intern_code + CppEnum::get_internal_code();
        }
        intern_code
    }

    pub fn write_forward_declarations(&self) -> String {
        if self.cpp_types.len() <= 1 {
            return "".to_string();
        }

        const PRE_DECL: [&str; 2] = ["\n// forward declarations\n", ""];
        let mut first = true;
        (&self.cpp_types)
            .iter()
            .enumerate()
            .filter_map(|it| {
                if it.1.needs_forward_declaration() {
                    let n = &self.names[it.0];
                    let pre = it.1.type_prefix();
                    let idx: usize = (!first) as usize;
                    first = false;
                    Some(format!("{}{} {};\n", PRE_DECL[idx], pre, n))
                } else {
                    None
                }
            })
            .collect()
    }

    pub fn write_alias(&self) -> String {
        const PRE_ALIAS: [&str; 2] = ["\n// aliases\n", ""];
        let mut first = true;
        (&self.cpp_types)
            .iter()
            .filter_map(|t| match t {
                CppTypes::Alias(a) => {
                    let idx = (!first) as usize;
                    first = false;
                    Some(format!(
                        "{}using {} = {};\n",
                        PRE_ALIAS[idx], a.name, a.sub_type
                    ))
                }
                _ => None,
            })
            .collect()
    }

    pub fn declare(&self, cpp_props: &CppProps) -> String {
        "\n// declarations".to_string()
            + &((&self.cpp_types)
                .iter()
                .map(|t| t.declare(self, cpp_props))
                .collect::<String>())
    }

    pub fn prototype(&self, cpp_props: &CppProps) -> String {
        "\n// prototypes".to_string()
            + &(self.cpp_types)
                .iter()
                .map(|t| t.prototype(self, cpp_props))
                .collect::<String>()
    }

    pub fn define(&self, cpp_props: &CppProps) -> String {
        self.cpp_types
            .iter()
            .map(|t| t.define(self, cpp_props))
            .collect::<String>()
    }

    pub fn parse_primitive(
        &mut self,
        expr: target::Expr,
        props: &CppProps,
        meta: Metadata,
    ) -> String {
        match expr {
            target::Expr::Boolean => self.add_primitive("bool", meta).0,
            target::Expr::Int8 => self.add_primitive("uint8_t", meta).0,
            target::Expr::Uint8 => self.add_primitive("uint8_t", meta).0,
            target::Expr::Int16 => self.add_primitive("uint16_t", meta).0,
            target::Expr::Uint16 => self.add_primitive("uint16_t", meta).0,
            target::Expr::Int32 => self.add_primitive("uint32_t", meta).0,
            target::Expr::Uint32 => self.add_primitive("uint32_t", meta).0,
            target::Expr::Float32 => self.add_primitive("float", meta).0,
            target::Expr::Float64 => self.add_primitive("double", meta).0,
            target::Expr::String => {
                self.add_include_file("<string>");
                self.add_primitive("std::string", meta).0
            }
            target::Expr::Timestamp => {
                // NOTHING FOR NOW, might have to include "chrono",
                panic!("time stamps are not supported yet")
            }
            target::Expr::ArrayOf(sub_type) => {
                let name = format!("std::vector<{}>", sub_type);
                match self.cpp_type_indices.get(&name) {
                    Some(_) => name,
                    None => {
                        self.add_include_file("<vector>");
                        let (_, sub_idx) = self.add_incomplete(&sub_type);
                        let cpp_type = CppTypes::Array(sub_idx);
                        self.add_or_replace_cpp_type(name, cpp_type, meta).0
                    }
                }
            }
            target::Expr::DictOf(sub_type) => {
                let (file, name) = props.get_dictionary_info(&sub_type);
                match self.cpp_type_indices.get(&name) {
                    Some(_) => name,
                    None => {
                        self.add_include_file("<string>");
                        self.add_include_file(file);

                        let opt_sub = if sub_type.is_empty() {
                            None
                        } else {
                            let (_, sub_idx) = self.add_incomplete(&sub_type);
                            Some(sub_idx)
                        };
                        let cpp_type = CppTypes::Dictionary(opt_sub);
                        self.add_or_replace_cpp_type(name, cpp_type, meta).0
                    }
                }
            }
            target::Expr::Empty => {
                self.add_include_file("<optional>");
                "std::optional<JsonTypedefCodeGen::Data::JsonValue>".to_string()
            }
            target::Expr::NullableOf(sub_type) => {
                let name = format!("std::unique_ptr<{}>", sub_type);
                match self.cpp_type_indices.get(&name) {
                    Some(_) => name,
                    None => {
                        self.add_include_file("<memory>");

                        let (_, sub_idx) = self.add_incomplete(&sub_type);
                        let cpp_type = CppTypes::Nullable(sub_idx);
                        self.add_or_replace_cpp_type(name, cpp_type, meta).0
                    }
                }
            }
        }
    }

    pub fn parse_alias(&mut self, name: String, type_: String, meta: Metadata) {
        let (_, _) = self.add_incomplete(&type_);
        let cpp_type = CppTypes::Alias(CppAlias {
            name: name.clone(),
            sub_type: type_.clone(),
        });
        self.add_or_replace_cpp_type(name, cpp_type, meta);
    }

    pub fn parse_enum(&mut self, name: String, members: Vec<target::EnumMember>, meta: Metadata) {
        let cpp_type = CppTypes::Enum(CppEnum {
            name: name.clone(),
            members,
        });
        self.requires_enum_internal_code = true;
        self.add_src_include_file("<array>");
        self.add_src_include_file("<format>");
        self.add_src_include_file("<span>");
        self.add_src_include_file("<string_view>");
        self.add_or_replace_cpp_type(name, cpp_type, meta);
    }

    pub fn parse_struct(&mut self, name: String, fields: Vec<target::Field>, meta: Metadata) {
        let cpp_type_indices: Vec<usize> = self.field_to_type_indices(&fields);
        let cpp_type = CppTypes::Struct(CppStruct {
            name: name.clone(),
            fields,
            cpp_type_indices,
        });
        self.add_or_replace_cpp_type(name, cpp_type, meta);
    }

    pub fn parse_discriminator(
        &mut self,
        name: String,
        variants: Vec<target::DiscriminatorVariantInfo>,
        tag_json_name: String,
        tag_field_name: String,
        meta: Metadata,
    ) {
        let cpp_type_indices: Vec<usize> = variants
            .iter()
            .map(|v| self.add_incomplete(&v.type_name).1)
            .collect();
        let cpp_type = CppTypes::Discriminator(CppDiscriminator {
            name: name.clone(),
            variants,
            tag_json_name,
            tag_field_name,
            cpp_type_indices,
        });
        self.add_or_replace_cpp_type(name, cpp_type, meta);
    }

    pub fn parse_discriminator_variant(
        &mut self,
        name: String,
        fields: Vec<target::Field>,
        tag_field_name: String,
        tag_json_name: String,
        tag_value: String,
        parent_name: String,
        meta: Metadata,
    ) {
        let cpp_type_indices: Vec<usize> = self.field_to_type_indices(&fields);
        let cpp_type = CppTypes::DiscriminatorVariant(CppDiscriminatorVariant {
            name: name.clone(),
            fields,
            tag_field_name,
            tag_json_name,
            tag_value,
            parent_name,
            cpp_type_indices,
        });
        self.add_or_replace_cpp_type(name, cpp_type, meta);
    }

    fn add_or_replace_cpp_type(
        &mut self,
        name: String,
        cpp_type: CppTypes,
        meta: Metadata,
    ) -> (String, usize) {
        match self.cpp_type_indices.get(&name) {
            Some(idx) => {
                let i = *idx;
                self.replace_incomplete(i, cpp_type, meta);
                (name, i)
            }
            None => self.add_cpp_type(name, cpp_type, meta),
        }
    }

    fn add_cpp_type(
        &mut self,
        name: String,
        cpp_type: CppTypes,
        meta: Metadata,
    ) -> (String, usize) {
        let idx = self.cpp_types.len();
        self.names.push(name.clone());
        self.cpp_types.push(cpp_type);
        self.cpp_type_indices.insert(name.clone(), idx);
        self.metadatas.push(meta);
        (name, idx)
    }

    fn replace_incomplete(&mut self, idx: usize, cpp_type: CppTypes, meta: Metadata) {
        let curr: &CppTypes = &self.cpp_types[idx];
        if (*curr) != cpp_type {
            match curr {
                CppTypes::Incomplete => {
                    self.cpp_types[idx] = cpp_type;
                    self.metadatas[idx] = meta;
                }
                _ => {
                    let pmsg =
                        format!("try to replace non-incomplete type {curr:?} with {cpp_type:?}");
                    panic!("{}", pmsg);
                }
            }
        }
    }

    fn add_primitive(&mut self, name: &str, meta: Metadata) -> (String, usize) {
        let sname = name.to_string();
        let cpp_type = CppTypes::Primitive(sname.clone());
        self.add_or_replace_cpp_type(sname, cpp_type, meta)
    }

    fn add_incomplete(&mut self, name: &str) -> (String, usize) {
        let sname = name.to_string();
        match self.cpp_type_indices.get(name) {
            Some(idx) => (sname, *idx),
            None => self.add_cpp_type(sname, CppTypes::Incomplete, Metadata::default()),
        }
    }

    fn add_include_file(&mut self, file: &str) -> bool {
        self.include_files.insert(file.to_string())
    }

    fn add_src_include_file(&mut self, file: &str) -> bool {
        self.src_include_files.insert(file.to_string())
    }

    fn field_to_type_indices(&mut self, fields: &Vec<target::Field>) -> Vec<usize> {
        fields
            .iter()
            .map(|f| self.add_incomplete(&f.type_).1)
            .collect()
    }
}
