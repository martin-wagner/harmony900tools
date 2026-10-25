// SPDX-License-Identifier: LGPL-2.1-or-later

/*
 * Tests for ProtocolDatabase (jsonProtocols.cpp)
 *
 * AllProtocols.json in this folder only has invented protocols and is marked with
 * "TestFixture": true. This file also contains some intentional errors for checking
 * error handling.
 */

#include <algorithm>
#include <array>
#include <cstddef>
#include <fstream>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include <QByteArray>
#include <QFile>
#include <QString>
#include <QTemporaryDir>
#include <QVariant>

#include <nlohmann/json.hpp>

#include "jsonProtocols.h"
#include "keyCode.h"

using namespace document;
using namespace document::files;

using binary::irProto::Code;
using binary::irProto::Group;
using binary::irProto::IrProto;
using binary::irProto::Item;
using binary::irProto::TimingSection;

/** let gtest print QString readable */
void PrintTo(const QString &text, std::ostream *os)
{
  *os << text.toStdString();
}

namespace
{

using Symbols = std::vector<uint8_t>;
using Records = std::vector<std::pair<int, Symbols>>;
using Items = std::vector<Item>;

QString okText()
{
  return QStringLiteral("OK");
}

QString allProtocolsPath()
{
  const QString path = qEnvironmentVariable("IR_TEST_ALL_PROTOCOLS");
  if (!path.isEmpty()) {
    return path;
  }
  return QString::fromUtf8(ALL_PROTOCOLS_TEST_PATH);
}

QString keyCodesPath()
{
  const QString path = qEnvironmentVariable("IR_TEST_KEY_CODES");
  if (!path.isEmpty()) {
    return path;
  }
  return QString::fromUtf8(KEY_CODE_TEST_PATH);
}

DatabasePaths makePaths(const QString &path)
{
  DatabasePaths paths;
  paths.protocols = path;
  return paths;
}

QString normalized(const QString &name)
{
  return name.trimmed().toLower();
}

/** value as bit list, msb first */
Symbols bitsOf(uint64_t value, int width)
{
  Symbols bits;
  for (int i = width - 1; i >= 0; i--) {
    bits.push_back(static_cast<uint8_t>((value >> i) & 1));
  }
  return bits;
}

Records recordsOf(const std::vector<Group> &groups)
{
  Records records;
  for (const auto &group : groups) {
    records.emplace_back(group.getSection(), group.getData());
  }
  return records;
}

/** item of a key code as list of elements. no element, one element or array. */
QVariantList elementsOfItem(const QVariant &item)
{
  QVariantList elements;
  const KeyCode::ItemType type = KeyCode::getItemType(item);
  if (type == KeyCode::ItemType::Array) {
    elements = item.toList();
  }
  if ((type == KeyCode::ItemType::Number)
      || (type == KeyCode::ItemType::Text)) {
    elements.append(item);
  }
  return elements;
}

//---------------------------------------------------------------------------
// minimal protocol, for the tests which need their own database file
//---------------------------------------------------------------------------

const std::string MINIMAL_PROTOCOL =
    R"({
  "Id-": 7, "Name": "Minimal", "CarrierFrequency": 38000,
  "KeyCode": { "Start": null, "Repeat": [ { "SegmentName": "Minimal", "SegmentType": 1 } ], "Finish": null },
  "IRSegments": [ {
    "Name": "Minimal", "Header": [], "Trailer": [], "TotalLength": 0,
    "Payload": { "NumberOfBits": 2, "ToggleBit": null, "Encodings": [
      { "BitType": 0, "Atoms": [ { "Type": 1, "Value": 500 }, { "Type": 0, "Value": 500 } ] },
      { "BitType": 1, "Atoms": [ { "Type": 1, "Value": 500 }, { "Type": 0, "Value": 1500 } ] } ] } } ],
  "CodeSegments": [] })";

nlohmann::json minimalProtocol(uint32_t id, const std::string &name)
{
  nlohmann::json protocol = nlohmann::json::parse(MINIMAL_PROTOCOL);
  protocol["Id-"] = id;
  protocol["Name"] = name;
  protocol["IRSegments"][0]["Name"] = name;
  protocol["KeyCode"]["Repeat"][0]["SegmentName"] = name;
  return protocol;
}

std::string databaseOf(const std::vector<nlohmann::json> &protocols)
{
  nlohmann::json content;
  content["Protocols"] = protocols;
  return content.dump();
}

//---------------------------------------------------------------------------
// database files written by the tests
//---------------------------------------------------------------------------

class ProtocolDatabaseLoadTest: public ::testing::Test
{
  protected:
    void SetUp() override
    {
      ASSERT_TRUE(dir.isValid());
    }

    QString writeFile(const std::string &content)
    {
      const QString path = dir.filePath(QStringLiteral("database.json"));
      QFile file(path);
      EXPECT_TRUE(file.open(QIODevice::WriteOnly));
      file.write(content.data(), static_cast<qint64>(content.size()));
      file.close();
      return path;
    }

    bool load(const std::string &content)
    {
      return db.setDatabase(makePaths(writeFile(content)));
    }

    QTemporaryDir dir;
    ProtocolDatabase db;
};

//---------------------------------------------------------------------------
// fixture: use prepared AllProtocols.json
//---------------------------------------------------------------------------

class ProtocolDatabaseTest: public ::testing::Test
{
  public:
    const uint16_t irProtoIndex = 42;

  protected:
    void SetUp() override
    {
      ASSERT_TRUE(db.setDatabase(makePaths(allProtocolsPath())))<<db.getErrStr().toStdString();
    }

    Code makeCode(const std::string &line)
    {
      auto code = db.getCode(QString::fromUtf8(line.c_str()));
      code.changeIndex(irProtoIndex); //needs to be set after adding irProto to file!!! not available in testing.
      return code;
    }

    /** repeat group of a line which has to be valid */
    Records repeatRecordsOf(const std::string &line)
    {
      SCOPED_TRACE(line);
      const Code code = makeCode(line);
      EXPECT_TRUE(code.isValid()) << db.getErrStr().toStdString();
      return recordsOf(code.accessRepeatGroups());
    }

    void expectInvalid(const std::string &line)
    {
      SCOPED_TRACE(line);
      const Code code = makeCode(line);
      EXPECT_FALSE(code.isValid());
      EXPECT_NE(db.getErrStr(), okText());
    }

    void expectNotLoadable(uint32_t id)
    {
      SCOPED_TRACE(id);
      EXPECT_TRUE(db.getProtocol(id).isEmpty());
      EXPECT_NE(db.getErrStr(), okText());
    }

    ProtocolDatabase db;
  };

}

//===========================================================================
// loading the database file
//===========================================================================

TEST_F(ProtocolDatabaseLoadTest, validDatabaseLoads)
{
  ASSERT_TRUE(load(databaseOf( { minimalProtocol(7, "Minimal") })));
  EXPECT_EQ(db.getErrStr(), okText());
  EXPECT_FALSE(db.getProtocol(7).isEmpty());
}

TEST_F(ProtocolDatabaseLoadTest, missingFileFails)
{
  EXPECT_FALSE(
      db.setDatabase(makePaths(dir.filePath(QStringLiteral("doesNotExist.json")))));
  EXPECT_NE(db.getErrStr(), okText());
}

TEST_F(ProtocolDatabaseLoadTest, emptyFileFails)
{
  EXPECT_FALSE(load(""));
  EXPECT_NE(db.getErrStr(), okText());
}

TEST_F(ProtocolDatabaseLoadTest, invalidJsonFails)
{
  EXPECT_FALSE(load("{ this is not json"));
  EXPECT_NE(db.getErrStr(), okText());
}

TEST_F(ProtocolDatabaseLoadTest, missingProtocolsFails)
{
  EXPECT_FALSE(load("{}"));
  EXPECT_NE(db.getErrStr(), okText());
}

TEST_F(ProtocolDatabaseLoadTest, protocolsNotAnArrayFails)
{
  EXPECT_FALSE(load("{ \"Protocols\": 5 }"));
  EXPECT_NE(db.getErrStr(), okText());
}

TEST_F(ProtocolDatabaseLoadTest, emptyProtocolsFails)
{
  EXPECT_FALSE(load("{ \"Protocols\": [] }"));
  EXPECT_NE(db.getErrStr(), okText());
}

TEST_F(ProtocolDatabaseLoadTest, protocolWithoutNameFails)
{
  nlohmann::json protocol = minimalProtocol(7, "Minimal");
  protocol.erase("Name");

  auto ret = load(databaseOf( { protocol }));
  EXPECT_FALSE(ret);
  EXPECT_NE(db.getErrStr(), okText());
}

TEST_F(ProtocolDatabaseLoadTest, duplicateIdFails)
{
  EXPECT_FALSE(
      load(
          databaseOf(
              { minimalProtocol(7, "First"), minimalProtocol(7, "Second") })));
  EXPECT_NE(db.getErrStr(), okText());
}

TEST_F(ProtocolDatabaseLoadTest, failedLoadLeavesNoData)
{
  ASSERT_TRUE(load(databaseOf( { minimalProtocol(7, "Minimal") })));
  ASSERT_FALSE(load("{ this is not json"));

  EXPECT_TRUE(db.getProtocol(7).isEmpty());
  EXPECT_NE(db.getErrStr(), okText());
}

TEST_F(ProtocolDatabaseLoadTest, canLoadAfterFailedLoad)
{
  ASSERT_FALSE(load("{ this is not json"));

  EXPECT_TRUE(load(databaseOf( { minimalProtocol(7, "Minimal") })));
  EXPECT_EQ(db.getErrStr(), okText());
  EXPECT_FALSE(db.getProtocol(7).isEmpty());
}

TEST_F(ProtocolDatabaseLoadTest, newDatabaseReplacesOldOne)
{
  ASSERT_TRUE(load(databaseOf( { minimalProtocol(7, "First") })));
  ASSERT_TRUE(load(databaseOf( { minimalProtocol(8, "Second") })));

  EXPECT_TRUE(db.getProtocol(7).isEmpty());
  EXPECT_FALSE(db.getProtocol(8).isEmpty());
}

TEST(ProtocolDatabaseNoDatabaseTest, lookupsFailWithoutDatabase)
{
  ProtocolDatabase db;

  EXPECT_TRUE(db.getProtocol(1).isEmpty());
  EXPECT_NE(db.getErrStr(), okText());

  EXPECT_FALSE(db.getCode(QStringLiteral("G:Minimal:()(0x1)():3")).isValid());
  EXPECT_NE(db.getErrStr(), okText());
}

//===========================================================================
// getProtocol
//===========================================================================

TEST_F(ProtocolDatabaseTest, protocolIsFoundById)
{
  std::string name;
  const IrProto proto = db.getProtocol(101, &name);

  EXPECT_FALSE(proto.isEmpty());
  EXPECT_EQ(name, "Test Simple 8 Bit");
  EXPECT_EQ(db.getErrStr(), okText());
}

TEST_F(ProtocolDatabaseTest, nameIsReturnedAsStored)
{
  std::string name;
  EXPECT_FALSE(db.getProtocol(106, &name).isEmpty());
  EXPECT_EQ(name, "Test Trailing Space 8 Bit ");
}

TEST_F(ProtocolDatabaseTest, idIsNotThePositionInTheFile)
{
  //id 5 is the last entry of the file
  std::string name;
  EXPECT_FALSE(db.getProtocol(5, &name).isEmpty());
  EXPECT_EQ(name, "Test Low Id 8 Bit");

  EXPECT_TRUE(db.getProtocol(1).isEmpty());
  EXPECT_TRUE(db.getProtocol(2).isEmpty());
}

TEST_F(ProtocolDatabaseTest, unknownIdFails)
{
  expectNotLoadable(9999);
}

TEST_F(ProtocolDatabaseTest, idZeroFails)
{
  expectNotLoadable(0);
}

TEST_F(ProtocolDatabaseTest, errorDoesNotBlockLaterLookups)
{
  expectNotLoadable(9999);

  EXPECT_FALSE(db.getProtocol(101).isEmpty());
  EXPECT_EQ(db.getErrStr(), okText());
}

TEST_F(ProtocolDatabaseTest, secondLookupGivesTheSameProtocol)
{
  const IrProto first = db.getProtocol(102);
  const IrProto second = db.getProtocol(102);

  ASSERT_FALSE(first.isEmpty());
  EXPECT_EQ(first.getSectionCount(), second.getSectionCount());
  EXPECT_EQ(first.getClockPeriod(), second.getClockPeriod());
  EXPECT_EQ(first.accessSection(0).serialise(0),
      second.accessSection(0).serialise(0));
}

TEST_F(ProtocolDatabaseTest, carrierIsConvertedToClockPeriod)
{
  EXPECT_EQ(db.getProtocol(101).getClockPeriod(), 26316);
  EXPECT_EQ(db.getProtocol(108).getClockPeriod(), 27778);
}

TEST_F(ProtocolDatabaseTest, missingCarrierUsesDefault)
{
  EXPECT_EQ(db.getProtocol(109).getClockPeriod(), 26316);
  EXPECT_EQ(db.getProtocol(121).getClockPeriod(), 26316);
}

TEST_F(ProtocolDatabaseTest, carrierWithoutValidClockPeriodFails)
{
  expectNotLoadable(110);
}

TEST_F(ProtocolDatabaseTest, protocolWithoutKeyCodesFails)
{
  //ip only
  expectNotLoadable(111);
  //no key code entry
  expectNotLoadable(120);
}

TEST_F(ProtocolDatabaseTest, protocolWithoutSectionsFails)
{
  expectNotLoadable(117);
}

TEST_F(ProtocolDatabaseTest, sectionsAreIrSegmentsFollowedByCodeSegments)
{
  const IrProto proto = db.getProtocol(105);

  ASSERT_EQ(proto.getSectionCount(), 6);
  const std::vector<int> bitCounts = { 8, 12, 4, 0, 0, 0 };
  const std::vector<int> quantisations = { 2, 2, 4, 0, 0, 0 };
  for (int i = 0; i < 6; i++) {
    SCOPED_TRACE(i);
    EXPECT_EQ(proto.accessSection(i).getBitCount(), bitCounts[i]);
    EXPECT_EQ(proto.accessSection(i).getData().getQuantisation(),
        quantisations[i]);
  }
}

TEST_F(ProtocolDatabaseTest, symbolTypeFollowsEncodingCount)
{
  EXPECT_TRUE(db.getProtocol(101).accessSection(0).getData().isBoolean());
  EXPECT_TRUE(db.getProtocol(103).accessSection(0).getData().isQuad());
  EXPECT_TRUE(db.getProtocol(104).accessSection(0).getData().isHex());
}

TEST_F(ProtocolDatabaseTest, sectionTimingsAreRead)
{
  const IrProto proto = db.getProtocol(101);
  const TimingSection &section = proto.accessSection(0);

  EXPECT_EQ(section.getBitCount(), 8);
  EXPECT_EQ(section.getTiming(), 60000u);
  EXPECT_FALSE(section.hasToggle());
  //type 1 = mark
  EXPECT_EQ(section.getSoF().accessStream(), (Items { { true, 7000 }, {
    false,
    3500 } }));
  EXPECT_EQ(section.getEoF().accessStream(), (Items { { true, 450 }, {
    false,
    12000 } }));
  EXPECT_EQ(section.getData().getTperQantum(), 2);
  EXPECT_EQ(section.getData().accessQuantum(0), (Items { { true, 450 }, {
    false,
    450 } }));
  EXPECT_EQ(section.getData().accessQuantum(1), (Items { { true, 450 }, {
    false,
    1350 } }));
}

TEST_F(ProtocolDatabaseTest, quadAndHexSymbolsHaveTheirOwnTimings)
{
  const IrProto quad = db.getProtocol(103);
  ASSERT_EQ(quad.accessSection(0).getData().getQuantisation(), 4);
  EXPECT_EQ(quad.accessSection(0).getData().accessQuantum(2), (Items { {
    true,
    400 }, { false, 1200 } }));

  const IrProto hex = db.getProtocol(104);
  ASSERT_EQ(hex.accessSection(0).getData().getQuantisation(), 16);
  EXPECT_EQ(hex.accessSection(0).getData().accessQuantum(15), (Items { {
    true,
    300 }, { false, 4800 } }));
}

TEST_F(ProtocolDatabaseTest, toggleBitIsRead)
{
  const IrProto proto = db.getProtocol(107);

  EXPECT_TRUE(proto.accessSection(0).hasToggle());
  EXPECT_EQ(proto.accessSection(0).getToggle(), 2);
  EXPECT_EQ(proto.accessSection(0).getBitCount(), 13);
}

TEST_F(ProtocolDatabaseTest, missingTotalLengthKeepsPauseInEof)
{
  const IrProto proto = db.getProtocol(107);

  EXPECT_EQ(proto.accessSection(0).getTiming(), TimingSection::PAUSE_IN_EOF);
}

TEST_F(ProtocolDatabaseTest, zeroTotalLengthKeepsPauseInEof)
{
  //section 1 is a code segment with total length 0
  const IrProto proto = db.getProtocol(102);

  EXPECT_EQ(proto.accessSection(0).getTiming(), 90000u);
  EXPECT_EQ(proto.accessSection(1).getTiming(), TimingSection::PAUSE_IN_EOF);
}

TEST_F(ProtocolDatabaseTest, codeSegmentHasNoPayload)
{
  const IrProto proto = db.getProtocol(102);
  ASSERT_EQ(proto.getSectionCount(), 2);
  const TimingSection &section = proto.accessSection(1);

  EXPECT_EQ(section.getBitCount(), 0);
  EXPECT_TRUE(section.getData().isEmpty());
  EXPECT_EQ(section.getSoF().accessStream(), (Items { { true, 8000 }, {
    false,
    2000 } }));
  EXPECT_EQ(section.getEoF().accessStream(), (Items { { true, 450 }, {
    false,
    30000 } }));
}

TEST_F(ProtocolDatabaseTest, longTimingsAreSplitIntoMultipleItems)
{
  const IrProto proto = db.getProtocol(112);
  ASSERT_FALSE(proto.isEmpty());

  //70000 us mark: max length mark, 0 us space, max length mark, 0 us space, rest
  const Items expectedSof = { { true, 32767 }, { false, 0 }, { true, 32767 }, {
    false,
    0 }, { true, 4466 }, { false, 1000 } };
  EXPECT_EQ(proto.accessSection(0).getSoF().accessStream(), expectedSof);

  //40000 us space
  const Items expectedEof = { { false, 32767 }, { true, 0 }, { false, 7233 } };
  EXPECT_EQ(proto.accessSection(0).getEoF().accessStream(), expectedEof);
}

TEST_F(ProtocolDatabaseTest, quantaAreFilledUpToTheSameItemCount)
{
  const IrProto proto = db.getProtocol(113);
  ASSERT_FALSE(proto.isEmpty());
  const auto &data = proto.accessSection(0).getData();

  //timings stay the same, the 0 us items alternate the state
  EXPECT_EQ(data.getTperQantum(), 3);
  EXPECT_EQ(data.accessQuantum(0), (Items { { false, 100 }, { true, 0 }, {
    false,
    0 } }));
  EXPECT_EQ(data.accessQuantum(1), (Items { { true, 10 }, { false, 20 }, {
    true,
    30 } }));
}

TEST_F(ProtocolDatabaseTest, symbolIsTheBitTypeNotThePosition)
{
  //the file lists BitType 1 first
  const IrProto proto = db.getProtocol(114);
  ASSERT_FALSE(proto.isEmpty());
  const auto &data = proto.accessSection(0).getData();

  EXPECT_EQ(data.accessQuantum(0), (Items { { true, 300 }, { false, 900 } }));
  EXPECT_EQ(data.accessQuantum(1), (Items { { true, 900 }, { false, 300 } }));
}

TEST_F(ProtocolDatabaseTest, invalidBitTypeFails)
{
  //BitType 0 used twice
  expectNotLoadable(115);
}

TEST_F(ProtocolDatabaseTest, timingsWhichDoNotFitIntoTheFormatFail)
{
  //header: 6 s pause needs more than 255 items
  expectNotLoadable(116);
  //payload: same
  expectNotLoadable(118);
}

//===========================================================================
// getCode
//===========================================================================

TEST_F(ProtocolDatabaseTest, simpleCodeIsCreated)
{
  const Code code = makeCode("G:Test Simple 8 Bit:()(0xA5)():3");

  ASSERT_TRUE(code.isValid())<< db.getErrStr().toStdString();
  EXPECT_EQ(db.getErrStr(), okText());
  EXPECT_EQ(code.getIndex(), irProtoIndex);
  EXPECT_TRUE(code.accessStartGroups().empty());
  EXPECT_EQ(recordsOf(code.accessRepeatGroups()),
      (Records { { 0, bitsOf(0xA5, 8) } }));
  EXPECT_TRUE(code.accessFinishGroups().empty());
}

TEST_F(ProtocolDatabaseTest, wordSelectsSectionWithoutData)
{
  const Code code = makeCode("G:Test NEC Like 32 Bit:(0x12345678)(Repeat)():3");

  ASSERT_TRUE(code.isValid())<< db.getErrStr().toStdString();
  EXPECT_EQ(code.getIndex(), irProtoIndex);
  EXPECT_EQ(recordsOf(code.accessStartGroups()),
      (Records { { 0, bitsOf(0x12345678, 32) } }));
  //the code segment is the section after the ir segment
  EXPECT_EQ(recordsOf(code.accessRepeatGroups()),
      (Records { { 1, Symbols() } }));
  EXPECT_TRUE(code.accessFinishGroups().empty());
}

TEST_F(ProtocolDatabaseTest, codeCreatesTheTransmitData)
{
  const Code code = makeCode("G:Test NEC Like 32 Bit:(0x12345678)(Repeat)():3");
  ASSERT_TRUE(code.isValid())<< db.getErrStr().toStdString();

  //start frame, then the repeat frame once per repeat
  const IrProto::Data data = code.getData();
  ASSERT_EQ(data.size(), 4u);
  EXPECT_EQ(data[0].first, 0);
  EXPECT_EQ(data[0].second, bitsOf(0x12345678, 32));
  EXPECT_EQ(data[1].first, 1);
  EXPECT_EQ(data[2].first, 1);
  EXPECT_EQ(data[3].first, 1);
}

TEST_F(ProtocolDatabaseTest, boolValueIsMsbFirstAndLeftPadded)
{
  EXPECT_EQ(repeatRecordsOf("G:Test Simple 8 Bit:()(0xA5)():3"), (Records { {
    0,
    bitsOf(0xA5, 8) } }));
  EXPECT_EQ(repeatRecordsOf("G:Test Simple 8 Bit:()(0x1)():3"), (Records { {
    0,
    bitsOf(0x01, 8) } }));
  EXPECT_EQ(repeatRecordsOf("G:Test Simple 8 Bit:()(0x0)():3"), (Records { {
    0,
    bitsOf(0x00, 8) } }));
  EXPECT_EQ(repeatRecordsOf("G:Test Simple 8 Bit:()(0xFF)():3"), (Records { {
    0,
    bitsOf(0xFF, 8) } }));
}

TEST_F(ProtocolDatabaseTest, leadingZerosCanBeOmittedOrAdded)
{
  EXPECT_EQ(repeatRecordsOf("G:Test Simple 8 Bit:()(0x00A5)():3"), (Records { {
    0,
    bitsOf(0xA5, 8) } }));
  EXPECT_EQ(repeatRecordsOf("G:Test Quad 6 Bit:()(0x0123)():3"), (Records { {
    0,
    { 0, 0, 0, 1, 2, 3 } } }));
  EXPECT_EQ(repeatRecordsOf("G:Test Hex 4 Bit:()(0x0A5F)():3"), (Records { {
    0,
    { 0, 10, 5, 15 } } }));
}

TEST_F(ProtocolDatabaseTest, quadValueIsOneSymbolPerDigit)
{
  EXPECT_EQ(repeatRecordsOf("G:Test Quad 6 Bit:()(0x321032)():3"), (Records { {
    0,
    { 3, 2, 1, 0, 3, 2 } } }));
  EXPECT_EQ(repeatRecordsOf("G:Test Quad 6 Bit:()(0x0)():3"), (Records { { 0, {
    0,
    0,
    0,
    0,
    0,
    0 } } }));
}

TEST_F(ProtocolDatabaseTest, hexValueIsOneSymbolPerDigit)
{
  EXPECT_EQ(repeatRecordsOf("G:Test Hex 4 Bit:()(0xA5F)():3"), (Records { { 0, {
    0,
    10,
    5,
    15 } } }));
  EXPECT_EQ(repeatRecordsOf("G:Test Hex 4 Bit:()(0xFEDC)():3"), (Records { {
    0,
    { 15, 14, 13, 12 } } }));
  EXPECT_EQ(repeatRecordsOf("G:Test Hex 4 Bit:()(0x0)():3"), (Records { { 0, {
    0,
    0,
    0,
    0 } } }));
}

TEST_F(ProtocolDatabaseTest, valuesWhichDoNotFitAreRejected)
{
  //9 bit for 8 bit section
  expectInvalid("G:Test Simple 8 Bit:()(0x1A5)():3");
  //7 symbols for 6 symbol section
  expectInvalid("G:Test Quad 6 Bit:()(0x1230123)():3");
  //5 symbols for 4 symbol section
  expectInvalid("G:Test Hex 4 Bit:()(0x10000)():3");
}

TEST_F(ProtocolDatabaseTest, symbolsWhichTheSectionDoesNotHaveAreRejected)
{
  //quad only has 0...3
  expectInvalid("G:Test Quad 6 Bit:()(0x0004)():3");
}

TEST_F(ProtocolDatabaseTest, numberInFrontOfTheXIsTheSection)
{
  const Code code = makeCode(
      "G:Test Multi Section:(Start)(0x12_1x345)(Trailer):3");

  ASSERT_TRUE(code.isValid())<< db.getErrStr().toStdString();
  EXPECT_EQ(recordsOf(code.accessStartGroups()),
      (Records { { 3, Symbols() } }));
  EXPECT_EQ(recordsOf(code.accessRepeatGroups()),
      (Records { { 0, bitsOf(0x12, 8) }, { 1, bitsOf(0x345, 12) } }));
  EXPECT_EQ(recordsOf(code.accessFinishGroups()),
      (Records { { 5, Symbols() } }));
}

TEST_F(ProtocolDatabaseTest, framesKeepTheirOrder)
{
  EXPECT_EQ(
      repeatRecordsOf(
          "G:Test Multi Section:()(Start_0x61_Space_1xA_Trailer)():3"),
      (Records { { 3, Symbols() }, { 0, bitsOf(0x61, 8) }, { 4, Symbols() }, {
        1,
        bitsOf(0xA, 12) }, { 5, Symbols() } }));
}

TEST_F(ProtocolDatabaseTest, sectionCanBeUsedMultipleTimes)
{
  EXPECT_EQ(repeatRecordsOf("G:Test Multi Section:()(0x01_0x02)():3"),
      (Records { { 0, bitsOf(0x01, 8) }, { 0, bitsOf(0x02, 8) } }));
}

TEST_F(ProtocolDatabaseTest, sectionCanBeLeftOut)
{
  EXPECT_EQ(repeatRecordsOf("G:Test Multi Section:()(2x0123)():3"), (Records { {
    2,
    { 0, 1, 2, 3 } } }));
}

TEST_F(ProtocolDatabaseTest, wordsCanBeUsedInEveryGroup)
{
  const Code code = makeCode("G:Test Multi Section:(Trailer)(0x01)(Start):3");

  ASSERT_TRUE(code.isValid())<< db.getErrStr().toStdString();
  EXPECT_EQ(recordsOf(code.accessStartGroups()),
      (Records { { 5, Symbols() } }));
  EXPECT_EQ(recordsOf(code.accessRepeatGroups()),
      (Records { { 0, bitsOf(0x01, 8) } }));
  EXPECT_EQ(recordsOf(code.accessFinishGroups()),
      (Records { { 3, Symbols() } }));
}

TEST_F(ProtocolDatabaseTest, wordsIgnoreCase)
{
  const Code code = makeCode("G:Test Multi Section:(START)()(trailer):3");

  ASSERT_TRUE(code.isValid())<< db.getErrStr().toStdString();
  EXPECT_EQ(recordsOf(code.accessStartGroups()),
      (Records { { 3, Symbols() } }));
  EXPECT_EQ(recordsOf(code.accessFinishGroups()),
      (Records { { 5, Symbols() } }));
}

TEST_F(ProtocolDatabaseTest, protocolNameIgnoresCaseAndSurroundingSpaces)
{
  EXPECT_TRUE(makeCode("G:test simple 8 bit:()(0xA5)():3").isValid());
  EXPECT_TRUE(makeCode("G:  Test Simple 8 Bit :()(0xA5)():3").isValid());
  //the file has a space at the end of the name
  EXPECT_TRUE(makeCode("G:Test Trailing Space 8 Bit:()(0x01)():3").isValid());
}

TEST_F(ProtocolDatabaseTest, tailAfterTheGroupsIsIgnored)
{
  const std::vector<std::string> tails = {
    ":3",
    ":0",
    ":1",
    ":3\t",
    ":3.",
    ":312",
    "" };

  for (const std::string &tail : tails) {
    const std::string line = "G:Test Simple 8 Bit:()(0xA5)()" + tail;
    SCOPED_TRACE(line);

    const Code code = makeCode(line);
    ASSERT_TRUE(code.isValid())<< db.getErrStr().toStdString();
    EXPECT_EQ(recordsOf(code.accessRepeatGroups()),
        (Records { { 0, bitsOf(0xA5, 8) } }));
    EXPECT_EQ(code.getRepeats(), Code::DEFAULT_REPEATS);
  }
}

TEST_F(ProtocolDatabaseTest, rawCommandsAreRejected)
{
  expectInvalid("R:Test Simple 8 Bit:()(0xA5)():3");
}

TEST_F(ProtocolDatabaseTest, unknownProtocolIsRejected)
{
  expectInvalid("G:Does Not Exist:()(0x01)():3");
}

TEST_F(ProtocolDatabaseTest, malformedLinesAreRejected)
{
  expectInvalid("");
  expectInvalid("garbage");
  expectInvalid("G:");
  expectInvalid("G:Test Simple 8 Bit");
}

TEST_F(ProtocolDatabaseTest, ipKeyCodesAreRejected)
{
  //protocol has no key codes at all
  expectInvalid("G:Test IP Only:(Key)(Back)():3");
  //key word in a normal protocol
  expectInvalid("G:Test Simple 8 Bit:()(Key)():3");
}

TEST_F(ProtocolDatabaseTest, unknownWordsAreRejected)
{
  expectInvalid("G:Test Multi Section:()(Bogus)():3");
  //protocol has no code segment with this name
  expectInvalid("G:Test Simple 8 Bit:()(Repeat)():3");
  //broken numbers are words
  expectInvalid("G:Test Simple 8 Bit:()(0x0x1)():3");
  expectInvalid("G:Test Simple 8 Bit:()(0xZZ)():3");
  expectInvalid("G:Test Simple 8 Bit:()(0x00FF48B7v)():3");
}

TEST_F(ProtocolDatabaseTest, unknownSectionIsRejected)
{
  //protocol has the sections 0...5
  expectInvalid("G:Test Multi Section:()(6x1)():3");
}

TEST_F(ProtocolDatabaseTest, dataForSectionWithoutPayloadIsRejected)
{
  //section 3 is a code segment
  expectInvalid("G:Test Multi Section:()(3x1)():3");
}

TEST_F(ProtocolDatabaseTest, lineWithoutDataIsRejected)
{
  expectInvalid("G:Test Simple 8 Bit:()()():3");
}

TEST_F(ProtocolDatabaseTest, protocolWhichCanNotBeLoadedIsRejected)
{
  expectInvalid("G:Test Low Carrier 8 Bit:()(0x01)():3");
}

TEST_F(ProtocolDatabaseTest, errorDoesNotBlockLaterCodes)
{
  expectInvalid("G:Test Simple 8 Bit:()(0x1A5)():3");

  const Code code = makeCode("G:Test Simple 8 Bit:()(0xA5)():3");
  EXPECT_TRUE(code.isValid());
  EXPECT_EQ(db.getErrStr(), okText());
}

//===========================================================================
// key code file: database is created from the lines, so every line can be checked
//===========================================================================

namespace
{

/** words which can be used in the key code file. everything else is not a section. */
const std::set<std::string>& knownWords()
{
  static const std::set<std::string> words = {
    "start",
    "repeat",
    "finish",
    "trailer",
    "space",
    "spacer",
    "spacer2",
    "ss",
    "ls",
    "lrs",
    "divider",
    "repeatstart",
    "stopspacer",
    "longspace" };
  return words;
}

struct SampleSection
{
    int maxSymbol = 0;
    std::vector<QString> values;
};

struct SampleProtocol
{
    QString name;
    std::map<int, SampleSection> sections;
    std::set<std::string> words;
    int irCount = 1;
    std::vector<int> quantisation;
    std::vector<int> bits;
};

struct SampleLine
{
    QString text;
    QString protocolKey;
    QString displayName;
    bool expectedValid = false;
    std::array<QVariantList, 3> groups;
};

int hexValue(const QChar &digit)
{
  return QString(digit).toInt(nullptr, 16);
}

/** symbols of a number, msb first. bool: bits of all hex digits. quad / hex: one symbol per digit. */
std::vector<int> symbolsOf(int quantisation, const QString &digits)
{
  std::vector<int> symbols;

  for (const QChar &digit : digits) {
    const int value = hexValue(digit);
    if (quantisation == 2) {
      for (int bit = 3; bit >= 0; bit--) {
        symbols.push_back((value >> bit) & 1);
      }
    } else {
      symbols.push_back(value);
    }
  }
  return symbols;
}

std::vector<int> significantSymbols(int quantisation, const QString &digits)
{
  const std::vector<int> symbols = symbolsOf(quantisation, digits);

  size_t first = 0;
  while ((first < symbols.size()) && (symbols[first] == 0)) {
    first++;
  }
  return std::vector<int>(symbols.begin() + static_cast<std::ptrdiff_t>(first),
      symbols.end());
}

SampleLine analyseLine(const QString &text)
{
  SampleLine line;
  line.text = text;

  KeyCode key;
  if (!KeyCode::parse(text, key) || (key.getGroup() != QStringLiteral("G"))) {
    return line;
  }
  line.displayName = key.getName().trimmed();
  line.protocolKey = normalized(key.getName());

  int count = 0;
  bool known = true;
  for (int g = 0; g < 3; g++) {
    line.groups[g] = elementsOfItem(key.getItem(g));
    for (const QVariant &element : line.groups[g]) {
      count++;
      if (KeyCode::getItemType(element) == KeyCode::ItemType::Text) {
        const std::string word = element.toString().toLower().toStdString();
        known = known && (knownWords().count(word) > 0);
      }
    }
  }
  line.expectedValid = known && (count > 0);
  return line;
}

void addToProtocol(const SampleLine &line, SampleProtocol &protocol)
{
  if (protocol.name.isEmpty()) {
    protocol.name = line.displayName;
  }

  for (const QVariantList &group : line.groups) {
    for (const QVariant &element : group) {
      if (KeyCode::getItemType(element) == KeyCode::ItemType::Text) {
        protocol.words.insert(element.toString().toLower().toStdString());
        continue;
      }

      const KeyCodeNumber number = element.value<KeyCodeNumber>();
      SampleSection &section = protocol.sections[number.prefix];
      section.values.push_back(number.digits);
      for (const QChar &digit : number.digits) {
        section.maxSymbol = std::max(section.maxSymbol, hexValue(digit));
      }
    }
  }
}

int quantisationOf(const QString &protocolKey, const SampleSection *section)
{
  if (protocolKey.contains(QStringLiteral("hex"))) {
    return 16;
  }
  if (protocolKey.contains(QStringLiteral("quad"))
      && ((section == nullptr) || (section->maxSymbol <= 3))) {
    return 4;
  }
  return 2;
}

/** size every section so that all values of the lines fit */
void finishProtocol(const QString &protocolKey, SampleProtocol &protocol)
{
  int count = 1;
  if (!protocol.sections.empty()) {
    count = protocol.sections.rbegin()->first + 1;
  }
  protocol.irCount = count;

  for (int index = 0; index < count; index++) {
    const SampleSection *section = nullptr;
    const auto found = protocol.sections.find(index);
    if (found != protocol.sections.end()) {
      section = &found->second;
    }

    const int quantisation = quantisationOf(protocolKey, section);
    int bits = 1;
    if (section != nullptr) {
      for (const QString &value : section->values) {
        bits = std::max(bits,
            static_cast<int>(significantSymbols(quantisation, value).size()));
      }
    }
    protocol.quantisation.push_back(quantisation);
    protocol.bits.push_back(bits);
  }
}

nlohmann::json atomJson(int type, int value)
{
  nlohmann::json atom;
  atom["Type"] = type;
  atom["Value"] = value;
  return atom;
}

nlohmann::json encodingsJson(int quantisation)
{
  nlohmann::json encodings = nlohmann::json::array();

  for (int symbol = 0; symbol < quantisation; symbol++) {
    nlohmann::json encoding;
    encoding["BitType"] = symbol;
    encoding["Atoms"] = nlohmann::json::array();
    encoding["Atoms"].push_back(atomJson(1, 400));
    encoding["Atoms"].push_back(atomJson(0, 400 * (symbol + 1)));
    encodings.push_back(encoding);
  }
  return encodings;
}

nlohmann::json segmentJson(const std::string &name, bool hasPayload,
    int quantisation, int bits)
{
  nlohmann::json segment;
  segment["Name"] = name;
  segment["Header"] = nlohmann::json::array();
  segment["Header"].push_back(atomJson(1, 7000));
  segment["Header"].push_back(atomJson(0, 3500));
  segment["Trailer"] = nlohmann::json::array();
  segment["Trailer"].push_back(atomJson(1, 450));
  segment["TotalLength"] = 0;

  if (hasPayload) {
    nlohmann::json payload;
    payload["NumberOfBits"] = bits;
    payload["ToggleBit"] = nullptr;
    payload["Encodings"] = encodingsJson(quantisation);
    segment["Payload"] = payload;
  } else {
    segment["Payload"] = nullptr;
  }
  return segment;
}

nlohmann::json sampleProtocolJson(uint32_t id, const SampleProtocol &protocol)
{
  const std::string name = protocol.name.toStdString();

  nlohmann::json result;
  result["Id-"] = id;
  result["Name"] = name;
  result["CarrierFrequency"] = 38000;
  result["KeyCode"] = nlohmann::json::object();
  result["IRSegments"] = nlohmann::json::array();
  result["CodeSegments"] = nlohmann::json::array();

  for (int index = 0; index < protocol.irCount; index++) {
    std::string segmentName = name;
    if (index > 0) {
      segmentName += " " + std::to_string(index);
    }
    result["IRSegments"].push_back(
        segmentJson(segmentName, true, protocol.quantisation[index],
            protocol.bits[index]));
  }
  for (const std::string &word : protocol.words) {
    result["CodeSegments"].push_back(
        segmentJson(name + " KeyCode" + word, false, 0, 0));
  }
  return result;
}

Records expectedRecords(const SampleProtocol &protocol,
    const QVariantList &elements)
{
  Records records;

  for (const QVariant &element : elements) {
    if (KeyCode::getItemType(element) == KeyCode::ItemType::Number) {
      const KeyCodeNumber number = element.value<KeyCodeNumber>();
      const int quantisation = protocol.quantisation[number.prefix];
      const int bits = protocol.bits[number.prefix];

      const std::vector<int> significant = significantSymbols(quantisation,
          number.digits);
      Symbols symbols(static_cast<size_t>(bits) - significant.size(), 0);
      for (int value : significant) {
        symbols.push_back(static_cast<uint8_t>(value));
      }
      records.emplace_back(number.prefix, symbols);
    } else {
      //code segments are in the order of the (sorted) words, after the ir segments
      const std::string word = element.toString().toLower().toStdString();
      const auto found = protocol.words.find(word);
      const int wordIndex = static_cast<int>(std::distance(
          protocol.words.begin(), found));
      records.emplace_back(protocol.irCount + wordIndex, Symbols());
    }
  }
  return records;
}

}

TEST(keyCodes, everyLineCreatesTheExpectedCode)
{
  QFile file(keyCodesPath());
  ASSERT_TRUE(file.open(QIODevice::ReadOnly));

  //protocols are sized to fit the lines, only lines with known words can be coded
  std::vector<SampleLine> lines;
  std::map<QString, SampleProtocol> protocols;
  while (!file.atEnd()) {
    const QString text = QString::fromUtf8(file.readLine());
    if (text.trimmed().isEmpty()) {
      continue;
    }

    lines.push_back(analyseLine(text));
    if (lines.back().expectedValid) {
      addToProtocol(lines.back(), protocols[lines.back().protocolKey]);
    }
  }
  ASSERT_FALSE(lines.empty());

  nlohmann::json content;
  content["Protocols"] = nlohmann::json::array();
  uint32_t id = 1;
  for (auto &entry : protocols) {
    finishProtocol(entry.first, entry.second);
    content["Protocols"].push_back(sampleProtocolJson(id, entry.second));
    id++;
  }

  QTemporaryDir dir;
  ASSERT_TRUE(dir.isValid());
  const QString path = dir.filePath(QStringLiteral("sample.json"));
  {
    QFile out(path);
    ASSERT_TRUE(out.open(QIODevice::WriteOnly));
    const std::string dump = content.dump();
    out.write(dump.data(), static_cast<qint64>(dump.size()));
  }

  ProtocolDatabase db;
  ASSERT_TRUE(db.setDatabase(makePaths(path)))<< db.getErrStr().toStdString();

  size_t validCount = 0;
  for (const SampleLine &line : lines) {
    SCOPED_TRACE(line.text.toStdString());
    Code code = db.getCode(line.text);
    EXPECT_EQ(code.getIndex(), 0xffff); //not available at parsing time
    code.changeIndex(42); //set valid index for serialiser.

    if (!line.expectedValid) {
      EXPECT_FALSE(code.isValid());
      EXPECT_NE(db.getErrStr(), okText());
      continue;
    }

    ASSERT_TRUE(code.isValid())<< db.getErrStr().toStdString();
    EXPECT_EQ(db.getErrStr(), okText());
    validCount++;

    const SampleProtocol &protocol = protocols.at(line.protocolKey);
    EXPECT_EQ(recordsOf(code.accessStartGroups()),
        expectedRecords(protocol, line.groups[0]));
    EXPECT_EQ(recordsOf(code.accessRepeatGroups()),
        expectedRecords(protocol, line.groups[1]));
    EXPECT_EQ(recordsOf(code.accessFinishGroups()),
        expectedRecords(protocol, line.groups[2]));
  }
  EXPECT_GT(validCount, 0u);
}

