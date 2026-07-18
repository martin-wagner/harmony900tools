// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <optional>
#include <vector>
#include <string>

#include <QByteArray>
#include <nlohmann/json.hpp>

#include "document/data/enum.h"
#include "document/data/property.h"
#include "unknown.h"

namespace document
{
namespace data
{
namespace serialiser
{

using nlohmann::ordered_json;

/** base64-encodes a byte vector using Qt's QByteArray */
inline std::string base64Encode(const std::vector<uint8_t> &data)
{
  QByteArray bytes(reinterpret_cast<const char*>(data.data()),
      int(data.size()));
  return bytes.toBase64().toStdString();
}

/** decodes a base64 string into a byte vector using Qt's QByteArray */
inline std::vector<uint8_t> base64Decode(const std::string &in)
{
  QByteArray decoded = QByteArray::fromBase64(QByteArray::fromStdString(in));
  return std::vector<uint8_t>(decoded.begin(), decoded.end());
}

/** writes a plain Property<T> if it is marked as used. skipped otherwise. */
template<typename T>
void toJson(ordered_json &out, const char *key, const Property<T> &prop)
{
  if (prop.isIncluded() == Used::YES) {
    out[key] = prop.get();
  }
}

/** reads a plain Property<T>. sets used to YES if key is present, NO otherwise. */
template<typename T>
void fromJson(const ordered_json &in, const char *key, Property<T> &prop)
{
  auto it = in.find(key);
  if (it != in.end()) {
    prop.set(it->get<T>());
    prop.setIncluded(Used::YES);
  } else {
    prop.setIncluded(Used::NO);
  }
}

/** writes a PropertyEnum<T> as its string representation, if used. */
template<typename T>
void toJson(ordered_json &out, const char *key, const PropertyEnum<T> &prop)
{
  if (prop.isIncluded() == Used::YES) {
    out[key] = prop.get().getString();
  }
}

/** reads a PropertyEnum<T> from its string representation. */
template<typename T>
void fromJson(const ordered_json &in, const char *key, PropertyEnum<T> &prop)
{
  auto it = in.find(key);
  if (it != in.end()) {
    prop.set(Enum<T>(it->get<std::string>()));
    prop.setIncluded(Used::YES);
  } else {
    prop.setIncluded(Used::NO);
  }
}

/** writes a plain (non-Property-wrapped) Enum<T> as its string representation. always written -- no Used gating. */
template<typename T>
void toJson(ordered_json &out, const char *key, const Enum<T> &value)
{
  out[key] = value.getString();
}

/** reads a plain (non-Property-wrapped) Enum<T> from its string representation. leaves value unchanged if key is missing. */
template<typename T>
void fromJson(const ordered_json &in, const char *key, Enum<T> &value)
{
  auto it = in.find(key);
  if (it != in.end()) {
    value = Enum<T>(it->get<std::string>());
  }
}

/** writes a single UnknownElement, recursing into children */
inline void toJson(ordered_json &out, const item::UnknownElement &elem)
{
  out["tag"] = elem.tag;
  out["attributes"] = elem.attributes;
  out["text"] = elem.text;

  if (!elem.children.empty()) {
    ordered_json children = ordered_json::array();
    for (const auto &child : elem.children) {
      ordered_json childJson;
      toJson(childJson, child);
      children.push_back(childJson);
    }
    out["children"] = children;
  }
}

/** reads a single UnknownElement, recursing into children */
inline void fromJson(const ordered_json &in, item::UnknownElement &elem)
{
  elem.tag = in.value("tag", "");
  elem.attributes = in.value("attributes",
      std::map<std::string, std::string>());
  elem.text = in.value("text", "");

  elem.children.clear();
  auto it = in.find("children");
  if (it != in.end()) {
    for (const auto &childJson : *it) {
      item::UnknownElement child;
      fromJson(childJson, child);
      elem.children.push_back(child);
    }
  }
}

/** writes a vector of UnknownElement under key, if non-empty */
inline void toJson(ordered_json &out, const char *key,
    const std::vector<item::UnknownElement> &elements)
{
  if (elements.empty()) {
    return;
  }

  ordered_json arr = ordered_json::array();
  for (const auto &elem : elements) {
    ordered_json elemJson;
    toJson(elemJson, elem);
    arr.push_back(elemJson);
  }
  out[key] = arr;
}

/** reads a vector of UnknownElement from key. clears/empties if key is missing. */
inline void fromJson(const ordered_json &in, const char *key,
    std::vector<item::UnknownElement> &elements)
{
  elements.clear();

  auto it = in.find(key);
  if (it == in.end()) {
    return;
  }

  for (const auto &elemJson : *it) {
    item::UnknownElement elem;
    fromJson(elemJson, elem);
    elements.push_back(elem);
  }
}

/** writes std::optional<T> under key if it has a value. T must have a toJson(ordered_json&, const T&) overload. */
template<typename T>
void toJsonOpt(ordered_json &out, const char *key, const std::optional<T> &opt)
{
  if (opt.has_value()) {
    ordered_json inner;
    toJson(inner, opt.value());
    out[key] = inner;
  }
}

/** reads std::optional<T> from key. resets to nullopt if key is missing. T must have a fromJson(const ordered_json&, T&) overload and be default-constructible. */
template<typename T>
void fromJsonOpt(const ordered_json &in, const char *key, std::optional<T> &opt)
{
  auto it = in.find(key);
  if (it == in.end()) {
    opt.reset();
    return;
  }

  T value;
  fromJson(*it, value);
  opt = value;
}

/** writes std::vector<T> under key if non-empty. T must have a toJson(ordered_json&, const T&) overload. */
template<typename T>
void toJsonVec(ordered_json &out, const char *key, const std::vector<T> &vec)
{
  if (vec.empty()) {
    return;
  }

  ordered_json arr = ordered_json::array();
  for (const auto &item : vec) {
    ordered_json itemJson;
    toJson(itemJson, item);
    arr.push_back(itemJson);
  }
  out[key] = arr;
}

/** reads std::vector<T> from key. clears if key is missing. T must have a fromJson(const ordered_json&, T&) overload and be default-constructible. */
template<typename T>
void fromJsonVec(const ordered_json &in, const char *key, std::vector<T> &vec)
{
  vec.clear();

  auto it = in.find(key);
  if (it == in.end()) {
    return;
  }

  for (const auto &itemJson : *it) {
    T value;
    fromJson(itemJson, value);
    vec.push_back(value);
  }
}

}
}
}
