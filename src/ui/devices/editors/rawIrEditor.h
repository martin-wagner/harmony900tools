// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QDialog>
#include <QLabel>
#include <QTableWidgetItem>

#include "bin/ssIr/file.h"

class QTableWidget;
class QDoubleSpinBox;

namespace editors
{

/**
 * @brief Modal dialog to view/edit the raw mark/pause pulses of a
 * binary::ssIr::SerialStreamIr as a table.
 */
class RawIrEditor: public QDialog
{
  Q_OBJECT

  public:
    explicit RawIrEditor(const binary::ssIr::SerialStreamIr &stream, QWidget *parent = nullptr);
    ~RawIrEditor() override;

    /** build a SerialStreamIr from the current table content + clock spinbox */
    binary::ssIr::SerialStreamIr getStream() const;

  protected:
    /** insert a row at the given position, clamping mark_us/pause_us to [0, MAX_VALUE] */
    void addRow(int row, uint16_t mark_us, uint16_t pause_us);

  private:
    static constexpr uint16_t MAX_VALUE = 32767;

    QTableWidget *table = nullptr;
    QDoubleSpinBox *clockBox = nullptr;
    QLabel *durationLabel = nullptr;

    void addRowClicked();
    void removeRowClicked();
    void itemChanged(QTableWidgetItem *item);
    void updateDuration();
};

}
