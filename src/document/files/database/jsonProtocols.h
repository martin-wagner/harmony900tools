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
    /** longest allowed pause (space) in us. */
    static constexpr uint32_t MAX_PAUSE = 1000000;
    /** some limits because further on values are stored in 1 / 2 bytes */
    static constexpr size_t MAX_ITEMS = 0xff;
    static constexpr size_t MAX_SECTIONS = 0xff;
    static constexpr size_t MAX_SYMBOLS = 0xff;
    static constexpr uint64_t MAX_BIT_COUNT = 0xffff;
    static constexpr size_t MAX_PROTOCOL_SIZE = 0xffff;

    void clear();
    bool loadFailed(const QString &message);
    static QString normalizeName(const QString &name);
    uint32_t findId(const QString &name) const;
    //protocol
    const CachedProtocol *loadProtocol(uint32_t id);
    bool buildProtocol(const nlohmann::json &jp, CachedProtocol &entry);
    static QString getKeyword(const QString &protocolName, const QString &segmentName);
    bool appendItems(const nlohmann::json &atom, std::vector<binary::irProto::Item> &out);
    bool buildHeader(const nlohmann::json &atoms, binary::irProto::TimingSectionIrHeader &header);
    bool buildPayload(const nlohmann::json &payload, binary::irProto::TimingSection &section);
    bool buildSection(const nlohmann::json &segment, binary::irProto::TimingSection &section);
    //code
    bool buildFrame(const QVariant &element, const CachedProtocol &protocol, std::vector<std::pair<uint8_t, std::vector<uint8_t>>> &out);
    bool buildGroup(const QVariant &item, const CachedProtocol &protocol, std::vector<std::pair<uint8_t, std::vector<uint8_t>>> &out);
    bool buildSymbols(const KeyCodeNumber &number, const binary::irProto::TimingSection &section, std::vector<uint8_t> &symbols);

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
