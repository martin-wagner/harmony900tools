// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <vector>

#include <QDialog>
#include <QSpinBox>

#include "bin/irProto/code.h"
#include "document/data/enum.h"

class QComboBox;
class QStackedWidget;
class QTableWidget;
class QTableWidgetItem;

class Context;

namespace editors
{

/**
 * @brief QSpinBox that can display its value as decimal, hex (0x...), or
 * binary (0b...), while always accepting decimal, 0x, or 0b prefixed
 * text as input (e.g. when pasting).
 */
class BaseSpinBox: public QSpinBox
{
  Q_OBJECT

  public:
    enum class Base
    {
      Decimal, Hex, Binary
    };

    explicit BaseSpinBox(QWidget *parent = nullptr);

    void setDisplayBase(Base newBase);

    QValidator::State validate(QString &input, int &pos) const override;
    int valueFromText(const QString &text) const override;
    QString textFromValue(int value) const override;

  private:
    Base displayBase = Base::Decimal;
};

/**
 * @brief Modal dialog to view/edit a binary::irProto::Code.
 *
 * Presents a different set of fields depending on the CodeType the code
 * belongs to:
 * - Proprietary: full raw Code structure (index, delay, repeats and the
 *   start / repeat / finish groups with their per-section data). The group
 *   layout is dictated by the irProto protocol and can't be changed here,
 *   only the symbol values of groups with up to 64 boolean symbols.
 * - PhilipsRc5: simplified Address / Command fields, built internally
 *   into a single-section Code
 *
 * Other CodeType values are not yet supported by this editor. Add new
 * protocols by extending isSupported(), createDefault(), and adding a
 * page + load/get pair, all in this one class.
 */
class CodeEditor: public QDialog
{
  Q_OBJECT

  public:
    CodeEditor(Context &ctx, document::data::CodeType codeType,
        const QString &codeString, uint32_t address, uint32_t command,
        const std::vector<uint8_t> &rawData, const binary::irProto::Code &code,
        QWidget *parent = nullptr);
    ~CodeEditor() override;

    /** which CodeType values can this editor handle */
    static bool isSupported(document::data::CodeType type);

    /** getters after editing */
    QString getCodeString() const { return codeString; };
    uint32_t getAddress() const { return address; };
    uint32_t getCommand() const { return command; };
    std::vector<uint8_t>getRawData() const { return rawData; };
    binary::irProto::Code getCode() const { return code; };

  protected slots:
    virtual void accept() override;

  private:
    Context &ctx;
    document::data::CodeType codeType;
    QString codeString;
    uint32_t address;
    uint32_t command;
    std::vector<uint8_t> rawData;
    binary::irProto::Code code;

    bool editAll = false;
    BaseSpinBox::Base displayBase = BaseSpinBox::Base::Decimal;

    QStackedWidget *stack = nullptr;
    QComboBox *displayBaseBox = nullptr;
    std::vector<BaseSpinBox*> baseSpinBoxes;

    //proprietary page
    QSpinBox *indexBox = nullptr;
    QSpinBox *delayBox = nullptr;
    QSpinBox *repeatCountBox = nullptr;
    QTableWidget *startTable = nullptr;
    QTableWidget *repeatTable = nullptr;
    QTableWidget *finishTable = nullptr;

    //nec page
    QSpinBox *necIndexBox = nullptr;
    QSpinBox *necDelayBox = nullptr;
    QComboBox *necSubTypeBox = nullptr;
    BaseSpinBox *necAddressBox = nullptr;
    BaseSpinBox *necCommandBox = nullptr;

    //kaseikyo page
    QSpinBox *kasIndexBox = nullptr;
    QSpinBox *kasDelayBox = nullptr;
    QComboBox *kasSubTypeBox = nullptr;
    BaseSpinBox *kasMnfBox = nullptr;
    BaseSpinBox *kasDeviceBox = nullptr;
    BaseSpinBox *kasSubdeviceBox = nullptr;
    BaseSpinBox *kasCommandBox = nullptr;
    QSpinBox *kasRepeatCountBox = nullptr;

    //sirc20 page
    QSpinBox *s20IndexBox = nullptr;
    QSpinBox *s20DelayBox = nullptr;
    QComboBox *s20SubTypeBox = nullptr;
    BaseSpinBox *s20AddressBox = nullptr;
    BaseSpinBox *s20CommandBox = nullptr;
    BaseSpinBox *s20ExtraBox = nullptr;
    QSpinBox *s20RepeatCountBox = nullptr;

    //address/command page
    QSpinBox *acIndexBox = nullptr;
    QSpinBox *acDelayBox = nullptr;
    QComboBox *acSubTypeBox = nullptr;
    BaseSpinBox *acAddressBox = nullptr;
    BaseSpinBox *acCommandBox = nullptr;
    QSpinBox *acRepeatCountBox = nullptr;

    void setDisplayBase(BaseSpinBox::Base newBase);

    QWidget* createProprietaryPage();
    QWidget* createNecPage();
    QWidget* createKasPage();
    QWidget* createS20Page();
    QWidget* createACPage();
    /** table showing the records of one group (start, repeat, finish) */
    QTableWidget* createGroupTable(QWidget *parent);

    void loadProprietary();
    void loadNec();
    void loadKas();
    void loadS20();
    void loadAC();
    void loadGroupTable(QTableWidget *table,
        const std::vector<binary::irProto::Group> &groups);
    /** show the values of the table in the current display base */
    void reformatGroupTable(QTableWidget *table);

    /** @return false when a value is invalid (user was informed) */
    bool updateProprietaryData();
    void updateNecData();
    void updateKasData();
    void updateS20Data();
    void updateACData();
    /**
     * read values back from table into the groups. Section, symbol count and
     * bits per symbol are defined by the protocol and stay as they are, only
     * values of editable rows (up to 64 boolean symbols) are replaced.
     *
     * @param groups current groups, same order as the table rows
     * @param name group name, used for the error message
     * @return false when a value is invalid (user was informed)
     */
    bool readGroupTable(const QTableWidget *table,
        std::vector<binary::irProto::Group> &groups, const QString &name);

    /** parse "1234" / "0x1234" / "0b1010" into a value, based on prefix */
    static uint64_t parsePrefixedValue(const QString &text, bool *ok = nullptr);
    static inline uint32_t MASK(uint8_t bits) { return ((1u << bits) - 1); };
};

}
