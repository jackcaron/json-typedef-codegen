use jtd_codegen::target;
use jtd_codegen::target::metadata::Metadata;
use std::collections::{BTreeMap, BTreeSet};

use crate::{cpp_snippets::*, cpp_types::*, props::CppProps};

#[derive(Default)]
pub struct CppState {
    // - header files to include, ex.: optional, vector, string, ...
    include_files: BTreeSet<String>,
    src_include_files: BTreeSet<String>, // header needed for the source file to work

    metadatas: Vec<Metadata>,
    names: Vec<String>,
    cpp_types: Vec<CppTypes>,
    cpp_type_indices: BTreeMap<String, usize>, // type name to index in "cpp_types"

    root_type: Option<String>,
}

impl CppState {
    pub fn get_aliased_name<'a>(&'a self, name: &'a str) -> &'a str {
        self.cpp_types
            .iter()
            .filter_map(|t| match t {
                CppTypes::Alias(alias) => Some(alias),
                _ => None,
            })
            .find(|a| a.sub_matches(name))
            .map(|a| a.get_name())
            .unwrap_or(name)
    }

    pub fn get_index_from_name(&self, name: &str) -> Option<usize> {
        match self.cpp_type_indices.get(name) {
            Some(ridx) => Some(*ridx),
            None => None,
        }
    }

    pub fn write_include_files(&self, cpp_props: &CppProps) -> String {
        cpp_props.get_codegen_includes()
            + &((&self.include_files)
                .iter()
                .map(|h| format!("#include {}\n", h))
                .collect::<String>())
    }

    pub fn write_src_include_files(&self, cpp_props: &CppProps) -> String {
        "#define IMPL_DESERIALIZE\n".to_string()
            + &cpp_props.get_codegen_src_includes()
            + &(self
                .src_include_files
                .iter()
                .filter_map(|h| {
                    if self.include_files.contains(h) {
                        None
                    } else {
                        Some(format!("#include {}\n", h))
                    }
                })
                .collect::<String>())
    }

    pub fn write_internal_code(&self, cpp_props: &CppProps) -> String {
        let type_internal_code = self
            .cpp_types
            .iter()
            .filter_map(|t| t.get_internal_code(&self, cpp_props))
            .collect::<String>();

        INTERNAL_CODE_HEADER.to_string()
            + &type_internal_code
            + "\n} // namespace JsonTypedefCodeGen::Deserialize\n"
    }

    pub fn write_forward_declarations(&self) -> String {
        let forward = (&self.cpp_types)
            .iter()
            .enumerate()
            .filter_map(|(i, t)| {
                t.type_prefix()
                    .map(|pf| format!("{} {};\n", pf, self.names[i]))
            })
            .collect::<String>();
        if forward.is_empty() {
            forward
        } else {
            format!("\n// forward declarations\n{}", forward)
        }
    }

    pub fn write_alias(&self) -> String {
        let aliases = (&self.cpp_types)
            .iter()
            .filter_map(|t| match t {
                CppTypes::Alias(a) => Some(a.format()),
                _ => None,
            })
            .collect::<String>();
        if aliases.is_empty() {
            aliases
        } else {
            format!("\n// aliases\n{}", aliases)
        }
    }

    pub fn conclude(&mut self) {}

    pub fn declare(&self, cpp_props: &CppProps) -> String {
        let declares = (&self.cpp_types)
            .iter()
            .filter_map(|t| t.declare(self, cpp_props))
            .collect::<String>();
        if declares.is_empty() {
            declares
        } else {
            format!("\n// declarations{}", declares)
        }
    }

    fn get_root_type(&self) -> &CppTypes {
        let opt_ct = match &self.root_type {
            None => panic!("Missing root type"),
            Some(name) => self
                .get_index_from_name(name)
                .map(|idx| &(self.cpp_types[idx])),
        };
        match opt_ct {
            Some(ct) => ct,
            None => panic!("Root type invalid"),
        }
    }

    pub fn prototype(&self, cpp_props: &CppProps) -> String {
        match self.get_root_type().prototype(self, cpp_props) {
            Some(proto) => format!("\n// prototypes{}", proto),
            None => String::new(),
        }
    }

    pub fn define(&self, cpp_props: &CppProps) -> String {
        match self.get_root_type().define(self, cpp_props) {
            Some(proto) => proto,
            None => String::new(),
        }
    }

    pub fn parse_primitive(
        &mut self,
        expr: target::Expr,
        props: &CppProps,
        meta: Metadata,
    ) -> String {
        match expr {
            target::Expr::Boolean => self.add_primitive(Primitives::Bool, meta).0,
            target::Expr::Int8 => self.add_primitive(Primitives::Int8, meta).0,
            target::Expr::Uint8 => self.add_primitive(Primitives::Uint8, meta).0,
            target::Expr::Int16 => self.add_primitive(Primitives::Int16, meta).0,
            target::Expr::Uint16 => self.add_primitive(Primitives::Uint16, meta).0,
            target::Expr::Int32 => self.add_primitive(Primitives::Int32, meta).0,
            target::Expr::Uint32 => self.add_primitive(Primitives::Uint32, meta).0,
            target::Expr::Float32 => self.add_primitive(Primitives::Float32, meta).0,
            target::Expr::Float64 => self.add_primitive(Primitives::Float64, meta).0,
            target::Expr::String => {
                self.add_include_file("<string>");
                self.add_primitive(Primitives::String, meta).0
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
                        let cpp_type = CppTypes::Array(CppArray::new(sub_idx, &name));
                        self.add_or_replace_cpp_type(&name, cpp_type, meta).0
                    }
                }
            }
            target::Expr::DictOf(sub_type) => {
                let (file, name) = props.get_dictionary_info(&sub_type);
                match self.cpp_type_indices.get(&name) {
                    Some(_) => name,
                    None => {
                        self.add_src_include_file("<format>");
                        self.add_include_file("<string>");
                        self.add_include_file(file);

                        let opt_sub = if sub_type.is_empty() {
                            None
                        } else {
                            let (_, sub_idx) = self.add_incomplete(&sub_type);
                            Some(sub_idx)
                        };
                        let cpp_type = CppTypes::Dictionary(CppDict::new(opt_sub, &name));
                        self.add_or_replace_cpp_type(&name, cpp_type, meta).0
                    }
                }
            }
            target::Expr::Empty => "JsonTypedefCodeGen::Data::JsonValue".to_string(),
            target::Expr::NullableOf(sub_type) => {
                let name = format!("std::unique_ptr<{}>", sub_type);

                if let Some(rootname) = &self.root_type {
                    if (*rootname) == sub_type {
                        self.root_type = Some(name.clone());
                    }
                }

                match self.cpp_type_indices.get(&name) {
                    Some(_) => name,
                    None => {
                        self.add_include_file("<memory>");

                        let (_, sub_idx) = self.add_incomplete(&sub_type);
                        let cpp_type = CppTypes::Nullable(CppNullable::new(sub_idx, &sub_type));
                        self.add_or_replace_cpp_type(&name, cpp_type, meta).0
                    }
                }
            }
        }
    }

    fn toggle_enum_index_code(&mut self) {
        self.add_src_include_file("<array>");
        self.add_src_include_file("<format>");
        self.add_src_include_file("<span>");
        self.add_src_include_file("<string_view>");
    }

    fn toggle_value_index_code(&mut self) {
        self.add_src_include_file("<array>");
        self.add_src_include_file("<format>");
        self.add_src_include_file("<span>");
        self.add_src_include_file("<string_view>");
    }

    pub fn parse_alias(&mut self, name: &str, type_: &str, meta: Metadata) {
        self.set_root_type(&name);
        let (_, _) = self.add_incomplete(&type_);
        let cpp_type = CppTypes::Alias(CppAlias::new(&name, type_));
        self.add_or_replace_cpp_type(name, cpp_type, meta);
    }

    pub fn parse_enum(&mut self, name: &str, members: Vec<target::EnumMember>, meta: Metadata) {
        self.set_root_type(&name);
        let cpp_type = CppTypes::Enum(CppEnum::new(&name, members));
        self.toggle_enum_index_code();
        self.add_or_replace_cpp_type(name, cpp_type, meta);
    }

    pub fn parse_struct(&mut self, name: &str, fields: Vec<target::Field>, meta: Metadata) {
        self.set_root_type(&name);
        let cpp_type_indices: Vec<usize> = self.field_to_type_indices(&fields);
        self.toggle_value_index_code();
        let cpp_type = CppTypes::Struct(CppStruct::new(&name, fields, cpp_type_indices));
        self.add_or_replace_cpp_type(name, cpp_type, meta);
    }

    pub fn parse_discriminator(
        &mut self,
        name: &str,
        variants: Vec<target::DiscriminatorVariantInfo>,
        tag_json_name: &str,
        tag_field_name: &str,
        meta: Metadata,
    ) {
        self.set_root_type(name);
        let cpp_type_indices: Vec<usize> = variants
            .iter()
            .map(|v| self.add_incomplete(&v.type_name).1)
            .collect();
        let cpp_type = CppTypes::Discriminator(CppDiscriminator::new(
            name,
            variants,
            tag_json_name,
            tag_field_name,
            cpp_type_indices,
        ));
        self.toggle_value_index_code();
        self.add_or_replace_cpp_type(name, cpp_type, meta);
    }

    pub fn parse_discriminator_variant(
        &mut self,
        name: &str,
        fields: Vec<target::Field>,
        tag_field_name: &str,
        tag_json_name: &str,
        tag_value: &str,
        parent_name: &str,
        meta: Metadata,
    ) {
        let cpp_type_indices: Vec<usize> = self.field_to_type_indices(&fields);
        let cpp_type = CppTypes::DiscriminatorVariant(CppDiscriminatorVariant::new(
            name,
            fields,
            tag_field_name,
            tag_json_name,
            tag_value,
            parent_name,
            cpp_type_indices,
        ));
        self.toggle_value_index_code();
        self.add_or_replace_cpp_type(&name, cpp_type, meta);
    }

    fn add_or_replace_cpp_type(
        &mut self,
        name: &str,
        cpp_type: CppTypes,
        meta: Metadata,
    ) -> (String, usize) {
        match self.cpp_type_indices.get(name) {
            Some(idx) => {
                let i = *idx;
                self.replace_incomplete(i, cpp_type, meta);
                (name.to_string(), i)
            }
            None => self.add_cpp_type(name, cpp_type, meta),
        }
    }

    fn add_cpp_type(&mut self, name: &str, cpp_type: CppTypes, meta: Metadata) -> (String, usize) {
        let idx = self.cpp_types.len();
        self.names.push(name.to_string());
        self.cpp_types.push(cpp_type);
        self.cpp_type_indices.insert(name.to_string(), idx);
        self.metadatas.push(meta);
        (name.to_string(), idx)
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

    fn add_primitive(&mut self, prim: Primitives, meta: Metadata) -> (String, usize) {
        let name = prim.cpp_name();
        let cpp_type = CppTypes::Primitive(prim);
        self.add_or_replace_cpp_type(name, cpp_type, meta)
    }

    fn add_incomplete(&mut self, name: &str) -> (String, usize) {
        match self.cpp_type_indices.get(name) {
            Some(idx) => (name.to_string(), *idx),
            None => self.add_cpp_type(name, CppTypes::Incomplete, Metadata::default()),
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

    fn set_root_type(&mut self, name: &str) {
        match self.root_type {
            Some(_) => {}
            None => self.root_type = Some(name.to_string()),
        }
    }
}
