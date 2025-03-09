use crate::cpp_types::shared::*;
use crate::props::CppProps;
use crate::state::CppState;

#[derive(Debug, PartialEq)]
pub struct CppArray {
    idx: TypeIndex,
    name: String,
}

impl CppArray {
    pub fn new(idx: TypeIndex, name: String) -> CppArray {
        CppArray { idx, name }
    }
}

#[derive(Debug, PartialEq)]
pub struct CppDict {
    opt_idx: Option<TypeIndex>,
    name: String,
}

impl CppDict {
    pub fn new(opt_idx: Option<TypeIndex>, name: String) -> CppDict {
        CppDict { opt_idx, name }
    }

    pub fn get_set_internal_function(cpp_props: &CppProps) -> String {
        let settype = cpp_props.get_dictionary_info("").1;
        // TODO: use a simple "using" statement to define the SetType then full static code
        format!(
            r#"
template<> struct FromJson<{}> {{
  static ExpType<{}> convert(const Reader::JsonValue &value) {{
    return make_json_error(JsonErrorTypes::Unknown, "not ready yet"sv); // probably just reading the keys, ignore the data
  }}
}};
"#,
            settype, settype
        )
    }
}

#[derive(Debug, PartialEq)]
pub struct CppNullable {
    idx: TypeIndex,
    name: String,
}

impl CppNullable {
    pub fn new(idx: TypeIndex, name: String) -> CppNullable {
        CppNullable { idx, name }
    }

    fn get_full_name(&self, cpp_state: &CppState) -> String {
        let uptr_name = format!("std::unique_ptr<{}>", self.name);
        cpp_state.get_aliased_name(&uptr_name).to_string()
    }

    pub fn prototype(&self, cpp_state: &CppState) -> String {
        let full_name = self.get_full_name(cpp_state);
        format!(
            r#"
JsonTypedefCodeGen::ExpType<{}> deserialize_Nullable{}(const JsonTypedefCodeGen::Reader::JsonValue& value);"#,
            full_name, full_name
        )
    }

    pub fn define(&self, cpp_state: &CppState) -> String {
        let full_name = self.get_full_name(cpp_state);
        format!(
            r#"
ExpType<{}> deserialize_Nullable{}(const Reader::JsonValue& value) {{
  return FromJson<{}>::deserialize(value);
}}
"#,
            full_name, full_name, full_name
        )
    }
}
