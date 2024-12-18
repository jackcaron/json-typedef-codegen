use jtd_codegen::target;
use jtd_codegen::target::metadata::Metadata;
use std::collections::{BTreeMap, BTreeSet};
use std::io::Write;

use crate::props::CppProps;

#[derive(Debug, PartialEq)]
pub struct CppEnum {
    members: Vec<target::EnumMember>,
}

#[derive(Debug, PartialEq)]
pub struct CppStruct {
    fields: Vec<target::Field>,
    cpp_type_indices: Vec<usize>,
}

#[derive(Debug, PartialEq)]
pub struct CppAlias {
    name: String,
    sub_type_idx: usize,
}

#[derive(Debug, PartialEq)]
pub struct CppDiscriminator {
    variants: Vec<target::DiscriminatorVariantInfo>,
    tag_json_name: String,
    tag_field_name: String,
    cpp_type_indices: Vec<usize>,
}

#[derive(Debug, PartialEq)]
pub struct CppDiscriminatorVariant {
    fields: Vec<target::Field>,
    tag_field_name: String,
    tag_json_name: String,
    tag_value: String,
    parent_name: String,
    cpp_type_indices: Vec<usize>,
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
    pub fn has_forward_declaration(&self) -> bool {
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
            Self::Enum(_) => "enum class",
            Self::Struct(_) => "struct",
            Self::Discriminator(_) => "struct",
            Self::DiscriminatorVariant(_) => "struct",
            _ => panic!("type doesn't have a prefix"),
        }
    }
}

#[derive(Default)]
pub struct CppState {
    // - header files to include, ex.: optional, vector, string, ...
    include_files: BTreeSet<String>,

    metadatas: Vec<Metadata>,
    names: Vec<String>,
    cpp_types: Vec<CppTypes>,
    cpp_type_indices: BTreeMap<String, usize>, // type name to index in "cpp_types"
}

impl CppState {
    pub fn inspect(&self) {
        println!("-- inspect:");
        if !self.include_files.is_empty() {
            println!("  - include files:");
            for f in &self.include_files {
                println!("    - {}", f);
            }
        }
        if !self.cpp_types.is_empty() {
            println!("  - types:");
            for (i, t) in (&self.cpp_types).into_iter().enumerate() {
                println!("    - #{i} : {t:?}");
            }
        }
        println!("-- inspect: DONE");
    }

    // SHOULD accumulate into a string instead
    pub fn write_include_files(&self, out: &mut dyn Write) -> jtd_codegen::Result<Option<String>> {
        for h in &self.include_files {
            writeln!(out, "#include {}", h)?;
        }
        Ok(None)
    }

    // SHOULD accumulate into a string instead
    pub fn write_forward_declarations(
        &self,
        out: &mut dyn Write,
    ) -> jtd_codegen::Result<Option<String>> {
        let forwards = (&self.cpp_types)
            .into_iter()
            .enumerate()
            .filter(|it| it.1.has_forward_declaration())
            .map(|it| (&self.names[it.0], it.1.type_prefix()));

        let mut first = true;
        for (name, prefix) in forwards {
            if first {
                first = false;
                writeln!(out, "\n// forward declarations")?;
            }
            writeln!(out, "{} {};", prefix, name)?;
        }
        Ok(None)
    }

    // SHOULD accumulate into a string instead
    pub fn write_alias(&self, out: &mut dyn Write) -> jtd_codegen::Result<Option<String>> {
        let aliases = (&self.cpp_types)
            .into_iter()
            .map(|t| match t {
                CppTypes::Alias(a) => Some(a),
                _ => None,
            })
            .filter(|o| o.is_some())
            .map(|o| unsafe { o.unwrap_unchecked() });

        let mut first = true;
        for alias in aliases {
            if first {
                first = false;
                writeln!(out, "\n// aliases")?;
            }
            writeln!(
                out,
                "using {} = {};",
                alias.name, self.names[alias.sub_type_idx]
            )?;
        }
        Ok(None)
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
                "timestamp".to_string() // dummy value
            }
            target::Expr::ArrayOf(sub_type) => {
                let name = format!("std::vector<{}>", sub_type);
                match self.cpp_type_indices.get(&name) {
                    Some(_) => name,
                    None => {
                        self.add_include_file("<vector>");
                        let (_, sub_idx) = self.add_incomplete(&sub_type);
                        let cpp_type = CppTypes::Array(sub_idx);
                        self.add_or_replace_cpp_type(name.to_string(), cpp_type, meta)
                            .0
                    }
                }
            }
            target::Expr::DictOf(sub_type) => {
                let is_empty = sub_type.is_empty();
                let (file, container) = props.get_dictionary_info(is_empty);
                let name = if is_empty {
                    format!("{}<std::string>", container)
                } else {
                    format!("{}<std::string, {}>", container, sub_type)
                };
                match self.cpp_type_indices.get(&name) {
                    Some(_) => name,
                    None => {
                        self.add_include_file("<string>");
                        self.add_include_file(file);

                        let opt_sub = if is_empty {
                            None
                        } else {
                            let (_, sub_idx) = self.add_incomplete(&sub_type);
                            Some(sub_idx)
                        };
                        let cpp_type = CppTypes::Dictionary(opt_sub);
                        self.add_or_replace_cpp_type(name.to_string(), cpp_type, meta)
                            .0
                    }
                }
            }
            target::Expr::Empty => "".to_string(), // set to be empty or null
            target::Expr::NullableOf(sub_type) => {
                let name = format!("std::unique_ptr<{}>", sub_type);
                match self.cpp_type_indices.get(&name) {
                    Some(_) => name,
                    None => {
                        self.add_include_file("<memory>");

                        let (_, sub_idx) = self.add_incomplete(&sub_type);
                        let cpp_type = CppTypes::Nullable(sub_idx);
                        self.add_or_replace_cpp_type(name.to_string(), cpp_type, meta)
                            .0
                    }
                }
            }
        }
    }

    pub fn parse_alias(&mut self, name: String, type_: String, meta: Metadata) {
        let (_, sub_type_idx) = self.add_incomplete(&type_);
        let cpp_type = CppTypes::Alias(CppAlias {
            name: name.clone(),
            sub_type_idx,
        });
        self.add_or_replace_cpp_type(name, cpp_type, meta);
    }

    pub fn parse_enum(&mut self, name: String, members: Vec<target::EnumMember>, meta: Metadata) {
        let cpp_type = CppTypes::Enum(CppEnum { members });
        self.add_or_replace_cpp_type(name, cpp_type, meta);
    }

    pub fn parse_struct(&mut self, name: String, fields: Vec<target::Field>, meta: Metadata) {
        let cpp_type_indices: Vec<usize> = self.field_to_type_indices(&fields);
        let cpp_type = CppTypes::Struct(CppStruct {
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

    fn field_to_type_indices(&mut self, fields: &Vec<target::Field>) -> Vec<usize> {
        fields
            .iter()
            .filter(|f| !f.type_.is_empty())
            .map(|f| self.add_incomplete(&f.type_).1)
            .collect()
    }
}
