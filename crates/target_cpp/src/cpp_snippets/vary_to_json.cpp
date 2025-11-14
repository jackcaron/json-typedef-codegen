
  template<> struct Serialize<$FULL_NAME$> {
    static ExpType<void> serialize(Writer::Serializer& serializer, const $FULL_NAME$& value) {
      using Struct = $FULL_NAME$;
  $WRITE_PROPS$    return ExpType<void>();
    }
  };
