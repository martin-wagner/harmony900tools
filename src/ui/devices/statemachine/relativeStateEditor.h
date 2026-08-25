// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QWidget>

#include "document/data/items/state.h"

class QListWidget;
class QPushButton;

namespace document
{
namespace data
{
namespace item
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

    void setRelativeActions(const RelativeActions &relativeActions);
    RelativeActions getRelativeActions() const;

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
    void updateActionButtonLabel(QPushButton *button, const DeviceAction &deviceAction);

    QListWidget *statesList = nullptr;
    QPushButton *addStateButton = nullptr;

    QPushButton *nextActionButton = nullptr;

    QPushButton *prevActionButton = nullptr;
    QPushButton *prevActionClearButton = nullptr;

    QPushButton *resetActionButton = nullptr;
    QPushButton *resetActionClearButton = nullptr;

    DeviceAction nextAction;
    std::optional<DeviceAction> prevAction;
    std::optional<DeviceAction> resetAction;
};

}
}
}
