// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QWidget>

#include "context.h"
#include "document/data/items/state.h"

class QComboBox;
class QSpinBox;
class QStackedWidget;
class QLabel;
class QToolButton;

namespace editors
{

class DiscreteStateEditor;
class RelativeStateEditor;

/** header (state machine type, delay) plus the type-specific editor for a
 * single StateMachine. The type-specific editor is chosen via a
 * QStackedWidget indexed by StateMachineType.
 */
class StateMachineDetailPanel : public QWidget
{
    Q_OBJECT

  public:
    explicit StateMachineDetailPanel(Context &ctx, QWidget *parent = nullptr);

    void setStateMachine(uint32_t devicePos, uint32_t smPos);

    void updateData();

    /** show an empty-state placeholder instead of an editor, e.g. when
     * nothing is selected in the state machine list */
    void showEmptyState();

  signals:
    void rerunWizardRequested();

  protected slots:
    void onEditingDelayFinished();

  private:
    document::Config &config;
    uint32_t devicePos = -1;
    uint32_t smPos = -1;

    const document::data::item::StateMachine &getMachine() const;

    void createView(Context &ctx);
    void createConnections();
    int stackIndexForType(document::data::item::StateMachineType type) const;

    QLabel *smTypeLabel = nullptr;
    QSpinBox *delaySpinBox = nullptr;
    QToolButton *rerunWizardButton = nullptr;

    QStackedWidget *typeStack = nullptr;
    QWidget *emptyStatePage = nullptr;
    DiscreteStateEditor *discreteEditor = nullptr;
    RelativeStateEditor *relativeEditor = nullptr;

    document::data::item::StateMachineType currentType = document::data::item::StateMachineType::Unknown;
};

}
