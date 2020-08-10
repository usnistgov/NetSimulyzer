#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

namespace util::json {

class JsonValue;

/**
 * Representation of a JSON array
 */
using JsonArray = std::vector<JsonValue>;

/**
 * Representation of a JSON object
 */
class JsonObject {
  std::unordered_map<std::string, std::shared_ptr<JsonValue>> members;

public:
  /**
   * Get a member of this object
   *
   * @param key
   * The name of the member
   *
   * @return
   * The `JsonValue` for 'key'
   */
  JsonValue &get(const std::string &key) {
    return *members[key];
  }

  /**
   * Get a member of this object.
   * const variant of `get`
   *
   * @param key
   * The name of the member
   *
   * @return
   * The `JsonValue` for 'key'
   */
  const JsonValue &get(const std::string &key) const {
    // operator[] on unordered map is not const...
    return *members.at(key);
  }

  /**
   * Alias for JsonValue &get(key)
   *
   * @param key
   * The name of the member
   *
   * @return
   * The `JsonValue` for 'key'
   *
   * @see get()
   */
  JsonValue &operator[](const std::string &key) {
    return get(key);
  }

  /**
   * Alias for JsonValue &get(key) const
   *
   * @param key
   * The name of the member
   *
   * @return
   * The `JsonValue` for 'key'
   *
   * @see get()
   */
  const JsonValue &operator[](const std::string &key) const {
    return get(key);
  }

  /**
   * Checks if the object contains a member with the name `key`
   *
   * @param key
   * The name of the member to check for
   *
   * @return
   * True if the member exists, False otherwise
   */
  bool contains(const std::string &key) const {
    return members.find(key) != members.end();
  }

  /**
   * Inserts a new key/value pair into this object.
   * Will overwrite a member with the same name
   *
   * @param key
   * The name for the new member
   *
   * @param value
   * The value of the member
   */
  void insert(const std::string &key, const JsonValue &value) {
    members[key] = std::make_shared<JsonValue>(value);
  }
};

/**
 * Representation of some value in a JSON document
 */
class JsonValue {
  /**
   * Actual contained value.
   * signed Integer types are boxed to `long long`
   * unsigned Integer types are boxed to `unsigned long long`
   * and floating point types are boxed to `double`.
   */
  std::variant<std::nullptr_t, bool, long long, unsigned long long, double, std::string, JsonArray, JsonObject> value;

public:
  /**
   * Constructs a null value
   */
  JsonValue() = default;

  /**
   * Explicit null constructor.
   *
   * @see setNull()
   */
  JsonValue(std::nullptr_t) : value(nullptr) {
  }

  /**
   * Constructs a boolean value
   *
   * @param value
   * The bool value to store
   */
  JsonValue(bool value) : value(value) {
  }

  // Box integer types into `long long`

  /**
   * Constructs an integer value
   *
   * @param value
   * The int value to store
   */
  JsonValue(int value) : value(static_cast<long long>(value)) {
  }

  /**
   * Constructs an unsigned integer value
   *
   * @param value
   * The unsigned int value to store
   */
  JsonValue(unsigned int value) : value(static_cast<unsigned long long>(value)) {
  }

  /**
   * Constructs a long value
   *
   * @param value
   * The long value to store
   */
  JsonValue(long value) : value(static_cast<long long>(value)) {
  }

  /**
   * Constructs an unsigned long value
   *
   * @param value
   * The unsigned long value to store
   */
  JsonValue(unsigned long value) : value(static_cast<unsigned long long>(value)) {
  }

  /**
   * Constructs a long long value
   *
   * @param value
   * The long long value to store
   */
  JsonValue(long long value) : value(value){};

  /**
   * Constructs an unsigned long long value
   *
   * @param value
   * The long long value to store
   */
  JsonValue(unsigned long long value) : value(value){};

  // No unsigned long long

  /**
   * Constructs a double value
   *
   * @param value
   * The double value to store
   */
  JsonValue(double value) : value(value) {
  }

  // Box floats into doubles
  /**
   * Constructs a float value
   *
   * @param value
   * The float value to store
   */
  JsonValue(float value) : value(static_cast<double>(value)) {
  }

  /**
   * Constructs a string value
   *
   * @param value
   * The std::string value to store
   */
  JsonValue(const std::string &value) : value(value) {
  }

  /**
   * Constructs an object value
   *
   * @param object
   * The object to store
   */
  JsonValue(const JsonObject &object) : value(object) {
  }

  /**
   * Constructs an array value
   *
   * @param array
   * The array to store
   */
  JsonValue(const JsonArray &array) : value(array) {
  }

  /**
   * Replaces the current value with `nullptr`
   */
  void setNull() {
    value.emplace<std::nullptr_t>();
  }

  /**
   * Checks if the value is of type `T`
   *
   * @tparam T
   * The type to match against the contained value
   *
   * @return
   * True if the contained type is exactly `T` (no conversions allowed),
   * False otherwise
   */
  template <class T>
  [[nodiscard]] bool is() const {
    return std::holds_alternative<T>(value);
  }

  /**
   * Checks if the contained value is null
   *
   * @return
   * True if the contained value is null,
   * False otherwise
   */
  [[nodiscard]] bool isNull() const {
    return std::holds_alternative<std::nullptr_t>(value);
  }

  /**
   * Checks if the contained value is an object
   *
   * @return
   * True if the contained value is an object,
   * False otherwise
   */
  [[nodiscard]] bool isObject() const {
    return std::holds_alternative<JsonObject>(value);
  }

  /**
   * Checks if the contained value is an array
   *
   * @return
   * True if the contained value is an array,
   * False otherwise
   */
  [[nodiscard]] bool isArray() const {
    return std::holds_alternative<JsonArray>(value);
  }

  /**
   * Retrieves the contained value as type `T`
   * This method _will_ perform conversions for
   * arithmetic types. (numbers).
   *
   * @tparam T
   * The type of value to retrieve.
   * Must exactly match the contained type,
   * or be arithmetic.
   *
   * @return
   * The contained value as type `T`
   *
   *
   * @throws std::bad_variant_access
   * If the specified type is not the type
   * of the contained value.
   *
   * Does not apply to arithmetic types
   */
  template <class T>
  [[nodiscard]] T get() const {
    if constexpr (std::is_arithmetic_v<T>) {

      if (std::holds_alternative<long long>(value))
        return static_cast<T>(std::get<long long>(value));
      else if (std::holds_alternative<unsigned long long>(value))
        return static_cast<T>(std::get<unsigned long long>(value));
      else if (std::holds_alternative<double>(value))
        return static_cast<T>(std::get<double>(value));

      std::cerr << "Requested conversion to numeric type, but contained type is not numeric!\n"
                << "Variant index: " << value.index() << '\n';
      std::abort();
    } else // Directly extract all remaining types
      return std::get<T>(value);
  }

  /**
   * Retrieved the contained object.
   *
   * @throws std::bad_variant_access
   * If the contained values is not an object
   *
   * @return
   * The contained object
   */
  [[nodiscard]] JsonObject &object() {
    return std::get<JsonObject>(value);
  }

  /**
   * Retrieved the contained object.
   *
   * @throws std::bad_variant_access
   * If the contained values is not an object
   *
   * @return
   * The contained object
   */
  [[nodiscard]] const JsonObject &object() const {
    return std::get<JsonObject>(value);
  }

  /**
   * Retrieved the contained array.
   *
   * @throws std::bad_variant_access
   * If the contained values is not an array
   *
   * @return
   * The contained array
   */
  [[nodiscard]] JsonArray &array() {
    return std::get<JsonArray>(value);
  }

  /**
   * Retrieved the contained array.
   *
   * @throws std::bad_variant_access
   * If the contained values is not an array
   *
   * @return
   * The contained array
   */
  [[nodiscard]] const JsonArray &array() const {
    return std::get<JsonArray>(value);
  }
};

// Specialize `bool` since that falls under `is_arithmetic`
template <>
[[nodiscard]] inline bool JsonValue::get() const {
  // Some other type
  return std::get<bool>(value);
}

} // namespace util::json
