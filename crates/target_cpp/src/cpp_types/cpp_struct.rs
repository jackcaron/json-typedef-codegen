use jtd_codegen::target;

use crate::cpp_snippets::INTERNAL_CODE_STRUCT;
use crate::cpp_types::shared::*;
use crate::props::CppProps;

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

    pub fn get_name<'a>(&'a self) -> &'a str {
        &self.name
    }

    pub fn get_prefix() -> &'static str {
        "struct"
    }

    pub fn declare(&self) -> String {
        let fields = (&self.fields)
            .iter()
            .map(|f| format!("  {} {};\n", f.type_, f.name))
            .collect::<String>();
        format!("\nstruct {} {{\n{}}};\n", self.name, fields)
    }

    pub fn prototype(&self) -> String {
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

    fn create_mandatory_indices(&self) -> String {
        let midx: Vec<usize> = self
            .fields
            .iter()
            .enumerate()
            .filter_map(|(i, f)| match f.optional {
                false => Some(i),
                true => None,
            })
            .collect();
        create_mandatory_indices(&midx)
    }

    pub fn get_internal_code(&self, cpp_props: &CppProps) -> String {
        let fullname = cpp_props.get_namespaced_name(&self.name);
        let clauses = self.create_switch_clauses();
        let entries = self.create_entry_array();
        let mandatory_indices = self.create_mandatory_indices();
        let visited = create_visited_array(self.fields.len());
        INTERNAL_CODE_STRUCT
            .replace("$FULL_NAME$", &fullname)
            .replace("$ENTRIES$", &entries)
            .replace("$MANDATORY$", &mandatory_indices)
            .replace("$VISITED$", &visited)
            .replace("$STRUCT_NAME$", &self.name)
            .replace("$CLAUSES$", &clauses)
    }
}
