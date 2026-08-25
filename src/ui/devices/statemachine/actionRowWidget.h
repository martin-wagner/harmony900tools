// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QWidget>

#include "document/data/items/action.h"

class QComboBox;
class QStackedWidget;
class QToolButton;
class QLineEdit;
class QLabel;

namespace editors
{

/** editor for a single Action (one step inside a DeviceAction sequence).
 *
 * shows a Target selector (hidden by default -- Activity targets are not
 * exposed in this UI yet, but the control exists and is kept in sync so it
 * can be revealed later without redesigning the row) and an Operation
 * selector that drives a QStackedWidget of parameter forms: only the
 * fields relevant to the chosen Operation are shown.
 */
class ActionRowWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit ActionRowWidget(QWidget *parent = nullptr);

    /** load this row's controls from an existing Action */
    void setAction(const document::data::item::DeviceAction &action);

    /** read the row's controls back into an Action */
    document::data::item::DeviceAction getAction() const;

    /** show/hide the drag handle (list end rows may not need one) */
    void setDragHandleVisible(bool visible);

  signals:
    void removeRequested();
    void changed();

  private slots:
    void onOperationChanged(int index);

  private:
    void buildUi();
    void buildParameterPages();
    QWidget* buildSendCommandPage();
    QWidget* buildSendDelayPage();
    QWidget* buildSendFlushPage();
    QWidget* buildSendNumberPage();
    QWidget* buildSetValuePage();
    QWidget* buildForceValuePage();

    QLabel *dragHandle = nullptr;
    QComboBox *targetCombo = nullptr; //hidden by default, see buildUi()
    QComboBox *operationCombo = nullptr;
    QStackedWidget *parameterStack = nullptr;
    QToolButton *removeButton = nullptr;

    //SendCommand page controls
    QComboBox *commandCombo = nullptr;
    QComboBox *modifierCombo = nullptr;

    //SendDelay page controls
    QLineEdit *delayEdit = nullptr;

    //SendNumber page controls
    QLineEdit *numberEdit = nullptr;

    //SetValue page controls
    QLineEdit *stateNameEdit = nullptr;
    QLineEdit *valueEdit = nullptr;

    //ForceValue page controls (kept separate from SetValue's, even though
    //the parameter shape is identical, so the two pages can diverge later)
    QLineEdit *forceStateNameEdit = nullptr;
    QLineEdit *forceValueEdit = nullptr;
};

}
