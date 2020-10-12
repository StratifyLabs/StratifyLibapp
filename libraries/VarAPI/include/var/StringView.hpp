/*! \file */ // Copyright 2011-2020 Tyler Gilbert and Stratify Labs, Inc; see
             // LICENSE.md for rights.
#ifndef VAR_API_STRING_VIEW_HPP
#define VAR_API_STRING_VIEW_HPP

#include <cstring>
#include <string>
#include <string_view>

#include "api/api.hpp"

#include "Vector.hpp"

namespace var {

class String;
class Data;

class StringView {
public:
  constexpr static size_t npos = std::string_view::npos;

  enum class Base { octal = 8, decimal = 10, hexidecimal = 16 };

  StringView() : m_string_view("") {}
  StringView(const char *value) {
    API_ASSERT(value != nullptr);
    m_string_view = std::string_view(value);
  }

  bool is_null() const { return m_string_view.data() == nullptr; }

  StringView &set_null() {
    m_string_view = std::string_view(nullptr, 0);
    return *this;
  }

  char at(size_t value) const { return m_string_view.at(value); }
  char front() const { return m_string_view.front(); }
  char back() const { return m_string_view.back(); }
  size_t length() const { return m_string_view.length(); }

  bool is_empty() const { return m_string_view.empty(); }
  StringView &pop_front(size_t length = 1) {
    m_string_view.remove_prefix(length);
    return *this;
  }

  class GetSubstring {
    API_AF(GetSubstring, size_t, position, 0);
    API_AF(GetSubstring, size_t, length, npos);
  };

  String get_substring(const GetSubstring &options) const;

  String get_substring_at_position(size_t position) const;

  String get_substring_with_length(size_t length) const;
  var::Vector<String> split(StringView delimeters) const;

  using iterator = typename std::string_view::iterator;
  using const_iterator = typename std::string_view::const_iterator;
  using reverse_iterator = typename std::string_view::reverse_iterator;
  using const_reverse_iterator =
    typename std::string_view::const_reverse_iterator;

  const_iterator begin() const noexcept { return m_string_view.begin(); }
  iterator begin() noexcept { return m_string_view.begin(); }

  const_iterator end() const noexcept { return m_string_view.end(); }
  iterator end() noexcept { return m_string_view.end(); }

  const_iterator cbegin() const noexcept { return m_string_view.cbegin(); }
  const_iterator cend() const noexcept { return m_string_view.cend(); }

  const_reverse_iterator rbegin() const noexcept {
    return m_string_view.rbegin();
  }
  reverse_iterator rbegin() noexcept { return m_string_view.rbegin(); }

  const_reverse_iterator rend() const noexcept { return m_string_view.rend(); }
  reverse_iterator rend() noexcept { return m_string_view.rend(); }

  const_reverse_iterator crbegin() const noexcept {
    return m_string_view.crbegin();
  }
  const_reverse_iterator crend() const noexcept {
    return m_string_view.crend();
  }

  const char *cstring() const { return m_string_view.data(); }

  size_t find(StringView a, size_t position = 0) const {
    return m_string_view.find(a.string_view(), position);
  }

  size_t find(char a, size_t position = 0) const {
    return m_string_view.find(a, position);
  }

  size_t find_first_of(StringView a, size_t position = 0) const {
    return m_string_view.find_first_of(a.string_view(), position);
  }

  size_t find_first_not_of(StringView a, size_t position = 0) const {
    return m_string_view.find_first_not_of(a.string_view(), position);
  }

  size_t reverse_find(StringView a, size_t position = npos) const {
    return m_string_view.rfind(a.string_view(), position);
  }

  size_t reverse_find(char a, size_t position = npos) const {
    return m_string_view.rfind(a, position);
  }

  size_t find_last_of(StringView a, size_t position = 0) const {
    return m_string_view.find_last_of(a.string_view(), position);
  }

  size_t find_last_not_of(StringView a, size_t position = 0) const {
    return m_string_view.find_last_not_of(a.string_view(), position);
  }

  const std::string_view &string_view() const { return m_string_view; }
  std::string_view &string_view() { return m_string_view; }

  bool operator==(StringView a) const {
    return a.string_view() == string_view();
  }

  bool operator!=(StringView a) const {
    return a.string_view() != string_view();
  }

  long to_long(Base base = Base::decimal) const {
    return ::strtol(cstring(), nullptr, static_cast<int>(base));
  }

  float to_float() const;

  unsigned long to_unsigned_long(Base base = Base::decimal) const {
    return ::strtoul(cstring(), nullptr, static_cast<int>(base));
  }

  bool operator==(const char *a) const { return StringView(a) == *this; }
  bool operator!=(const char *a) const { return StringView(a) != *this; }

private:
  std::string_view m_string_view;
};

class NumberToString {
public:
  template <typename T> NumberToString(T value, const char *fmt = nullptr) {

    // guarantee null termination
    m_buffer[sizeof(m_buffer) - 1] = 0;

    if (fmt == nullptr) {

      if (std::is_integral<T>::value == true) {

#if defined __link
        strncpy(m_buffer, std::to_string(value).c_str(), sizeof(m_buffer) - 1);
#else
        if (std::is_signed<T>::value == true) {
          snprintf(m_buffer, sizeof(m_buffer) - 1, F32D, value);
        } else {
          snprintf(m_buffer, sizeof(m_buffer) - 1, F32U, value);
        }
#endif
      } else if (std::is_floating_point<T>::value == true) {
#if 0
        static_assert(
          sizeof(T) > 4,
          "Cannot convert double types to string using NumberToString");
#endif

        // snprintf(m_buffer, sizeof(m_buffer) - 1, "%f", value);
      }

      return;
    }

    snprintf(m_buffer, sizeof(m_buffer) - 1, fmt, value);
  }

  operator StringView() { return StringView(m_buffer); }
  const char *cstring() const { return m_buffer; }

private:
  char m_buffer[64];
};

using Ntos = NumberToString;

} // namespace var

#endif // VAR_API_STRING_VIEW_HPP
