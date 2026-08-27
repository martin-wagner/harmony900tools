// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QWidget>
#include <QList>

#include "document/data/items/state.h"
#include "document/data/items/action.h"

class QVBoxLayout;
class QPushButton;
class QLabel;
class QCheckBox;
class QComboBox;
class Context;

namespace editors
{

class SequenceItemWidget;

/** editor for a whole DeviceAction (an ordered list of Action steps).
 *
 * this single widget backs every action slot in the state machine editor --
 * SetAction, ChangeAction, NextAction, PrevAction, ResetAction, StartAction,
 * FinishAction all reuse it, one instance per slot when opened.
 */
class DeviceActionEditor : public QWidget
{
    Q_OBJECT

  public:
    explicit DeviceActionEditor(Context &ctx, uint32_t devicePos, uint32_t smPos, QWidget *parent = nullptr);

    /** create editor, open it in box */
    static std::optional<document::data::item::DeviceAction> openEditor(Context &ctx, uint32_t devicePos, uint32_t smPos, const document::data::item::DeviceAction &deviceAction, document::data::ActionType type, const QString &title, QWidget *parent = nullptr);

    /** heading shown above the step list, e.g. "Set action - state Off" */
    void setTitle(const QString &title);

    /** load rows from an existing DeviceAction */
    void setDeviceAction(const document::data::item::DeviceAction &deviceAction, document::data::ActionType type);

    /** read the current rows back into a DeviceAction */
    document::data::item::DeviceAction getDeviceAction() const;

  signals:
    void changed();

  private slots:
    void onAddStepClicked();
    void onRowRemoveRequested();

  private:
    Context &ctx;
    uint32_t devicePos = 0xffffffff;
    uint32_t smPos = 0xffffffff;

    void createView();
    void createConnections();
    void addRow(const document::data::item::SequenceItem &sequenceItem);

    QLabel *titleLabel = nullptr;
    QComboBox *typeBox = nullptr;
    QCheckBox *repeatBox = nullptr;
    QVBoxLayout *rowsLayout = nullptr;
    QPushButton *addStepButton = nullptr;
    QList<SequenceItemWidget*> rows;
};

}
