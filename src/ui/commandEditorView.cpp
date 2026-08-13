// SPDX-License-Identifier: LGPL-2.1-or-later

#include "models/rawIrListModel.h"
#include "models/protocolIrListModel.h"
#include "delegates/protocolIr.h"
#include "delegates/rawIr.h"
#include "delegates/combobox.h"
#include "commandEditorView.h"

using namespace std;

namespace editors
{

RawCommandTreeView::RawCommandTreeView(Context &ctx, QWidget *parent) :
    BaseTreeView(ctx, tr("Raw IR"), false, parent)
{
  setupDelegates();
}

RawCommandTreeView::~RawCommandTreeView() = default;

void RawCommandTreeView::setModel(QAbstractItemModel *model)
{
  bindModel(model);
}

void RawCommandTreeView::setLearnedCommand(
    ConcordConnection::LearnedCommandMode m, const binary::TimingStream &t,
    uint32_t carrier)
{
  if (model == nullptr) {
    return;
  }

  auto stream = binary::ssIr::SerialStreamIr(t, carrier);
  auto value = QVariant::fromValue(stream);

  auto *rawModel = static_cast<models::RawIrModel*>(model);
  auto index = rawModel->index(getCurrentRow(),
      models::RawIrModel::Column::DATA);
  rawModel->setData(index, value, Qt::EditRole);
}

void RawCommandTreeView::setupDelegates()
{
  auto *editorDelegate = new delegates::RawIr(this);
  treeView->setItemDelegateForColumn(
      static_cast<int>(models::RawIrModel::Column::DATA), editorDelegate);
}

ProtoCommandTreeView::ProtoCommandTreeView(Context &ctx, QWidget *parent) :
    BaseTreeView(ctx, tr("Typed IR"), false, parent)
{
  setupDelegates();
}

ProtoCommandTreeView::~ProtoCommandTreeView() = default;

void ProtoCommandTreeView::setModel(QAbstractItemModel *model)
{
  bindModel(model);
}

void ProtoCommandTreeView::setLearnedCommand(
    ConcordConnection::LearnedCommandMode m, const binary::TimingStream &t,
    uint32_t carrier)
{
}

void ProtoCommandTreeView::setupDelegates()
{
  auto *comboBoxDelegate = new delegates::ComboBox(this);
  auto *editorDelegate = new delegates::ProtocolIr(ctx, this);
  treeView->setItemDelegateForColumn(
      static_cast<int>(models::ProtocolIrModel::Column::TYPE),
      comboBoxDelegate);
  treeView->setItemDelegateForColumn(
      static_cast<int>(models::ProtocolIrModel::Column::DATA), editorDelegate);
}

CommandEditorView::CommandEditorView(Context &ctx, QWidget *parent) :
    QWidget(parent), ctx(ctx)
{
  createView();
  setupTreeView();
  createConnections();
}

CommandEditorView::~CommandEditorView() = default;

void CommandEditorView::addTreeViews(ProtoCommandTreeView *protoTree,
    RawCommandTreeView *rawTree)
{
  auto *commandSplitter = new QSplitter(Qt::Horizontal, splitter);
  commandSplitter->addWidget(protoTree);
  commandSplitter->addWidget(rawTree);
  splitter->addWidget(commandSplitter);
}

void CommandEditorView::setData(uint32_t deviceId,
    QAbstractItemModel *protoModel, QAbstractItemModel *rawModel)
{
  if ((protoModel == nullptr) || (rawModel == nullptr)) {
    this->deviceId = 0;
    pressPreSilenceSpinBox->setValue(0);
    holdPreSilencSpinBox->setValue(0);
    interKeySpinBox->setValue(0);

  }
  this->deviceId = deviceId;

  //pull data from document
  onIrDeviceDataChanged(document::data::Item::DEVICE_IR, 0);
}

void CommandEditorView::onEditingPressPreSilenceFinished()
{
  uint32_t devicePos;
  ctx.config()->data().getDevice(deviceId, &devicePos);

  auto val = pressPreSilenceSpinBox->value();
  ctx.config()->modify().setIrPressPreSilenceMs(val, devicePos);
}

void CommandEditorView::onEditingHoldPreSilenceFinished()
{
  uint32_t devicePos;
  ctx.config()->data().getDevice(deviceId, &devicePos);

  auto val = holdPreSilencSpinBox->value();
  ctx.config()->modify().setIrHoldPreSilenceMs(val, devicePos);
}

void CommandEditorView::onEditingInterKeyFinished()
{
  uint32_t devicePos;
  ctx.config()->data().getDevice(deviceId, &devicePos);

  auto val = interKeySpinBox->value();
  ctx.config()->modify().setIrPressInterKeyMs(val, devicePos);
  ctx.config()->modify().setIrHoldInterKeyMs(val, devicePos);
}

void CommandEditorView::onIrDeviceDataChanged(document::data::Item item,
    uint32_t pos)
{
  if (item != document::data::Item::DEVICE_IR) {
    return;
  }

  auto *device = ctx.config()->data().getDevice(deviceId);
  if (device == nullptr) {
    return;
  }
  auto &commands = device->getIrCommands();

  pressPreSilenceSpinBox->setValue(commands.pressPreSilenceMs.get());
  holdPreSilencSpinBox->setValue(commands.holdPreSilenceMs.get());
  interKeySpinBox->setValue(
      max(commands.pressInterKeyMs.get(), commands.holdInterKeyMs.get()));
}

void CommandEditorView::createView()
{
  QLabel *label;

  //fixme use ads for this
  layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  header = new QLabel("IR commands", this);
  auto font = header->font();
  font.setBold(true);
  header->setFont(font);
  layout->addWidget(header);

  splitter = new QSplitter(Qt::Vertical, this);

  QGroupBox *delayGroupBox = new QGroupBox("Inter-command pause times", this);
  delayGroupBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  QGridLayout *delayLayout = new QGridLayout(delayGroupBox);

  pressPreSilenceSpinBox = new QSpinBox(delayGroupBox);
  pressPreSilenceSpinBox->setRange(document::data::item::Commands::SILENCE_MIN,
      document::data::item::Commands::SILENCE_MAX);
  pressPreSilenceSpinBox->setSuffix(" ms");
  pressPreSilenceSpinBox->setToolTip(tr("This pause is placed before sending "
      "a single command or a command sequence"));
  label = new QLabel(tr("Pause before send:"), this);
  label->setToolTip(pressPreSilenceSpinBox->toolTip());
  label->setBuddy(pressPreSilenceSpinBox);
  delayLayout->addWidget(label, 0, 0);
  delayLayout->addWidget(pressPreSilenceSpinBox, 0, 1);

  interKeySpinBox = new QSpinBox(delayGroupBox);
  interKeySpinBox->setRange(document::data::item::Commands::SILENCE_MIN,
      document::data::item::Commands::SILENCE_MAX);
  interKeySpinBox->setSuffix(" ms");
  interKeySpinBox->setToolTip(tr("This pause is placed between two "
      "comamnds for the same device. This overrides the previous setting"));
  label = new QLabel(tr("Pause between send:"), this);
  label->setToolTip(interKeySpinBox->toolTip());
  label->setBuddy(interKeySpinBox);
  delayLayout->addWidget(label, 1, 0);
  delayLayout->addWidget(interKeySpinBox, 1, 1);

  holdPreSilencSpinBox = new QSpinBox(delayGroupBox);
  holdPreSilencSpinBox->setRange(document::data::item::Commands::SILENCE_MIN,
      document::data::item::Commands::SILENCE_MAX);
  holdPreSilencSpinBox->setSuffix(" ms");
  holdPreSilencSpinBox->setToolTip(tr("This pause is placed between two "
      "comamnds when holding the key down. This is the minimum, the actual "
      "pause can be longer if learned command demands so."));
  label = new QLabel(tr("Pause when holding button:"), this);
  label->setToolTip(holdPreSilencSpinBox->toolTip());
  label->setBuddy(holdPreSilencSpinBox);
  delayLayout->addWidget(label, 2, 0);
  delayLayout->addWidget(holdPreSilencSpinBox, 2, 1);

  splitter->addWidget(delayGroupBox);
  layout->addWidget(splitter);
}

void CommandEditorView::setupTreeView()
{
}

void CommandEditorView::createConnections()
{
  connect(pressPreSilenceSpinBox, &QSpinBox::editingFinished, this,
      &CommandEditorView::onEditingPressPreSilenceFinished);
  connect(holdPreSilencSpinBox, &QSpinBox::editingFinished, this,
      &CommandEditorView::onEditingHoldPreSilenceFinished);
  connect(interKeySpinBox, &QSpinBox::editingFinished, this,
      &CommandEditorView::onEditingInterKeyFinished);
  connect(ctx.config(), &document::Config::itemChanged, this,
      &CommandEditorView::onIrDeviceDataChanged);
}

}
