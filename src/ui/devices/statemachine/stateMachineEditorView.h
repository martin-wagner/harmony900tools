// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QWidget>
#include <QVector>
#include <QHBoxLayout>

#include "document/config.h"
#include "context.h"
#include "ui/logViewer.h"
#include "ui/baseTreeView.h"

namespace editors
{

class StateMachineDetailPanel;

/**
 * @brief list tree view for state machines
 *
 */
class StateMachineTreeView: public BaseTreeView
{
  Q_OBJECT

  public:
    explicit StateMachineTreeView(Context &ctx, QWidget *parent = nullptr);
    ~StateMachineTreeView() override;

    void setModel(QAbstractItemModel *model) override;

  private:
    void setupDelegates();
};


/**
 * @brief editor view for state machines
 *
 * top-level widget: tree of a device's state machines on the left, detail
 * editor on the right.
 */
class StateMachineEditorView: public QWidget
{
  Q_OBJECT

  public:
    explicit StateMachineEditorView(Context &ctx, StateMachineTreeView *stateTree, QWidget *parent = nullptr);
    ~StateMachineEditorView();

    /** load data, update model
     *
     * non-table values are directly pulled / set from the config data set, not via model */
    void setData(uint32_t deviceId, QAbstractItemModel *stateModel);

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

  protected slots:
//    void onEditingPressPreSilenceFinished();
//    void onEditingHoldPreSilenceFinished();
//    void onEditingInterKeyFinished();
    void onStateMachineSelectionChanged(int row);
    void onStateMachineDataChanged(document::data::Item item, uint32_t pos);
    void onWizardRequested();

  protected:
    void createView();
    void setupTreeView();
    void createConnections();

  protected:
    Context &ctx;
    uint32_t deviceId = 0;

  private:
    QHBoxLayout *layout = nullptr;
    QLabel *header = nullptr;
    StateMachineTreeView *tree;

    StateMachineDetailPanel *detailPanel = nullptr;


//    QSpinBox *pressPreSilenceSpinBox;
//    QSpinBox *holdPreSilencSpinBox;
//    QSpinBox *interKeySpinBox; //combines both inter-key from the data set
};






}
