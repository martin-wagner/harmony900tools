// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QWidget>

#include "context.h"
#include "document/data/items/state.h"

class QTableWidget;
class QPushButton;

namespace editors
{

/** editor for a DiscreteActions state machine -- one row per state, each
 * with the action fired on entering that state (DiscreteActions::states[i]
 * <-> DiscreteActions::enterStateAction[i]). Clicking a row's action button
 * expands a DeviceActionEditor for that row inline.
 */
class DiscreteStateEditor : public QWidget
{
    Q_OBJECT

  public:
    explicit DiscreteStateEditor(Context &ctx, QWidget *parent = nullptr);

    void setDiscreteActions(uint32_t devicePos, uint32_t smPos);

    void updateData();

  private slots:
    void onAddStateClicked();
    void onRemoveStateClicked();
    void onActionButtonClicked();
    void onStateNameChanged(int row, const QString &text);

  private:
    Context &ctx;
    document::Config &config;
    uint32_t devicePos = 0xffffffff;
    uint32_t smPos = 0xffffffff;

    const document::data::item::DiscreteActions &getActions() const;

    void createView(Context &ctx);
    void createConnections();
    void addStateRow(const QString &stateName, const document::data::item::DeviceAction &enterAction);
    void updateActionButtonLabel(QPushButton *button, const document::data::item::DeviceAction &deviceAction);

    QString makeStateNameUnique(const QString &name);

    QTableWidget *table = nullptr;
    QPushButton *addStateButton = nullptr;
};

}
