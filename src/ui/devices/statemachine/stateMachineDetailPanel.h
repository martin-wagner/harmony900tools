// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QWidget>

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
    explicit StateMachineDetailPanel(QWidget *parent = nullptr);

    void setStateMachine(const document::data::item::StateMachine &stateMachine);
    document::data::item::StateMachine getStateMachine() const;

    /** show an empty-state placeholder instead of an editor, e.g. when
     * nothing is selected in the state machine list */
    void showEmptyState();

  signals:
    void changed();
    void deleteRequested();
    void rerunWizardRequested();

  private:
    void buildUi();
    int stackIndexForType(document::data::item::StateMachineType type) const;

    QLabel *smTypeLabel = nullptr;
    QSpinBox *delaySpin = nullptr;
    QToolButton *deleteButton = nullptr;
    QToolButton *rerunWizardButton = nullptr;

    QStackedWidget *typeStack = nullptr;
    QWidget *emptyStatePage = nullptr;
    DiscreteStateEditor *discreteEditor = nullptr;
    RelativeStateEditor *relativeEditor = nullptr;

    document::data::item::StateMachineType currentType = document::data::item::StateMachineType::Unknown;
};

}
