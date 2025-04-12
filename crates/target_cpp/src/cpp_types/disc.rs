use jtd_codegen::target;

use crate::cpp_snippets::{DESC_DECL, INTERNAL_CODE_DISC, INTERNAL_CODE_VARY};
use crate::cpp_types::shared::*;
use crate::props::CppProps;

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
        name: &str,
        variants: Vec<target::DiscriminatorVariantInfo>,
        tag_json_name: &str,
        tag_field_name: &str,
        cpp_type_indices: Vec<TypeIndex>,
    ) -> CppDiscriminator {
        CppDiscriminator {
            name: name.to_string(),
            variants,
            tag_json_name: tag_json_name.to_string(),
            tag_field_name: tag_field_name.to_string(),
            cpp_type_indices,
        }
    }

    pub fn get_name<'a>(&'a self) -> &'a str {
        &self.name
    }

    pub fn get_prefix() -> &'static str {
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

    pub fn declare(&self) -> String {
        let variant_types = self.get_variante_types();
        let enum_items = self.get_enum_type_items();
        DESC_DECL
            .replace("$DISCRIMINATOR_NAME$", &self.name)
            .replace("$VARIANT_NAMES$", &variant_types)
            .replace("$TYPE_VALUES$", &enum_items)
    }

    pub fn prototype(&self) -> String {
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

    fn create_switch_clauses(&self, cpp_props: &CppProps) -> String {
        self.variants
            .iter()
            .enumerate()
            .map(|(i, v)| {
                format!(
                    r#"
        case {}: return JsonTypedefCodeGen::Deserialize::Json<{}>::deserialize(object).transform(cast);"#,
                    i,
                    cpp_props.get_namespaced_name(&v.type_name)
                )
            })
            .collect::<String>()
    }

    pub fn get_internal_code(&self, cpp_props: &CppProps) -> String {
        let fullname = cpp_props.get_namespaced_name(&self.name);
        let entries = self.create_entry_array();
        let clauses = self.create_switch_clauses(cpp_props);
        INTERNAL_CODE_DISC
            .replace("$FULL_NAME$", &fullname)
            .replace("$ENTRIES$", &entries)
            .replace("$TAG_KEY$", &self.tag_json_name)
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
        name: &str,
        fields: Vec<target::Field>,
        tag_field_name: &str,
        tag_json_name: &str,
        tag_value: &str,
        parent_name: &str,
        cpp_type_indices: Vec<TypeIndex>,
    ) -> CppDiscriminatorVariant {
        CppDiscriminatorVariant {
            name: name.to_string(),
            fields,
            tag_field_name: tag_field_name.to_string(),
            tag_json_name: tag_json_name.to_string(),
            tag_value: tag_value.to_string(),
            parent_name: parent_name.to_string(),
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
                let prefix = if i != 0 && (i % 5) == 0 {
                    ",\n        "
                } else {
                    ", "
                };
                format!("{}\"{}\"sv", prefix, f.json_name)
            })
            .collect::<String>();

        create_entry_array(
            &format!("\"{}\"sv{}", self.tag_json_name, items),
            self.fields.len() + 1,
        )
    }

    pub fn get_internal_code(&self, cpp_props: &CppProps) -> String {
        let fullname = cpp_props.get_namespaced_name(&self.name);
        let entries = self.create_entry_array();
        let mandatory_indices = create_mandatory_indices(&self.fields, 1);
        let visited = create_visited_array(self.fields.len() + 1);
        let clauses = create_switch_clauses(&self.fields, 1);
        INTERNAL_CODE_VARY
            .replace("$FULL_NAME$", &fullname)
            .replace("$ENTRIES$", &entries)
            .replace("$MANDATORY$", &mandatory_indices)
            .replace("$VISITED$", &visited)
            .replace("$VARY_NAME$", &self.name)
            .replace("$CLAUSES$", &clauses)
    }
}
