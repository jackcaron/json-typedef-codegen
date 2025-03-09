
  template<>
  struct FromJson<$FULL_NAME$> {
    static ExpType<$FULL_NAME$> deserialize(const Reader::JsonValue& value) {
      return $READ_PRIM_VALUE$$READER_XFORM$;
    }
    static ExpType<$FULL_NAME$> deserialize(const Data::JsonValue& value) {
      return optional_to_exp_type($READ_PRIM_VALUE$, JsonErrorTypes::Invalid, "not a $EXP_TYPE$"sv)$DATA_XFORM$;
    }
  };
