use anyhow;
use serde::Deserialize;
use std::fs::File;
use std::io;
use std::path::Path;

#[derive(Deserialize)]
pub enum Guard {
    #[serde(rename = "pragma")]
    Pragma,
    #[serde(rename = "name")]
    Name(String),
}

impl Guard {
    fn get_guard(&self) -> String {
        match self {
            Guard::Pragma => "#pragma once".to_string(),
            Guard::Name(n) => format!("#ifndef {}\n#define {}", n, n),
        }
    }

    fn get_footer(&self) -> String {
        match self {
            Guard::Pragma => String::new(),
            Guard::Name(_) => "#endif".to_string(),
        }
    }
}

#[derive(Default, Deserialize)]
pub struct CppProps {
    #[serde(rename = "guard")]
    guard: Option<Guard>,

    #[serde(rename = "namespace")]
    namespace: Option<String>,
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
            Some(ns) => format!("namespace {} {{", ns),
            None => String::new(),
        }
    }

    pub fn close_namespace(&self) -> String {
        match &self.namespace {
            Some(ns) => format!("}} // namespace {}", ns),
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
}

// ------
#[cfg(test)]
mod tests {
    use crate::props::{CppProps, Guard};

    #[test]
    fn default() {
        let props = CppProps::default();
        assert_eq!(props.guard.is_none(), true);
        assert_eq!(props.namespace.is_none(), true);
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
}
