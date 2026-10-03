// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QString>
#include <QVariant>

namespace document
{

namespace files
{


/**
 * A single numeric element like "0x7", "1x0" or "2xB9FE".
 * The digit in front of the "x" is kept as prefix, the hex digits as string,
 * because values can be much wider than 64 bit (up to ~450 bit in the data).
 */
class KeyCodeNumber {
  public:
    int prefix = 0;
    QString digits;

    /** Converts digits to a number. Returns false if it does not fit into 64 bit. */
    bool toUInt64(uint64_t& value) const;
};

/**
 * Parsed form of a KeyCode string like
 * "G:Philips 16 Bit:()(0x1_1x2_2x3)():3"
 *
 * Each of the three items is a QVariant:
 *  - invalid          -> empty "()"
 *  - QString          -> a single word, e.g. "Repeat"
 *  - KeyCodeNumber    -> a single number
 *  - QVariantList     -> "_" separated array, each entry is a QString or KeyCodeNumber
 */
class KeyCode {
  public:
    enum class ItemType {
        Empty,
        Text,
        Number,
        Array
    };

    /** Parses one KeyCode string. Returns false if the basic structure is broken. */
    static bool parse(const QString& text, KeyCode& result);

    /** The leading letter, e.g. "G". */
    QString getGroup() const;

    /** Manufacturer and bits as one string, e.g. "Sony 15 Bit". */
    QString getName() const;

    /** Number in front of "Bit" in the name, -1 if there is none. */
    int getBitCount() const;

    /** Check if coding type is quad (four states per bit) */
    bool isQuadBitType() const;

    /** Get quad bit 0...3 */
    int getQuadSymbol(const KeyCodeNumber& number, int index) const;

    /** The number after the last ":", -1 if missing or not a number. */
    int getTrailer() const;

    /** Item 0..2. Returns an invalid QVariant for out of range. */
    QVariant getItem(int index) const;

    ItemType getItemType(int index) const;
    static ItemType getItemType(const QVariant& value);

  private:
    static int parseBitCount(const QString& name);
    static QVariant parseElement(const QString& token);
    static QVariant parseItem(const QString& content);

    QString group;
    QString name;
    int bitCount = -1;
    int trailer = -1;
    std::array<QVariant, 3> items;
};

}
}


