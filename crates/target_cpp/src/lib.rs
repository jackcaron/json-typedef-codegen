pub mod props;
mod state;

use jtd_codegen::target::{self, inflect, metadata};
use lazy_static::lazy_static;
use props::CppProps;
use std::collections::BTreeSet;

lazy_static! {
    static ref KEYWORDS: BTreeSet<String> = include_str!("keywords")
        .lines()
        .map(str::to_owned)
        .collect();
    static ref TYPE_NAMING_CONVENTION: Box<dyn inflect::Inflector + Send + Sync> =
        Box::new(inflect::KeywordAvoidingInflector::new(
            KEYWORDS.clone(),
            inflect::CombiningInflector::new(inflect::Case::pascal_case())
        ));
    static ref FIELD_NAMING_CONVENTION: Box<dyn inflect::Inflector + Send + Sync> =
        Box::new(inflect::KeywordAvoidingInflector::new(
            KEYWORDS.clone(),
            inflect::TailInflector::new(inflect::Case::snake_case())
        ));
    static ref ENUM_MEMBER_NAMING_CONVENTION: Box<dyn inflect::Inflector + Send + Sync> =
        Box::new(inflect::KeywordAvoidingInflector::new(
            KEYWORDS.clone(),
            inflect::TailInflector::new(inflect::Case::pascal_case())
        ));
}

pub struct Target {
    props: CppProps,
    root_name: String,
}

impl Target {
    pub fn new(props: CppProps, root_name: String) -> Self {
        Self { props, root_name }
    }
}

impl jtd_codegen::target::Target for Target {
    type FileState = state::CppState;

    fn strategy(&self) -> target::Strategy {
        let fname = format!("{}.cpp", self.root_name);
        target::Strategy {
            file_partitioning: target::FilePartitioningStrategy::SingleFile(fname),
            enum_member_naming: target::EnumMemberNamingStrategy::Unmodularized,
            optional_property_handling: target::OptionalPropertyHandlingStrategy::WrapWithNullable,
            booleans_are_nullable: false,
            int8s_are_nullable: false,
            uint8s_are_nullable: false,
            int16s_are_nullable: false,
            uint16s_are_nullable: false,
            int32s_are_nullable: false,
            uint32s_are_nullable: false,
            float32s_are_nullable: false,
            float64s_are_nullable: false,
            strings_are_nullable: false,
            timestamps_are_nullable: false,
            arrays_are_nullable: false,
            dicts_are_nullable: false,
            aliases_are_nullable: false,
            enums_are_nullable: false,
            structs_are_nullable: false,
            discriminators_are_nullable: false,
        }
    }

    fn name(&self, kind: target::NameableKind, name_parts: &[String]) -> String {
        todo!()
    }

    fn expr(
        &self,
        state: &mut Self::FileState,
        metadata: metadata::Metadata,
        expr: target::Expr,
    ) -> String {
        todo!()
    }

    fn item(
        &self,
        out: &mut dyn std::io::Write,
        state: &mut Self::FileState,
        item: target::Item,
    ) -> jtd_codegen::Result<Option<String>> {
        todo!()
    }

    fn conclude(&self) -> jtd_codegen::Result<()> {
        // write the header file here
        Ok(())
    }
}
