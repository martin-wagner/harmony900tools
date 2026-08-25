// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QWidget>

#include "document/data/items/state.h"

class QTableWidget;
class QPushButton;

namespace document
{
namespace data
{
namespace item
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
    explicit DiscreteStateEditor(QWidget *parent = nullptr);

    void setDiscreteActions(const DiscreteActions &discreteActions);
    DiscreteActions getDiscreteActions() const;

  signals:
    void changed();

  private slots:
    void onAddStateClicked();
    void onRemoveStateClicked();
    void onActionButtonClicked();

  private:
    void buildUi();
    void addStateRow(const QString &stateName, const DeviceAction &enterAction);
    void updateActionButtonLabel(QPushButton *button, const DeviceAction &deviceAction);

    QTableWidget *table = nullptr;
    QPushButton *addStateButton = nullptr;
};

}
}
}
