use anyhow;
use serde::Deserialize;
use std::fs::File;
use std::io;
use std::path::Path;

#[derive(Deserialize)]
enum Guard {
    #[serde(rename = "pragma")]
    Pragma,
    #[serde(rename = "name")]
    Name(String),
    // option to be the "output file name"
}

impl Guard {
    fn get_guard(&self) -> String {
        match self {
            Guard::Pragma => "#pragma once\n\n".to_string(),
            Guard::Name(n) => format!("#ifndef {}\n#define {}\n\n", n, n),
        }
    }

    fn get_footer(&self) -> String {
        match self {
            Guard::Pragma => String::new(),
            Guard::Name(_) => "\n#endif\n".to_string(),
        }
    }
}

#[derive(Deserialize, Default)]
enum Dictionary {
    #[serde(rename = "unordered")]
    Unordered,
    #[default]
    #[serde(rename = "ordered")]
    Ordered,
}

impl Dictionary {
    fn get_include_file(&self) -> &'static str {
        match self {
            Dictionary::Unordered => "<unordered_map>",
            Dictionary::Ordered => "<map>",
        }
    }
    fn get_container(&self) -> &'static str {
        match self {
            Dictionary::Unordered => "std::unordered_map",
            Dictionary::Ordered => "std::map",
        }
    }
    fn get_include_file_set(&self) -> &'static str {
        match self {
            Dictionary::Unordered => "<unordered_set>",
            Dictionary::Ordered => "<set>",
        }
    }
    fn get_container_set(&self) -> &'static str {
        match self {
            Dictionary::Unordered => "std::unordered_set",
            Dictionary::Ordered => "std::set",
        }
    }
}

#[derive(Default, Deserialize)]
enum JsonCodeGenInclude {
    #[default]
    #[serde(rename = "ignore")]
    Ignore,
    #[serde(rename = "local")]
    Local,
    #[serde(rename = "system")]
    System,
    #[serde(rename = "path")]
    Path(String),
}

impl JsonCodeGenInclude {
    pub fn get_header_file(&self, basename: &str) -> String {
        let value = match self {
            JsonCodeGenInclude::Ignore => None,
            JsonCodeGenInclude::Local => Some(format!("\"{}\"", basename)),
            JsonCodeGenInclude::System => Some(format!("<{}>", basename)),
            JsonCodeGenInclude::Path(path) => Some(path.clone()),
        };
        match value {
            None => String::new(),
            Some(inc) => format!("#include {}\n", inc),
        }
    }
}

#[derive(Default, Deserialize)]
pub struct CppProps {
    #[serde(rename = "guard")]
    guard: Option<Guard>,

    #[serde(rename = "namespace")]
    namespace: Option<String>,

    #[serde(rename = "dictionary_type", default)]
    dictionary_type: Dictionary,

    #[serde(rename = "include_reader", default)]
    include_reader: JsonCodeGenInclude,
    #[serde(rename = "include_data", default)]
    include_data: JsonCodeGenInclude,
    // include found header files needed?
    // implement destructors
    // provide copy (with constructor/assignment) or "clone" function
    // use module (future)
    // identation: Space(#), Tabs ????
}

impl CppProps {
    pub fn get_guard(&self) -> String {
        match &self.guard {
            Some(head) => head.get_guard(),
            None => String::new(),
        }
    }

    pub fn get_footer(&self) -> String {
        match &self.guard {
            Some(head) => head.get_footer(),
            None => String::new(),
        }
    }

    pub fn open_namespace(&self) -> String {
        match &self.namespace {
            Some(ns) => format!("namespace {} {{\n", ns),
            None => String::new(),
        }
    }

    pub fn close_namespace(&self) -> String {
        match &self.namespace {
            Some(ns) => format!("}} // namespace {}\n", ns),
            None => String::new(),
        }
    }

    pub fn get_namespaced_name(&self, name: &str) -> String {
        match &self.namespace {
            Some(ns) => format!("{}::{}", ns, name),
            None => name.to_string(),
        }
    }

    pub fn get_codegen_includes(&self) -> String {
        self.include_data.get_header_file("json_data.hpp")
            + &self.include_reader.get_header_file("json_reader.hpp")
    }

    pub fn from_file(filename: Option<&str>) -> anyhow::Result<CppProps> {
        match filename {
            Some(fname) => {
                let pname = Path::new(fname);
                let file = File::open(pname)?;
                let reader = io::BufReader::new(file);

                Ok(serde_json::from_reader(reader)?)
            }
            None => Ok(CppProps::default()),
        }
    }

    pub fn get_dictionary_info(&self, sub_type: &str) -> (&'static str, String) {
        let dic = &self.dictionary_type;
        if sub_type.is_empty() {
            (
                dic.get_include_file_set(),
                format!("{}<std::string>", dic.get_container_set()),
            )
        } else {
            (
                dic.get_include_file(),
                format!("{}<std::string, {}>", dic.get_container(), sub_type),
            )
        }
    }
}

// ------
#[cfg(test)]
mod tests {
    use crate::props::{CppProps, Dictionary, Guard, JsonCodeGenInclude};

    #[test]
    fn default() {
        let props = CppProps::default();
        assert_eq!(props.guard.is_none(), true);
        assert_eq!(props.namespace.is_none(), true);
        assert_eq!(matches!(props.dictionary_type, Dictionary::Ordered), true);
        assert_eq!(
            matches!(props.include_data, JsonCodeGenInclude::Ignore),
            true
        );
        assert_eq!(
            matches!(props.include_reader, JsonCodeGenInclude::Ignore),
            true
        );
    }

    #[test]
    fn has_namespace() {
        let json = r#"{"namespace":"bob"}"#;

        let props: CppProps = serde_json::from_str(json).unwrap();
        assert_eq!(props.guard.is_none(), true);
        assert_eq!(props.namespace.is_none(), false);
        assert_eq!(props.namespace, Some("bob".to_string()));
    }

    #[test]
    fn uses_pragma() {
        let json = r#"{"guard":"pragma"}"#;

        let props: CppProps = serde_json::from_str(json).unwrap();
        assert_eq!(
            match &props.guard {
                Some(Guard::Pragma) => true,
                _ => false,
            },
            true
        );
        assert_eq!(props.namespace.is_none(), true);
    }

    #[test]
    fn uses_guard() {
        let json = r#"{"guard":{"name":"GUARD"}}"#;

        let props: CppProps = serde_json::from_str(json).unwrap();
        assert_eq!(
            match &props.guard {
                Some(Guard::Name(g)) => g.eq("GUARD"),
                _ => false,
            },
            true
        );
        assert_eq!(props.namespace.is_none(), true);
    }

    #[test]
    fn uses_ordered_map() {
        let json = r#"{"dictionary_type":"ordered"}"#;

        let props: CppProps = serde_json::from_str(json).unwrap();
        assert_eq!(matches!(props.dictionary_type, Dictionary::Ordered), true);
        assert_eq!(props.namespace.is_none(), true);
    }

    #[test]
    fn uses_includes() {
        let inc1 = r#"{"include_reader":"local","include_data":"system"}"#;
        let inc2 = r#"{"include_data":{"path":"where/it/is"}}"#;

        let props1: CppProps = serde_json::from_str(inc1).unwrap();
        assert_eq!(
            matches!(props1.include_reader, JsonCodeGenInclude::Local),
            true
        );
        assert_eq!(
            matches!(props1.include_data, JsonCodeGenInclude::System),
            true
        );
        let props2: CppProps = serde_json::from_str(inc2).unwrap();
        assert_eq!(
            matches!(props2.include_reader, JsonCodeGenInclude::Ignore),
            true
        );
        assert_eq!(
            match props2.include_data {
                JsonCodeGenInclude::Path(path) => path.eq("where/it/is"),
                _ => false,
            },
            true
        );
    }
}
