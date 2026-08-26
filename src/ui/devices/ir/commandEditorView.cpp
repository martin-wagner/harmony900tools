// SPDX-License-Identifier: LGPL-2.1-or-later

#include "models/rawIrListModel.h"
#include "models/protocolIrListModel.h"
#include "lib/qtHelpers.h"
#include "ui/delegates/protocolIr.h"
#include "ui/delegates/rawIr.h"
#include "ui/delegates/combobox.h"
#include "bin/codec/decode.h"
#include "bin/codec/encode.h"
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
  QMessageBox msgBox(this);
  msgBox.setIcon(QMessageBox::Warning);

  if (model == nullptr) {
    return;
  }

  //step 1: decode the received data, check if valid

  auto decoder = binary::codec::Decode(t, carrier);
  auto &data = decoder.getData();
  auto codeStr = qstr(data.codeString);
  switch (data.decoded) {
    case binary::codec::Status::OK:
      break;
    case binary::codec::Status::ERROR_UNSUPPORTED:
      msgBox.setText(tr("IR protocol %1 not supported in typed IR mode. "
          "Try again or use Raw IR mode.").arg(codeStr));
      msgBox.exec();
      return;
    case binary::codec::Status::ERROR_SIZE:
      msgBox.setText(
          tr("Learning failed (data to short). Try again.").arg(
              qstr(data.codeString)));
      msgBox.exec();
      return;
    default:
      msgBox.setText(
          tr("Learning failed or IR protocol not supported in typed IR mode. "
              "Try again or use Raw IR mode.").arg(qstr(data.codeString)));
      msgBox.exec();
      return;
  }
  switch (data.codeType) {
    case document::data::CodeType::Unknown:
    case document::data::CodeType::Proprietary:
    case document::data::CodeType::None:
      msgBox.setText(tr("Learning selected invalid protocol. "
          "Try again or use Raw IR mode."));
      msgBox.exec();
      return;
    default:
      break;
  }

  auto *protocolModel = static_cast<models::ProtocolIrModel*>(model);
  auto index = protocolModel->index(getCurrentRow(), 0);

  ctx.undoStack().beginMacro(tr("learned command: %1").arg(codeStr));

  //step 2: set protocol type in model. this will add the protocol to the
  //        irProt list if necessary.
  auto res = protocolModel->setData(
      index.siblingAtColumn(models::ProtocolIrModel::Column::TYPE),
      document::data::Enum<document::data::CodeType>::toQString(data.codeType),
      Qt::EditRole);
  if (res != true) {
    msgBox.setText(tr("Adding learned command failed (setting "
        "protocol type)"));
    msgBox.exec();
    ctx.undoStack().endMacro();
    return;
  }
  //step 3: get index of added protocol in irProt
  auto protocolIndex = protocolModel->data(
      index.siblingAtColumn(models::ProtocolIrModel::Column::PROTO),
      Qt::EditRole).toInt();
  if (protocolIndex < 0) {
    msgBox.setText(tr("Adding learned command failed (adding "
        "protocol type)"));
    msgBox.exec();
    ctx.undoStack().endMacro();
    return;
  }
  //step 4: create the command code from received data
  auto code = binary::codec::encode(protocolIndex, data.codeType,
      data.codeString, data.address, data.command, data.data);
  if (code.getDataSectionCount() == 0) {
    msgBox.setText(tr("Creating data stream from learned command failed. "
        "Try again or use Raw IR mode."));
    msgBox.exec();
    ctx.undoStack().endMacro();
    return;
  }
  //step 5: we have created a valid command. set all the data to the model
  protocolModel->setData(
      index.siblingAtColumn(models::ProtocolIrModel::Column::VERBOSE),
      qstr(data.codeString), Qt::EditRole);
  protocolModel->setData(
      index.siblingAtColumn(models::ProtocolIrModel::Column::IRADDRESS),
      data.address, Qt::EditRole);
  protocolModel->setData(
      index.siblingAtColumn(models::ProtocolIrModel::Column::IRCOMMAND),
      data.command, Qt::EditRole);
  protocolModel->setData(
      index.siblingAtColumn(models::ProtocolIrModel::Column::IRBITS),
      QVariant::fromValue(data.data), Qt::EditRole);
  protocolModel->setData(
      index.siblingAtColumn(models::ProtocolIrModel::Column::DATA),
      QVariant::fromValue(code), Qt::EditRole);

  ctx.undoStack().endMacro();
}

void ProtoCommandTreeView::onUserLevelChanged(lib::UserLevel::Level l)
{
  BaseTreeView::onUserLevelChanged(l);

  if (lib::UserLevel::validate(l, lib::UserLevel::Level::Developer)) {
    treeView->showColumn(models::ProtocolIrModel::Column::IRBITS);
    treeView->showColumn(models::ProtocolIrModel::Column::PROTO);
  } else {
    treeView->hideColumn(models::ProtocolIrModel::Column::IRBITS);
    treeView->hideColumn(models::ProtocolIrModel::Column::PROTO);
  }
}

void ProtoCommandTreeView::setupDelegates()
{
  auto *comboBoxDelegate = new delegates::ComboBox(this);
  auto *editorDelegate = new delegates::ProtocolIr(ctx, this);
  treeView->setItemDelegateForColumn(
      static_cast<int>(models::ProtocolIrModel::Column::TYPE),
      comboBoxDelegate);
  treeView->setItemDelegateForColumn(
      static_cast<int>(models::ProtocolIrModel::Column::VERBOSE),
      editorDelegate);
  treeView->setItemDelegateForColumn(
      static_cast<int>(models::ProtocolIrModel::Column::IRADDRESS),
      editorDelegate);
  treeView->setItemDelegateForColumn(
      static_cast<int>(models::ProtocolIrModel::Column::IRCOMMAND),
      editorDelegate);
  treeView->setItemDelegateForColumn(
      static_cast<int>(models::ProtocolIrModel::Column::IRBITS),
      editorDelegate);
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
    return;
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
  pressPreSilenceSpinBox->setSingleStep(50);
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
  interKeySpinBox->setSingleStep(50);
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
  holdPreSilencSpinBox->setSingleStep(50);
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
