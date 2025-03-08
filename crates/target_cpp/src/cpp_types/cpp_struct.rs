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
        create_struct_from_fields(&self.name, &self.fields)
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

    pub fn get_internal_code(&self, cpp_props: &CppProps) -> String {
        let fullname = cpp_props.get_namespaced_name(&self.name);
        let entries = self.create_entry_array();
        let mandatory_indices = create_mandatory_indices(&self.fields, 0);
        let visited = create_visited_array(self.fields.len());
        let clauses = create_switch_clauses(&self.fields, 0);
        INTERNAL_CODE_STRUCT
            .replace("$FULL_NAME$", &fullname)
            .replace("$ENTRIES$", &entries)
            .replace("$MANDATORY$", &mandatory_indices)
            .replace("$VISITED$", &visited)
            .replace("$STRUCT_NAME$", &self.name)
            .replace("$CLAUSES$", &clauses)
    }
}
