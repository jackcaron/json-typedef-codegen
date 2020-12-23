using System;
using System.Text.Json;
using System.Text.Json.Serialization;
namespace JtdCodegenE2E
{
    [JsonConverter(typeof(Foo.JsonConverter))]
    public class Foo
    {
        public Bar Value { get; set; }
        public class JsonConverter : JsonConverter<Foo>
        {
            public override Foo Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
            {
                return new Foo { Value = JsonSerializer.Deserialize<Bar>(ref reader, options) };
            }
            public override void Write(Utf8JsonWriter writer, Foo value, JsonSerializerOptions options)
            {
                JsonSerializer.Serialize<Bar>(writer, value.Value, options);
            }
        }
    }
}
