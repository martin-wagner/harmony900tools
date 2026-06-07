// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <cstdint>
#include <string_view>

namespace lib
{

constexpr uint32_t hash_fnv1a(const char *str, std::size_t n)
{
  uint32_t hash = 2166136261u;
  for (std::size_t i = 0; i < n; ++i) {
    hash ^= static_cast<uint8_t>(str[i]);
    hash *= 16777619u;
  }
  return hash;
}

}

//in global namespace
constexpr uint32_t operator"" _hash(const char *str, std::size_t n)
{
  return lib::hash_fnv1a(str, n);
}

/*
 * use:
string name = "test";
const uint32_t h = fnv1a(name.data(), name.size());
switch (h) {
  case "text"_hash:
    break;
  default:
    break;
}
*/
