pub mod alias;
pub mod containers;
pub mod cpp_enum;
pub mod cpp_struct;
pub mod disc;
pub mod primitives;
mod shared;

use crate::props::CppProps;
use crate::state::CppState;
pub use alias::CppAlias;
pub use containers::*;
pub use cpp_enum::CppEnum;
pub use cpp_struct::CppStruct;
pub use disc::{CppDiscriminator, CppDiscriminatorVariant};
pub use primitives::Primitives;
use shared::{get_complete_definition, prototype_name};

#[derive(Debug, PartialEq)]
pub enum CppTypes {
    Incomplete,
    Primitive(Primitives),
    Array(CppArray),
    Dictionary(CppDict),
    Nullable(CppNullable),
    Alias(CppAlias),
    Enum(CppEnum),
    Struct(CppStruct),
    DiscriminatorVariant(CppDiscriminatorVariant),
    Discriminator(CppDiscriminator),
}

impl CppTypes {
    pub fn type_prefix(&self) -> Option<&'static str> {
        match &self {
            Self::Enum(_) => Some(CppEnum::get_prefix()),
            Self::Struct(_) => Some(CppStruct::get_prefix()),
            Self::Discriminator(_) => Some(CppDiscriminator::get_prefix()),
            Self::DiscriminatorVariant(_) => Some(CppDiscriminatorVariant::get_prefix()),
            _ => None,
        }
    }

    pub fn declare(&self, _cpp_state: &CppState, _cpp_props: &CppProps) -> Option<String> {
        match &self {
            CppTypes::Enum(_enum) => Some(_enum.declare()),
            CppTypes::Struct(_struct) => Some(_struct.declare()),
            CppTypes::Discriminator(disc) => Some(disc.declare()),
            CppTypes::DiscriminatorVariant(vary) => Some(vary.declare()),
            _ => None,
        }
    }

    pub fn prototype(&self, cpp_state: &CppState, cpp_props: &CppProps) -> Option<String> {
        match &self {
            CppTypes::Enum(_enum) => Some(_enum.prototype(cpp_props)),
            CppTypes::Struct(_struct) => Some(_struct.prototype(cpp_props)),
            CppTypes::Discriminator(disc) => Some(disc.prototype(cpp_props)),
            CppTypes::DiscriminatorVariant(vary) => Some(vary.prototype(cpp_props)),
            CppTypes::Nullable(null) => Some(null.prototype(cpp_props, cpp_state)),
            CppTypes::Alias(alias) => Some(prototype_name(&alias.get_name(), cpp_props)),
            _ => None,
        }
    }

    pub fn define(&self, cpp_state: &CppState, cpp_props: &CppProps) -> Option<String> {
        match &self {
            CppTypes::Enum(_enum) => Some(get_complete_definition(&_enum.get_name(), cpp_props)),
            CppTypes::Struct(_struct) => {
                Some(get_complete_definition(&_struct.get_name(), cpp_props))
            }
            CppTypes::Discriminator(disc) => {
                Some(get_complete_definition(&disc.get_name(), cpp_props))
            }
            CppTypes::DiscriminatorVariant(vary) => {
                Some(get_complete_definition(&vary.get_name(), cpp_props))
            }
            CppTypes::Nullable(null) => Some(null.define(cpp_props, cpp_state)),
            CppTypes::Alias(alias) => Some(get_complete_definition(&alias.get_name(), cpp_props)),
            _ => None,
        }
    }

    pub fn get_common_internal_code(
        &self,
        _cpp_state: &CppState,
        cpp_props: &CppProps,
    ) -> Option<String> {
        match self {
            CppTypes::Enum(_enum) => Some(_enum.get_common_internal_code(cpp_props)),
            CppTypes::Struct(_struct) => Some(_struct.get_common_internal_code(cpp_props)),
            CppTypes::Discriminator(disc) => Some(disc.get_common_internal_code(cpp_props)),
            CppTypes::DiscriminatorVariant(vary) => Some(vary.get_common_internal_code(cpp_props)),
            _ => None,
        }
    }

    pub fn get_des_internal_code(
        &self,
        _cpp_state: &CppState,
        cpp_props: &CppProps,
    ) -> Option<String> {
        match self {
            CppTypes::Enum(_enum) => Some(_enum.get_des_internal_code(cpp_props)),
            CppTypes::Struct(_struct) => Some(_struct.get_des_internal_code(cpp_props)),
            CppTypes::Discriminator(disc) => Some(disc.get_des_internal_code(cpp_props)),
            CppTypes::DiscriminatorVariant(vary) => Some(vary.get_des_internal_code(cpp_props)),
            _ => None,
        }
    }

    pub fn get_ser_internal_code(
        &self,
        _cpp_state: &CppState,
        cpp_props: &CppProps,
    ) -> Option<String> {
        match self {
            CppTypes::Enum(_enum) => Some(_enum.get_ser_internal_code(cpp_props)),
            CppTypes::Struct(_struct) => Some(_struct.get_ser_internal_code(cpp_props)),
            CppTypes::Discriminator(disc) => Some(disc.get_ser_internal_code(cpp_props)),
            CppTypes::DiscriminatorVariant(vary) => Some(vary.get_ser_internal_code(cpp_props)),
            _ => None,
        }
    }
}
