// SPDX-License-Identifier: LGPL-2.1-or-later

#include "document/config.h"
#include "protocolIrListModel.h"
#include "bin/codec/encode.h"
#include "lib/qtHelpers.h"
#include "ui/devices/editors/codeEditor.h" //default code generation, code serialiser

using namespace std;

namespace models
{

ProtocolIrModel::ProtocolIrModel(document::Config &config, uint32_t deviceId,
    QObject *parent) :
    BaseModel(document::data::Item::DEVICE_IR_DATA, parent), config(config), id(
        deviceId)
{
  createActions(&config);
}

ProtocolIrModel::~ProtocolIrModel() = default;

QVariant ProtocolIrModel::headerData(int section, Qt::Orientation orientation,
    int role) const
{
  if ((orientation != Qt::Horizontal) || (role != Qt::DisplayRole)) {
    return {};
  }
  try {
    return columnSetup.at(static_cast<Column>(section)).name;
  } catch (...) {
  }
  return {};
}

QModelIndex ProtocolIrModel::index(int row, int column,
    const QModelIndex &parent) const
{
  if (parent.isValid()) {
    //not a hierarchical model! parent = header, data = first.
    return {};
  }
  if ((row >= getCmds().size()) || (column >= Column::COUNT)) {
    return {};
  }
  return createIndex(row, column, &getCmds()[row]);
}

int ProtocolIrModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid()) {
    return 0;
  }
  return getCmds().size();
}

int ProtocolIrModel::columnCount(const QModelIndex &parent) const
{
  return columnSetup.size();
}

Qt::ItemFlags ProtocolIrModel::flags(const QModelIndex &index) const
{
  if (!index.isValid()) {
    return Qt::NoItemFlags;
  }

  auto flags = QAbstractItemModel::flags(index);
  try {
    auto isConst = columnSetup.at(static_cast<Column>(index.column())).isConst;
    if (!isConst) {
      flags = flags | Qt::ItemIsEditable;
    }
    auto dataType = columnSetup.at(static_cast<Column>(index.column())).dataType;
    if (dataType == "bool") {
      flags = flags | Qt::ItemIsUserCheckable;
    }
  } catch (...) {
  }
  return flags;
}

bool ProtocolIrModel::setData(const QModelIndex &index, const QVariant &value,
    int role)
{
  if (index.parent().isValid() || (role != Qt::EditRole)) {
    return false;
  }
  auto row = index.row();
  auto column = static_cast<Column>(index.column());

  //checks
  if (!columnSetup.contains(column)) {
    return false;
  }
  if (columnSetup.at(column).isConst) { //caller should have used flags() method!
    return false;
  }
  if (row >= rowCount()) {
    return false;
  }
  auto currentValue = data(index, role);
  if (currentValue.isValid() && (currentValue == value)) {
    return true;
  }

  auto &worker = config.modify();
  try {
    switch (index.column()) {
      case Column::NAME:
        return setCommandName(worker, row, value);
      case Column::TYPE:
        return setCommandType(worker, row, value);
      case Column::VERBOSE:
        return setCommandVerbose(worker, row, value);
      case Column::IRADDRESS:
        return setCommandIrAddress(worker, row, value);
      case Column::IRCOMMAND:
        return setCommandIrCommand(worker, row, value);
      case Column::IRBITS:
        return setCommandIrBits(worker, row, value);
      case Column::DATA:
        return setCommandData(worker, row, value);
      default:
        return false;
    }
  } catch (const out_of_range &ex) {
    return false;
  }

  //don't emit dataChanged event, is done inside observers anyway
  return true;
}

bool ProtocolIrModel::insertRows(int position, int rows,
    const QModelIndex &parent)
{
  bool success = true;

  if (parent.isValid()) {
    return false;
  }

  //no begin/end insert rows, is done inside observers

  config.beginMacro(QObject::tr("Add %1 protocol ir cmd(s)").arg(rows));

  for (int i = position; i < position + rows; i++) {
    auto ret = addItem(i);
    if (!ret) {
      success = false;
      continue;
    }
  }

  config.endMacro();

  return success;
}

bool ProtocolIrModel::removeRows(int position, int rows,
    const QModelIndex &parent)
{
  bool success = true;

  if (parent.isValid()) {
    return false;
  }

  //no begin/end remove rows, is done inside observers

  config.beginMacro(QObject::tr("Remove %1 protocol ir cmd(s)").arg(rows));

  for (int i = 0; i < rows; i++) {
    //remove beginning at last item
    auto currRow = position + rows - 1 - i;
    auto ret = removeItem(currRow);
    if (!ret) {
      success = false;
      continue;
    }
  }

  config.endMacro();

  return success;
}

bool models::ProtocolIrModel::addItem(int row)
{
  uint32_t devicePos;
  QString commandToUse;
  document::data::item::ProtoCommand cmd;

  QString suggestName = tr("New Command");

  auto *device = config.data().getDevice(id, &devicePos);

  //auto assign names. try all hard buttons, then default to "new command"
  auto names =
      document::data::Enum<document::data::HardButtons>::toQStringList(); //this is logically sorted
  auto usedNames = toQStringList(
      device->getIrCommands().getAvailableCommands());
  for (const auto &name : names) {
    if (!usedNames.contains(name)) {
      suggestName = name;
    }
  }
  suggestName = makeStringUnique(usedNames, suggestName);
  cmd.name = suggestName.toStdString();
  cmd.codeType.set(document::data::CodeType::None);
  cmd.canDecode.set(true);
  return config.modify().setIrCommand(devicePos, cmd, row, false);
}

bool models::ProtocolIrModel::removeItem(int row)
{
  uint32_t devicePos;

  config.data().getDevice(id, &devicePos);

  return config.modify().removeIrProtoCommand(devicePos, row);
}

const vector<document::data::item::ProtoCommand>& ProtocolIrModel::getCmds(
    uint32_t *devicePos) const
{
  return config.data().getDevice(id, devicePos)->getIrCommands().getProtoCommands();
}

QVariant ProtocolIrModel::getDisplayData(const QModelIndex &index) const
{
  uint32_t devicePos;

  try {
    auto &cmd = getCmds(&devicePos).at(index.row());
    switch (index.column()) {
      case Column::NAME:
        return QString::fromStdString(cmd.name.get());
      case Column::TYPE:
        return cmd.codeType.get().getQString();
      case Column::PROTO:
        return cmd.protocolIndex.get();
      case Column::VERBOSE:
        if (printData(cmd)) {
          return QString::fromStdString(cmd.libName.get());
        }
        return tr("Not available");
      case Column::IRADDRESS:
        if (printData(cmd)) {
          return cmd.irAddress.get();
        }
        break;
      case Column::IRCOMMAND:
        if (printData(cmd)) {
          return cmd.irCommand.get();
        }
        break;
      case Column::IRBITS:
        if (printData(cmd)) {
          return tr("0x%1, %2 bits").arg(
              QString::number(cmd.irBitData.get(), 16)).arg(
              cmd.irBitCount.get());
        }
        break;
      case Column::DATA:
        return visualiseData(cmd);
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QVariant ProtocolIrModel::getEditData(const QModelIndex &index) const
{
  uint32_t devicePos;

  try {
    auto &cmd = getCmds(&devicePos).at(index.row());
    switch (index.column()) {
      case Column::NAME:
        return QString::fromStdString(cmd.name.get());
      case Column::TYPE:
        return cmd.codeType.get().getQString();
      case Column::PROTO:
        return cmd.protocolIndex.get();
      case Column::VERBOSE:
        return QString::fromStdString(cmd.libName.get());
      case Column::IRADDRESS:
        return cmd.irAddress.get();
      case Column::IRCOMMAND:
        return cmd.irCommand.get();
      case Column::IRBITS:
        return QVariant::fromValue(
            binary::irProto::Code::u64tobits(cmd.irBitCount.get(),
                cmd.irBitData.get()));
      case Column::DATA: {
        return QVariant::fromValue(cmd.command);
      }
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QVariant ProtocolIrModel::getTooltipData(const QModelIndex &index) const
{
  try {
    return columnSetup.at(static_cast<Column>(index.column())).context;
  } catch (...) {
  }
  return {};
}

QVariant ProtocolIrModel::getSelectionItemsData(const QModelIndex &index) const
{
  uint32_t devicePos;

  try {
    auto &cmd = getCmds(&devicePos).at(index.row());
    switch (index.column()) {
      case Column::TYPE: {
        const auto &current = cmd.codeType.get();
        auto list = current.getQStringList();
        list.removeAll(
            document::data::Enum(document::data::CodeType::Unknown).getQString());
        list.removeAll(
            document::data::Enum(document::data::CodeType::Proprietary).getQString());
        if (!list.contains(current.getQString())) {
          list.prepend(current.getQString());
        }
        return list;
      }
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QVariant ProtocolIrModel::getFontData(const QModelIndex &index) const
{
  switch (index.column()) {
    case Column::DATA: {
      QFont font("Monospace");
      font.setStyleHint(QFont::Monospace);
      return font;
    }
    default:
      break;
  }
  return {};
}

bool ProtocolIrModel::printData(
    const document::data::item::ProtoCommand &cmd) const
{
  if (cmd.canDecode.get() == true) {
    switch (cmd.codeType.get().getValue()) {
      case document::data::CodeType::Unknown:
      case document::data::CodeType::Proprietary:
      case document::data::CodeType::None:
        return false;
      default:
        return true;
    }
  }
  return false;
}

QVariant ProtocolIrModel::visualiseData(
    const document::data::item::ProtoCommand &cmd) const
{
  binary::TimingStream timingData;

  if (cmd.canDecode.get() != true) {
    return tr("Proprietary data, format not supported!");
  }
  if ((cmd.codeType.get().getValue() == document::data::CodeType::Unknown)
      || (cmd.codeType.get().getValue() == document::data::CodeType::None)) {
    return "";
  }

  auto binaryData = cmd.command.getData();
  config.data().getProtocolLib().serialiseIrStream(timingData,
      cmd.protocolIndex.get(), binaryData);
  auto str = timingData.convertAsciiPlot(250, true, false);
  while (!str.empty() && str.back() == '\n') {
    str.pop_back();
  }
  return QString::fromStdString(str);
}

bool ProtocolIrModel::setCommandName(document::data::CmdCatalogue &worker,
    int row, const QVariant &value)
{
  uint32_t devicePos;
  QStringList usedNames;

  auto &cmds = getCmds(&devicePos);
  for (const auto &cmd : cmds) {
    usedNames.push_back(QString::fromStdString(cmd.name.get()));
  }
  auto &rawCmds =
      config.data().getDevices()[devicePos].getIrCommands().getRawCommands();
  for (const auto &cmd : rawCmds) {
    usedNames.push_back(QString::fromStdString(cmd.name.get()));
  }

  auto name = makeStringUnique(usedNames, value.toString());
  auto cmd = cmds.at(row);
  cmd.name = name.toStdString();
  return worker.setIrCommand(devicePos, cmd, row, true);
}

bool models::ProtocolIrModel::setCommandType(
    document::data::CmdCatalogue &worker, int row, const QVariant &value)
{
  uint32_t devicePos;
  int index = -1;
  string codeString;
  uint32_t address;
  uint32_t command;
  vector<bool> rawData;

  config.beginMacro(QObject::tr("Set IR protocol"));

  auto cmd = getCmds(&devicePos).at(row);
  cmd.codeType.set(value.toString());
  if (cmd.codeType.get().getValue() != document::data::CodeType::None) {
    index = worker.appendIrProtoLibItem(cmd.codeType.get());
    if (index < 0) {
      config.endMacro();
      return false;
    }
  }
  cmd.protocolIndex.set(index);
  cmd.canDecode.set(true);
  auto code = binary::codec::encodeDefaults(cmd.protocolIndex.get(),
      cmd.codeType.get().getValue(), codeString, address, command, rawData);
  cmd.libName.set(codeString);
  cmd.irAddress.set(address);
  cmd.irCommand.set(command);
  cmd.irBitCount.set(rawData.size());
  cmd.irBitData.set(binary::irProto::Code::bitsToU64(rawData));
  cmd.command = code;
  cmd.data.set( { });
  auto ret = worker.setIrCommand(devicePos, cmd, row, true);

  config.endMacro();

  return ret;
}

bool ProtocolIrModel::setCommandVerbose(document::data::CmdCatalogue &worker,
    int row, const QVariant &value)
{
  uint32_t devicePos;

  auto cmd = getCmds(&devicePos).at(row);
  cmd.libName.set(value.toString().toStdString());
  return worker.setIrCommand(devicePos, cmd, row, true);
}

bool ProtocolIrModel::setCommandIrAddress(document::data::CmdCatalogue &worker,
    int row, const QVariant &value)
{
  uint32_t devicePos;

  auto cmd = getCmds(&devicePos).at(row);
  cmd.irAddress.set(value.toUInt());
  return worker.setIrCommand(devicePos, cmd, row, true);
}

bool ProtocolIrModel::setCommandIrCommand(document::data::CmdCatalogue &worker,
    int row, const QVariant &value)
{
  uint32_t devicePos;

  auto cmd = getCmds(&devicePos).at(row);
  cmd.irCommand.set(value.toUInt());
  return worker.setIrCommand(devicePos, cmd, row, true);
}

bool ProtocolIrModel::setCommandIrBits(document::data::CmdCatalogue &worker,
    int row, const QVariant &value)
{
  uint32_t devicePos;

  auto cmd = getCmds(&devicePos).at(row);
  auto bits = qvariant_cast<vector<bool>>(value);
  cmd.irBitCount = bits.size();
  cmd.irBitData = binary::irProto::Code::bitsToU64(bits);
  return worker.setIrCommand(devicePos, cmd, row, true);
}

bool models::ProtocolIrModel::setCommandData(
    document::data::CmdCatalogue &worker, int row, const QVariant &value)
{
  uint32_t devicePos;

  auto cmd = getCmds(&devicePos).at(row);
  auto code = qvariant_cast<binary::irProto::Code>(value);
  cmd.canDecode.set(true);
  cmd.command = code;
  cmd.data.set( { });
  return worker.setIrCommand(devicePos, cmd, row, true);
}

}

