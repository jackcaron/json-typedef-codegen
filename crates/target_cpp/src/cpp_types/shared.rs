use jtd_codegen::target::Field;

use crate::cpp_snippets::ENTRIES_ARRAY;

pub type TypeIndex = usize;

pub fn create_entry_array(entries: &str, size: usize) -> String {
    ENTRIES_ARRAY
        .replace("$SIZE$", &size.to_string())
        .replace("$ENTRIES$", entries)
}

fn get_mandatory_indices(fields: &Vec<Field>, idx_offset: usize) -> Vec<usize> {
    fields
        .iter()
        .enumerate()
        .filter_map(|(i, f)| match f.optional {
            false => Some(i + idx_offset),
            true => None,
        })
        .collect()
}

pub fn create_mandatory_indices(fields: &Vec<Field>, idx_offset: usize) -> String {
    let midx: Vec<usize> = get_mandatory_indices(fields, idx_offset);

    let str_midx = midx
        .iter()
        .enumerate()
        .map(|(j, idx)| {
            let prefix = if j == 0 { "" } else { ", " };
            format!("{}{}", prefix, idx)
        })
        .collect::<String>();

    format!(
        r#"static constexpr std::array<int, {}> mandatory_indices = {{ {} }};"#,
        midx.len(),
        str_midx
    )
}

pub fn create_struct_from_fields(name: &str, fields: &Vec<Field>) -> String {
    let fields = &fields
        .iter()
        .map(|f| format!("  {} {};\n", f.type_, f.name))
        .collect::<String>();
    format!(
        r#"
struct {} {{
{}}};
"#,
        name, fields
    )
}

pub fn create_switch_clauses(fields: &Vec<Field>, offset: usize) -> String {
    fields
        .iter()
        .enumerate()
        .map(|(i, f)| {
            format!(
                r#"
                  case {}: return deserialize_and_set(result.{}, val);"#,
                i + offset,
                f.name
            )
        })
        .collect::<String>()
}

pub fn deserialize_name(name: &str) -> String {
    format!("deserialize_{}", name)
}

pub fn function_name(name: &str, full_ns: bool) -> String {
    format!(
        "ExpType<{}> {}(const {}Reader::JsonValue& value)",
        name,
        deserialize_name(name),
        if full_ns { "JsonTypedefCodeGen::" } else { "" }
    )
}

pub fn prototype_name(name: &str) -> String {
    format!(
        r#"
JsonTypedefCodeGen::{};"#,
        function_name(name, true)
    )
}

pub fn get_complete_definition(name: &str) -> String {
    format!(
        r#"
{} {{
  return JsonTypedefCodeGen::Deserialize::Json<{}>::deserialize(value);
}}
"#,
        function_name(name, false),
        name
    )
}

pub fn create_visited_array(sz: usize) -> String {
    let falses = match sz {
        0 | 1 => String::new(),
        _ => (0..(sz - 1)).map(|_i| ", false").collect::<String>(),
    };
    format!(
        r#"std::array<bool, {}> visited = {{ false{} }};"#,
        sz, falses
    )
}
