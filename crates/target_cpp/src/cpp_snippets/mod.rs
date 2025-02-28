use crate::props::CppProps;

pub const INTERNAL_CODE_HEADER: &'static str = include_str!("./internal_code_ns_header.cpp");

pub const INTERNAL_CODE_RAW_JSON_DATA: &'static str = include_str!("./internal_code_raw_json.cpp");

pub const INTERNAL_CODE_ARRAY: &'static str = include_str!("./internal_code_array.cpp");
pub const INTERNAL_CODE_MAP: &'static str = include_str!("./internal_code_object.cpp");
pub const INTERNAL_CODE_UPTR: &'static str = include_str!("./internal_code_unique_ptr.cpp");

pub fn get_from_json_dictionary_converter(cpp_props: &CppProps) -> String {
    format!(
        r#"
  template<typename Type>
  using JsonMap = {};
    {}"#,
        cpp_props.get_dictionary_info("Type").1,
        INTERNAL_CODE_MAP
    )
}

pub const INTERNAL_CODE_VALUE_INDEX: &'static str = include_str!("./internal_code_val_idx.cpp");

pub const INTERNAL_CODE_ENUM_INDEX: &'static str = include_str!("./internal_code_enum_idx.cpp");

pub const INTERNAL_CODE_STRUCT: &'static str = include_str!("./struct_from_json.cpp");

pub const INTERNAL_CODE_DISC: &'static str = include_str!("./disc_from_json.cpp");

pub const INTERNAL_CODE_VARY: &'static str = include_str!("./vary_from_json.cpp");

pub const INTERNAL_CODE_PRIM: &'static str = include_str!("./prim_from_json.cpp");

pub const ENUM_DECL: &'static str = include_str!("./enum_from_json.cpp");

pub const DESC_DECL: &'static str = include_str!("./disc_declare.cpp");

pub const ENTRIES_ARRAY: &'static str = include_str!("./entries_array.cpp");
