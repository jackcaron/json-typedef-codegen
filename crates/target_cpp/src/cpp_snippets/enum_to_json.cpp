
  ExpType<void> serialize(Writer::Serializer& serializer, const $FULL_NAME$ value) {
    using Enum = $FULL_NAME$;
    const auto strvalue = ([value](){
          switch (value) {
            default:
$CLAUSES$          }
        })();
    return serializer.write_str(strvalue);
  }