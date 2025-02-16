use crate::cpp_snippets::ENTRIES_ARRAY;

pub type TypeIndex = usize;

pub fn create_entry_array(entries: &str, size: usize) -> String {
    ENTRIES_ARRAY
        .replace("$SIZE$", &size.to_string())
        .replace("$ENTRIES$", entries)
}

pub fn deserialize_name(name: &str) -> String {
    format!("fromJson{}", name)
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
return FromJson<{}>::convert(value);
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
