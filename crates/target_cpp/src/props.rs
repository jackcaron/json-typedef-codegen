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
            Guard::Name(_) => "#endif\n".to_string(),
        }
    }
}

#[derive(Deserialize, Default)]
enum Dictionary {
    #[default]
    #[serde(rename = "unordered")]
    Unordered,
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
pub struct CppProps {
    #[serde(rename = "guard")]
    guard: Option<Guard>,

    #[serde(rename = "namespace")]
    namespace: Option<String>,

    #[serde(rename = "dictionary_type", default)]
    dictionary_type: Dictionary,
    // include found header files needed?
    // include Json library, and where it is ?
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

    pub fn get_dictionary_info(&self, sub_type: &String) -> (&'static str, String) {
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
    use crate::props::{CppProps, Dictionary, Guard};

    #[test]
    fn default() {
        let props = CppProps::default();
        assert_eq!(props.guard.is_none(), true);
        assert_eq!(props.namespace.is_none(), true);
        assert_eq!(matches!(props.dictionary_type, Dictionary::Unordered), true);
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
}
