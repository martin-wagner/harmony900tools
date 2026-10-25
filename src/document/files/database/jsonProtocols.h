// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <QHash>
#include <QString>
#include <QVariant>

#include <nlohmann/json.hpp>

#include "bin/irProto/code.h"
#include "bin/irProto/file.h"
#include "database.h"
#include "keyCode.h"

namespace document
{

namespace files
{

/**
 * Parse protocols and key codes from the database json file
 *
 * Protocol sections are the "IRSegments" of the protocol (same index as used in
 * the key codes), followed by the data-less "CodeSegments" ("<name> KeyCodeRepeat" ...).
 * Words in a key code ("Repeat", "Space", ...) select the CodeSegment of the same name.
 */
class ProtocolDatabase {
  public:
    ProtocolDatabase();

    /** load file. returns false on error, see getErrStr() */
    bool setDatabase(const DatabasePaths &paths);

    /** get protocol by "Id-" value. check prot.isEmpty() for failure.
     * name (optional) receives the protocol name */
    binary::irProto::IrProto getProtocol(uint32_t id, std::string *name = nullptr);

    /** create code from a keycode line. check code.isValid() for failure */
    binary::irProto::Code getCode(const QString &keyCodeLine);

    /** get last error. every call of the functions above resets it */
    QString getErrStr() const { return errStr; }

  private:
    /** protocol plus the lookup for words used in key codes */
    struct CachedProtocol {
      binary::irProto::IrProto proto;
      QString name;
      //lower case word ("repeat", "space", ...) -> section index
      QHash<QString, uint8_t> keywords;
    };

    /** longest timing a single item can hold */
    static constexpr uint32_t MAX_SINGLE_TIMING = 0x7fff;
    /** item count is stored in one byte */
    static constexpr size_t MAX_ITEMS = 255;
    /** section index is stored in one byte */
    static constexpr size_t MAX_SECTIONS = 255;

    void clear();
    bool loadFailed(const QString &message);
    static QString normalizeName(const QString &name);
    uint32_t findId(const QString &name) const;
    const CachedProtocol *loadProtocol(uint32_t id);
    bool buildProtocol(const nlohmann::json &jp, CachedProtocol &entry);
    static QString getKeyword(const QString &protocolName, const QString &segmentName);
    static QVariantList elementsOf(const QVariant &item);
    static void appendItems(const nlohmann::json &atom, std::vector<binary::irProto::Item> &out);
    bool buildHeader(const nlohmann::json &atoms, binary::irProto::TimingSectionIrHeader &header);
    bool buildSection(const nlohmann::json &segment, binary::irProto::TimingSection &section);
    bool buildGroup(const QVariantList &elements, const CachedProtocol &protocol,
        std::vector<std::pair<uint8_t, std::vector<uint8_t>>> &out);
    bool buildSymbols(const KeyCodeNumber &number, const binary::irProto::TimingSection &section,
        std::vector<uint8_t> &symbols);

    QString path;
    QString errStr = "OK";

    nlohmann::json j;

    //Id-, index in "Protocols"
    std::unordered_map<uint32_t, size_t> idToIndex;
    //normalized name, Id-
    QHash<QString, uint32_t> nameToId;
    //Id-, protocol. node based, references stay valid when entries are added.
    std::unordered_map<uint32_t, CachedProtocol> irProtoCache;
};

}
}
