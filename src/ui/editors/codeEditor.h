// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QDialog>

#include "bin/irProto/code.h"
#include "document/data/enum.h"

class QComboBox;
class QSpinBox;
class QStackedWidget;
class QTableWidget;
class QTableWidgetItem;

namespace editors
{

/**
 * @brief Modal dialog to view/edit a binary::irProto::Code.
 *
 * Presents a different set of fields depending on the CodeType the code
 * belongs to:
 * - Proprietary: full raw Code structure (index, clock, control type,
 *   repeat behaviour, per-section data)
 * - PhilipsRc5: simplified Address / Command fields, built internally
 *   into a single-section Code
 *
 * Other CodeType values are not yet supported by this editor.
 */
class CodeEditor: public QDialog
{
  Q_OBJECT

  public:
    CodeEditor(const binary::irProto::Code &code, document::data::CodeType codeType, QWidget *parent = nullptr);
    ~CodeEditor() override;

    /** build a Code from the current field content, matching the constructor's codeType */
    binary::irProto::Code getCode() const;

  private:
    document::data::CodeType codeType;

    QStackedWidget *stack = nullptr;

    //proprietary page
    QSpinBox *indexBox = nullptr;
    QSpinBox *clockBox = nullptr;
    QComboBox *controlTypeBox = nullptr;
    QSpinBox *sectionCountBox = nullptr; //only used for Multi Section
    QComboBox *repeatModeBox = nullptr;
    QSpinBox *repeatCountBox = nullptr;
    QTableWidget *sectionTable = nullptr;

    //rc5 page
    QSpinBox *rc5AddressBox = nullptr;
    QSpinBox *rc5CommandBox = nullptr;

    QWidget* createProprietaryPage();
    QWidget* createRc5Page();

    void loadProprietary(const binary::irProto::Code &code);
    void loadRc5(const binary::irProto::Code &code);

    binary::irProto::Code getProprietaryCode() const;
    binary::irProto::Code getRc5Code() const;

    void updateSectionTableRowCount();
    void controlTypeChanged(int index);
    void repeatModeChanged(int index);

    /** parse "1234" / "0x1234" / "0b1010" into a value, based on prefix */
    static uint64_t parsePrefixedValue(const QString &text, bool *ok);
};

//todo verschiedene protokoll varianten https://www.mikrocontroller.net/articles/IRMP#Anhang
//nec: nec Addr: 8 / ~8 + Cmd 8 / ~8 ; ext-nec addr 16 + Cmd 8 / ~8 ; onkyo addr 16 + Cmd 16, apple 16 Adress-Bits + 11100000 + 8 Kommando-Bits
//KASEIKYO / panasonic  16 Hersteller-Bits + 4 Parity-Bits + 4 Genre1-Bits + 4 Genre2-Bits + 10 Kommando-Bits + 2 ID-Bits + 8 Parity-Bits
//rc5: 14 datenbits im editor, start, toggle, end im editor belegen.
//rc6: check ob das so ok ist, 4 startbits rc6 + 16 datenbits
//sony sircs, 12 datenbits, problem: bitanzahl 12..20 in sircs... 7 Kommando-Bits + 5 Adress-Bits, mind. 3 x senden
//samsung   16 Adress-Bits + 16 Kommando-Bits


}
