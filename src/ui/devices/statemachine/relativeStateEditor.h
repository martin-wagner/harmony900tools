// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QWidget>

#include "document/data/items/state.h"

class QListWidget;
class QPushButton;

namespace editors
{

class DeviceActionEditor;

/** editor for a RelativeActions state machine -- an ordered, reorderable
 * list of state names (the cycle order), plus three action slots: Next
 * (required), Prev (optional) and Reset (optional).
 */
class RelativeStateEditor : public QWidget
{
    Q_OBJECT

  public:
    explicit RelativeStateEditor(QWidget *parent = nullptr);

    void setRelativeActions(const document::data::item::RelativeActions &relativeActions);
    document::data::item::RelativeActions getRelativeActions() const;

  signals:
    void changed();

  private slots:
    void onAddStateClicked();
    void onEditNextActionClicked();
    void onEditPrevActionClicked();
    void onEditResetActionClicked();
    void onClearPrevActionClicked();
    void onClearResetActionClicked();

  private:
    void buildUi();
    void updatePrevSlotState();
    void updateResetSlotState();
    void updateActionButtonLabel(QPushButton *button, const document::data::item::DeviceAction &deviceAction);

    QListWidget *statesList = nullptr;
    QPushButton *addStateButton = nullptr;

    QPushButton *nextActionButton = nullptr;

    QPushButton *prevActionButton = nullptr;
    QPushButton *prevActionClearButton = nullptr;

    QPushButton *resetActionButton = nullptr;
    QPushButton *resetActionClearButton = nullptr;

    document::data::item::DeviceAction nextAction;
    std::optional<document::data::item::DeviceAction> prevAction;
    std::optional<document::data::item::DeviceAction> resetAction;
};

}
