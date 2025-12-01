#pragma once

#include <iomanip>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>

#include "utils.h"

using RowVariant = std::variant<int64_t, double, std::string>;

struct Row {
  std::unordered_map<std::string, RowVariant> data;

  template <typename T>
  T get(const std::string& column_name) const {
    auto it{data.find(column_name)};
    if (it == data.end()) {
      return Utils::get_null<T>();
    }

    return (*std::get_if<T>(&it->second));
  }

  const RowVariant& operator[](const std::string& column_name) const {
    return data.at(column_name);
  }

  RowVariant& operator[](const std::string& column_name) {
    return data.at(column_name);
  }

  friend std::ostream& operator<<(std::ostream& os, const Row& row) {
    size_t count{};

    os << "{ ";

    for (const auto& [key, value] : row.data) {
      os << key << ": ",

          std::visit(
              [&os](const auto& val) {
                if (Utils::is_null(val)) {
                  os << "NULL";
                } else {
                  os << val;
                }
              },
              value);

      ++count;
      if (count < row.data.size()) {
        os << ", ";
      }
    }

    os << " }";
    return os;
  }
};