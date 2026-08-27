// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QWidget>

#include "context.h"
#include "document/data/items/action.h"
#include "document/data/items/device.h"

class QComboBox;
class QStackedWidget;
class QToolButton;
class QLineEdit;
class QLabel;
class QSpinBox;

namespace editors
{

/** editor for a single Sequence item (one step inside a DeviceAction sequence).
 */
class SequenceItemWidget : public QWidget
{
    Q_OBJECT

  public:
    enum class ParentType {
      DEVICE,
      ACTIVITY
    };

    explicit SequenceItemWidget(Context &ctx, uint32_t devicePos, uint32_t smPos, ParentType t, QWidget *parent = nullptr);

    /** load this row's controls from an existing Sequence */
    void setSequenceItem(const document::data::item::SequenceItem &sequenceItem);

    /** read the row's controls back into an Action */
    document::data::item::SequenceItem getSequenceItem() const;

  signals:
    void removeRequested();
    void changed();

  private slots:
    void onOperationChanged(const QString &text);
    void onStateNameComboChanged(const QString &text);
    void onForcedStateNameComboChanged(const QString &text);

  private:
    document::Config &config;
    uint32_t devicePos = 0xffffffff;
    uint32_t smPos = 0xffffffff;
    document::data::item::SequenceItem item;

    const document::data::item::Device &getDevice() const;
    const std::vector<document::data::item::StateMachine> &getMachines() const;
    const document::data::item::StateMachine &getMachine() const;

    void createView(ParentType t);
    void createConnections();

    void buildParameterPages();
    QWidget* buildSendCommandPage();
    QWidget* buildSendDelayPage();
    QWidget* buildSendFlushPage();
    QWidget* buildSendNumberPage();
    QWidget* buildSetValuePage();
    QWidget* buildForceValuePage();

    QComboBox *targetCombo = nullptr; //hidden by default, see buildUi()
    QComboBox *opcodeCombo = nullptr;
    QStackedWidget *parameterStack = nullptr;
    QToolButton *removeButton = nullptr;

    //SendCommand page controls
    QComboBox *cmdCombo = nullptr;
    QComboBox *modCombo = nullptr;

    //SendDelay page controls
    QSpinBox *delayBox = nullptr;

    //SendFlush page controls
    QComboBox *deviceListCombo = nullptr;

    //SendNumber page controls
    QSpinBox *valueBox = nullptr;

    //SetValue page controls
    QComboBox *stateNameCombo = nullptr;
    QComboBox *valueCombo = nullptr;

    //ForceValue page controls (no idea what force means...)
    QComboBox *forcedStateNameCombo = nullptr;
    QComboBox *forcedStateValueCombo = nullptr;

};

}
