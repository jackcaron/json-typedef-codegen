use jtd_codegen::target;
use jtd_codegen::target::metadata::Metadata;
use std::collections::{BTreeMap, BTreeSet};
use std::io::Write;

use crate::props::CppProps;

#[derive(PartialEq)]
pub struct CppEnum {}

#[derive(PartialEq)]
pub struct CppStruct {}

#[derive(PartialEq)]
pub struct CppAlias {
    name: String,
    sub_type_idx: usize,
}

#[derive(PartialEq)]
pub struct CppDiscriminator {}

#[derive(PartialEq)]
pub struct CppDiscriminatorVariant {}

#[derive(PartialEq)]
pub enum CppTypes {
    Incomplete,
    Primitive(String),
    Array(usize),
    Dictionary(usize),
    Nullable(usize),
    Enum(CppEnum),
    Struct(CppStruct),
    Alias(CppAlias),
    Discriminator(CppDiscriminator),
    DiscriminatorVariant(CppDiscriminatorVariant),
}

#[derive(Default)]
pub struct CppState {
    // - header files to include, ex.: optional, vector, string, ...
    include_files: BTreeSet<String>,

    metadatas: Vec<Metadata>,
    cpp_types: Vec<CppTypes>,
    cpp_type_indices: BTreeMap<String, usize>, // type name to index in "cpp_types"
}

impl CppState {
    pub fn write_include_files(&self, out: &mut dyn Write) -> jtd_codegen::Result<Option<String>> {
        for h in &self.include_files {
            writeln!(out, "#include {}", h)?;
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
                let (file, container) = props.get_dictionary_info();
                let name = format!("{}<std::string, {}>", container, sub_type);
                match self.cpp_type_indices.get(&name) {
                    Some(_) => name,
                    None => {
                        self.add_include_file("<string>");
                        self.add_include_file(file);

                        let (_, sub_idx) = self.add_incomplete(&sub_type);
                        let cpp_type = CppTypes::Dictionary(sub_idx);
                        self.add_or_replace_cpp_type(name.to_string(), cpp_type, meta)
                            .0
                    }
                }
            }
            target::Expr::Empty => "empty".to_string(), // no idea what to do
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
                    panic!("try to replace non-incomplete type");
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
}
