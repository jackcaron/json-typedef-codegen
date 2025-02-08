
  template<>
  struct FromJson<FULL_NAME> {{
    static ExpType<FULL_NAME> convert(const Reader::JsonValue& value) {{
      return READ_PRIM_VALUEREADER_XFORM;
    }}
    static ExpType<FULL_NAME> convert(const Data::JsonValue& value) {{
      return optionalToExpType(READ_PRIM_VALUE, JsonErrorTypes::Invalid, "not a EXP_TYPE"sv)DATA_XFORM;
    }}
  }};
