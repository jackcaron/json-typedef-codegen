#[derive(Debug, PartialEq)]
pub struct CppAlias {
    name: String,
    sub_type: String,
}

impl CppAlias {
    pub fn new(name: &str, sub_type: &str) -> CppAlias {
        CppAlias {
            name: name.to_string(),
            sub_type: sub_type.to_string(),
        }
    }

    pub fn format(&self) -> String {
        format!("using {} = {};\n", self.name, self.sub_type)
    }

    pub fn sub_matches(&self, sub_type: &str) -> bool {
        self.sub_type == sub_type
    }

    pub fn get_name<'a>(&'a self) -> &'a str {
        &self.name
    }
}
