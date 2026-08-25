// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QListWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

#include "stateMachineListWidget.h"

using namespace document::data::item;

namespace editors
{

StateMachineListWidget::StateMachineListWidget(QWidget *parent) :
    QWidget(parent)
{
  buildUi();
}

void StateMachineListWidget::setStateMachines(
    const QVector<StateMachine> &stateMachines)
{
  listWidget->clear();

  for (const StateMachine &stateMachine : stateMachines) {
    const StateMachineType type =
        stateMachine.discrete.empty() ?
            StateMachineType::Relative : StateMachineType::Discrete;

    const QString title = QString::fromStdString(
        stateMachine.smType.get().getString());
    const QString subtitle = labelForType(type);

    QListWidgetItem *item = new QListWidgetItem(
        QStringLiteral("%1\n%2").arg(title, subtitle), listWidget);
    item->setData(Qt::UserRole, static_cast<int>(type));
  }
}

int StateMachineListWidget::currentRow() const
{
  return listWidget->currentRow();
}

void StateMachineListWidget::setCurrentRow(int row)
{
  listWidget->setCurrentRow(row);
}

void StateMachineListWidget::buildUi()
{
  listWidget = new QListWidget(this);
  connect(listWidget, &QListWidget::currentRowChanged, this,
      &StateMachineListWidget::currentChanged);

  addButton = new QToolButton(this);
  addButton->setText(tr("Add state machine"));
  addButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  connect(addButton, &QToolButton::clicked, this,
      &StateMachineListWidget::addRequested);

  QVBoxLayout *rootLayout = new QVBoxLayout(this);
  rootLayout->addWidget(addButton);
  rootLayout->addWidget(listWidget, 1);
}

QString StateMachineListWidget::labelForType(StateMachineType type) const
{
  switch (type) {
    case StateMachineType::Discrete:
      return tr("Discrete");
    case StateMachineType::Relative:
      return tr("Relative");
    case StateMachineType::Unknown:
      return tr("Unknown");
  }
  return tr("Unknown");
}

}
