#include "json_writer.hpp"
#include "internal.hpp"
#include "spec_writer.hpp"

using namespace std::string_view_literals;

namespace JsonTypedefCodeGen::Writer {

  namespace Specialization {

    BaseSerializer::~BaseSerializer() {}
    AbsSerializer::~AbsSerializer() {}
    Serializer BaseSerializer::create(SerializerPtr&& pimpl) {
      return Serializer(std::move(pimpl));
    }

    const AbsSerializer* unbase(const SerializerPtr& base) {
      return dynamic_cast<const AbsSerializer*>(base.get());
    }
    AbsSerializer* unbase(SerializerPtr& base) {
      return dynamic_cast<AbsSerializer*>(base.get());
    }

  } // namespace Specialization

  namespace Spec = Specialization;

  // ------------------------------------------
  static UnexpJsonError noPimpl() {
    return make_json_error(JsonErrorTypes::Invalid,
                           "invalid/empty Serializer"sv);
  }

  Serializer::Serializer(Specialization::SerializerPtr&& pimpl)
      : m_pimpl(std::move(pimpl)) {}

  DLL_PUBLIC ExpType<bool> Serializer::write_null() {
    return m_pimpl ? Spec::unbase(m_pimpl)->write_null() : noPimpl();
  }
  DLL_PUBLIC ExpType<bool> Serializer::write_bool(const bool b) {
    return m_pimpl ? Spec::unbase(m_pimpl)->write_bool(b) : noPimpl();
  }
  DLL_PUBLIC ExpType<bool> Serializer::write_double(const double d) {
    return m_pimpl ? Spec::unbase(m_pimpl)->write_double(d) : noPimpl();
  }
  DLL_PUBLIC ExpType<bool> Serializer::write_i64(const int64_t i) {
    return m_pimpl ? Spec::unbase(m_pimpl)->write_i64(i) : noPimpl();
  }
  DLL_PUBLIC ExpType<bool> Serializer::write_u64(const uint64_t u) {
    return m_pimpl ? Spec::unbase(m_pimpl)->write_u64(u) : noPimpl();
  }
  DLL_PUBLIC ExpType<bool> Serializer::write_str(const std::string_view str) {
    return m_pimpl ? Spec::unbase(m_pimpl)->write_str(str) : noPimpl();
  }

  DLL_PUBLIC ExpType<bool> Serializer::start_object() {
    return m_pimpl ? Spec::unbase(m_pimpl)->start_object() : noPimpl();
  }
  DLL_PUBLIC ExpType<bool> Serializer::end_object() {
    return m_pimpl ? Spec::unbase(m_pimpl)->start_object() : noPimpl();
  }

  DLL_PUBLIC ExpType<bool> Serializer::start_array() {
    return m_pimpl ? Spec::unbase(m_pimpl)->start_object() : noPimpl();
  }
  DLL_PUBLIC ExpType<bool> Serializer::end_array() {
    return m_pimpl ? Spec::unbase(m_pimpl)->start_object() : noPimpl();
  }

} // namespace JsonTypedefCodeGen::Writer
