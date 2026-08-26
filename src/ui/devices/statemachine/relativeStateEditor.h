// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QWidget>

#include "context.h"
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
    explicit RelativeStateEditor(Context &ctx, QWidget *parent = nullptr);

    void setRelativeActions(uint32_t devicePos, uint32_t smPos);

    void updateData();

  private slots:
    void onAddStateClicked();
    void onRemoveStateClicked();
    void onEditNextActionClicked();
    void onEditPrevActionClicked();
    void onEditResetActionClicked();
    void onClearPrevActionClicked();
    void onClearResetActionClicked();
    void onStateNameChanged(int row, const QString &text);
    void onStateNameMoved(int start, int destinationRow);

  private:
    document::Config &config;
    uint32_t devicePos = 0xffffffff;
    uint32_t smPos = 0xffffffff;

    const document::data::item::RelativeActions &getActions() const;
    void createView(Context &ctx);
    void createConnections();

    void updatePrevSlotState();
    void updateResetSlotState();
    void updateActionButtonLabel(QPushButton *button, const document::data::item::DeviceAction &deviceAction);

    QString makeStateNameUnique(const QString &name);

    QListWidget *statesList = nullptr;
    QPushButton *addStateButton = nullptr;
    QPushButton *removeStateButton = nullptr;

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
