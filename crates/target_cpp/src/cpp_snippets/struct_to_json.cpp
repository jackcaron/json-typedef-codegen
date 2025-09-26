
  template<> struct Serialize<$FULL_NAME$> {
    static ExpType<void> serialize(Writer::Serializer& serializer, const $FULL_NAME$& value) {
      using Struct = $FULL_NAME$;
      SHORT_EXP(serializer.start_object());
$WRITE_PROPS$      return serializer.end_object();
    }
  };
