use jtd_codegen::target;

use crate::cpp_snippets::{ENUM_DECL, ENUM_SER_DECL};
use crate::cpp_types::shared::*;
use crate::props::CppProps;

#[derive(Debug, PartialEq)]
pub struct CppEnum {
    name: String,
    members: Vec<target::EnumMember>,
}

impl CppEnum {
    pub fn new(name: &str, members: Vec<target::EnumMember>) -> CppEnum {
        CppEnum {
            name: name.to_string(),
            members,
        }
    }

    pub fn get_name<'a>(&'a self) -> &'a str {
        &self.name
    }

    pub fn get_prefix() -> &'static str {
        "enum class"
    }

    pub fn declare(&self) -> String {
        let members = self
            .members
            .iter()
            .map(|m| format!("  {},\n", m.name))
            .collect::<String>();
        format!(
            r#"
enum class {}: int {{
{}}};
"#,
            self.name, members
        )
    }

    pub fn prototype(&self, cpp_props: &CppProps) -> String {
        prototype_name(&self.name, cpp_props)
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

    pub fn get_common_internal_code(&self, cpp_props: &CppProps) -> String {
        let fullname = cpp_props.get_namespaced_name(&self.name);
        let entries = self.create_entry_array();

        format!(
            r#"
  template<> struct Common<{}> {{
    {}
  }};
"#,
            fullname, entries
        )
    }

    pub fn get_des_internal_code(&self, cpp_props: &CppProps) -> String {
        let fullname = cpp_props.get_namespaced_name(&self.name);
        ENUM_DECL
            .replace("$FULL_NAME$", &fullname)
            .replace("$ENUM_NAME$", &self.name)
    }

    pub fn get_ser_internal_code(&self, cpp_props: &CppProps) -> String {
        let fullname = cpp_props.get_namespaced_name(&self.name);
        ENUM_SER_DECL.replace("$FULL_NAME$", &fullname)
    }
}
