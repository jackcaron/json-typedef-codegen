use jtd_codegen::target;

use crate::cpp_snippets::{ENUM_DECL, INTERNAL_CODE_ENUM_INDEX};
use crate::cpp_types::shared::*;
use crate::props::CppProps;

#[derive(Debug, PartialEq)]
pub struct CppEnum {
    name: String,
    members: Vec<target::EnumMember>,
}

impl CppEnum {
    pub fn new(name: String, members: Vec<target::EnumMember>) -> CppEnum {
        CppEnum { name, members }
    }

    pub fn get_name<'a>(&'a self) -> &'a str {
        &self.name
    }

    pub fn get_prefix() -> &'static str {
        "enum class"
    }

    pub fn get_enum_index_code() -> &'static str {
        INTERNAL_CODE_ENUM_INDEX
    }

    pub fn declare(&self) -> String {
        let members = self
            .members
            .iter()
            .map(|m| format!("  {},\n", m.name))
            .collect::<String>();
        format!(
            r#"
enum class {} {{
{}}};
"#,
            self.name, members
        )
    }

    pub fn prototype(&self) -> String {
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

    pub fn get_internal_code(&self, cpp_props: &CppProps) -> String {
        let fullname = cpp_props.get_namespaced_name(&self.name);
        let entries = self.create_entry_array();
        let clauses = self.create_switch_clauses();
        ENUM_DECL
            .replace("$FULL_NAME$", &fullname)
            .replace("$ENTRIES$", &entries)
            .replace("$ENUM_NAME$", &self.name)
            .replace("$CLAUSES$", &clauses)
    }
}
