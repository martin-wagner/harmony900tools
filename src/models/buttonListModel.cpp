// SPDX-License-Identifier: LGPL-2.1-or-later

#include "document/config.h"
#include "buttonListModel.h"

using namespace std;

namespace models
{

/*
 *  Base Model
 */

ButtonBaseModel::ButtonBaseModel(document::Config &config,
    document::data::Item item, QObject *parent) :
    BaseModel(item, parent), config(config)
{
  createActions(&config);
}

ButtonBaseModel::~ButtonBaseModel() = default;

QVariant ButtonBaseModel::headerData(int section, Qt::Orientation orientation,
    int role) const
{
  if ((orientation != Qt::Horizontal) || (role != Qt::DisplayRole)) {
    return {};
  }
  try {
    return columnSetup.at(mapColumn(section)).name;
  } catch (...) {
  }
  return {};
}

QModelIndex ButtonBaseModel::index(int row, int column,
    const QModelIndex &parent) const
{
  if (parent.isValid()) {
    //not a hierarchical model! parent = header, data = first.
    return {};
  }
  if ((row >= getButtons().size()) || (mapColumn(column) >= Column::COUNT)) {
    return {};
  }
  return createIndex(row, column, &getButtons()[row]); //no "mapColumn()"!
}

int ButtonBaseModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid()) {
    return 0;
  }
  return getButtons().size();
}

Qt::ItemFlags ButtonBaseModel::flags(const QModelIndex &index) const
{
  if (!index.isValid()) {
    return Qt::NoItemFlags;
  }

  try {
    auto isConst = columnSetup.at(mapColumn(index.column())).isConst;
    if (!isConst) {
      return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
    }
  } catch (...) {
  }
  return QAbstractItemModel::flags(index);
}

bool ButtonBaseModel::insertRows(int position, int rows,
    const QModelIndex &parent)
{
  bool success = true;

  if (parent.isValid()) {
    return false;
  }

  //no begin/end insert rows, is done inside observers

  config.beginMacro(QObject::tr("Add %1 buttons(s)").arg(rows));

  for (int i = position; i < position + rows; i++) {
    auto ret = addButton(i);
    if (!ret) {
      success = false;
      continue;
    }
  }

  config.endMacro();

  return success;
}

bool ButtonBaseModel::removeRows(int position, int rows,
    const QModelIndex &parent)
{
  bool success = true;

  if (parent.isValid()) {
    return false;
  }
  auto &buttons = getButtons();
  if ((position + rows) > buttons.size()) {
    return false;
  }

  //no begin/end remove rows, is done inside observers

  config.beginMacro(QObject::tr("Remove %1 buttons(s)").arg(rows));

  for (int i = 0; i < rows; i++) {
    //remove beginning at last item
    auto currRow = position + rows - 1 - i;
    auto ret = removeButton(currRow);
    if (!ret) {
      success = false;
      continue;
    }
  }

  config.endMacro();

  return success;
}

QStringList ButtonBaseModel::getUnusedButtons() const
{
  auto list =
      document::data::Enum<document::data::HardButtons>::toQStringList();
  for (const auto &button : getButtons()) {
    auto name = QString::fromStdString(button.name.get());
    list.removeAll(name);
  }
  return list;
}

QStringList ButtonBaseModel::getUnusedButtons(
    const document::data::item::Button &button) const
{
  auto list = getUnusedButtons();
  auto current = QString::fromStdString(button.name.get());

  if (!list.contains(current)) {
    list.append(current);
  }

  return list;
}

QString models::ButtonBaseModel::toPositionString(int pos) const
{
  return tr("Page %1, Button %2").arg(pos / SOFTBUTTONS_PER_PAGE + 1).arg(
      pos % SOFTBUTTONS_PER_PAGE + 1);
}

QStringList ButtonBaseModel::getAvailableCommands(
    const document::data::item::Device *device) const
{
  QStringList list;

  auto commands = device->getIrCommands().getAvailableCommands();
  for (const auto &command : commands) {
    list.push_back(QString::fromStdString(command));
  }
  list.sort(Qt::CaseInsensitive);
  return list;
}

QList<QPair<uint32_t, QString>> ButtonBaseModel::getAvailableDevices() const
{
  QList<QPair<uint32_t, QString>> list;

  auto &devices = config.data().getDevices();
  for (const auto &device : devices) {
    list.push_back(
        { device.getId(), QString::fromStdString(device.label.get()) });
  }
  return list;
}

void ButtonBaseModel::itemChangedObserver(document::data::Item item, int pos)
{
  if (item == this->item) {
    //we don't know the column!
    emit dataChanged(index(pos, 0), index(pos, columnCount()), {
      Qt::DisplayRole,
      Qt::EditRole });
  } else if (item == document::data::Item::DEVICE) {
    //we don't know if/which buttons are affecated by the device data change
    emit dataChanged(index(0, 0), index(rowCount(), columnCount()), {
      Qt::DisplayRole,
      Qt::EditRole });
  }
}

void ButtonBaseModel::itemAboutToBeAddedObserver(document::data::Item item,
    int pos)
{
  if (item != this->item) {
    return;
  }
  emit beginInsertRows(QModelIndex(), pos, pos);
}

void ButtonBaseModel::itemAddedObserver(document::data::Item item, int pos)
{
  if (item != this->item) {
    return;
  }
  emit endInsertRows();
}

void ButtonBaseModel::itemAboutToBeRemovedObserver(document::data::Item item,
    int pos)
{
  if (item != this->item) {
    return;
  }
  emit beginRemoveRows(QModelIndex(), pos, pos);
}

void ButtonBaseModel::itemRemovedObserver(document::data::Item item, int pos)
{
  if (item != this->item) {
    return;
  }
  emit endRemoveRows();
}

/*
 * Device Hard Buttons
 */

DeviceHardButtonModel::DeviceHardButtonModel(document::Config &config,
    uint32_t deviceId, QObject *parent) :
    ButtonBaseModel(config, document::data::Item::DEVICE_HARD_BUTTON), id(
        deviceId)
{
}

bool DeviceHardButtonModel::setData(const QModelIndex &index,
    const QVariant &value, int role)
{
  uint32_t devicePos;

  if (index.parent().isValid() || (role != Qt::EditRole)) {
    return false;
  }
  auto row = index.row();
  auto baseColumn = mapColumn(index.column());

  //checks
  if (!columnSetup.contains(baseColumn)) {
    return false;
  }
  if (columnSetup.at(baseColumn).isConst) { //caller should have used flags() method!
    return false;
  }
  if (row >= rowCount()) {
    return false;
  }
  auto currentValue = data(index, role);
  if (currentValue.isValid() && (currentValue == value)) {
    return true;
  }

  config.data().getDevice(id, &devicePos);
  switch (static_cast<Column>(index.column())) {
    case Column::COMMAND:
      return config.modify().setDeviceButtonAction(
          value.toString().toStdString(), devicePos, type, row);
    case Column::BUTTON:
      return config.modify().setDeviceButtonName(value.toString().toStdString(),
          devicePos, type, row);
    default:
      return false;
  }

  //don't emit dataChanged event, is done inside observers anyway
  return true;
}

ButtonBaseModel::Column models::DeviceHardButtonModel::mapColumn(
    int viewColumn) const
{
  return mapColumn(static_cast<Column>(viewColumn));
}

ButtonBaseModel::Column models::DeviceHardButtonModel::mapColumn(
    Column viewColumn) const
{
  switch (viewColumn) {
    case Column::COMMAND:
      return ButtonBaseModel::Column::COMMAND;
    case Column::BUTTON:
      return ButtonBaseModel::Column::BUTTON;
    default:
      return ButtonBaseModel::Column::COUNT;
  }
}

int DeviceHardButtonModel::columnCount(const QModelIndex &parent) const
{
  return static_cast<int>(Column::COUNT);
}

const vector<document::data::item::Button>& DeviceHardButtonModel::getButtons() const
{
  return config.data().getDevice(id)->getHardButtons();
}

bool DeviceHardButtonModel::addButton(int row)
{
  uint32_t devicePos;
  QString commandToUse;

  auto *device = config.data().getDevice(id, &devicePos);

  auto commands = getAvailableCommands(device);
  auto buttons = getUnusedButtons();

  if (commands.size() == 0) {
    emit writeMsg(
        tr("No IR commands available to add. Add an IR command first"));
    return false;
  }
  if (buttons.size() == 0) {
    emit writeMsg(tr("All hardware buttons are used. Can't add more"));
    return false;
  }
  auto buttonToUse = buttons[0];

  auto ret = config.modify().addDeviceButtonCommand(devicePos, type, row);
  if (ret != true) {
    return ret;
  }
  if (commands.contains(buttonToUse)) {
    commandToUse = buttonToUse;
  } else {
    commandToUse = commands[0];
  }
  config.modify().setDeviceButtonAction(commandToUse.toStdString(), devicePos,
      type, row);
  config.modify().setDeviceButtonName(buttonToUse.toStdString(), devicePos,
      type, row);
  return true;
}

bool DeviceHardButtonModel::removeButton(int row)
{
  uint32_t devicePos;

  config.data().getDevice(id, &devicePos);

  return config.modify().removeDeviceButtonCommand(devicePos, type, row);
}

QVariant DeviceHardButtonModel::getDisplayData(const QModelIndex &index) const
{
  try {
    auto &button = getButtons().at(index.row());
    switch (static_cast<Column>(index.column())) {
      case Column::COMMAND:
        return QString::fromStdString(button.action.get());
      case Column::BUTTON:
        return QString::fromStdString(button.name.get());
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QVariant DeviceHardButtonModel::getEditData(const QModelIndex &index) const
{
  try {
    auto &button = getButtons().at(index.row());
    switch (static_cast<Column>(index.column())) {
      case Column::COMMAND:
        return QString::fromStdString(button.action.get());
      case Column::BUTTON:
        return QString::fromStdString(button.name.get());
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QVariant DeviceHardButtonModel::getTooltipData(const QModelIndex &index) const
{
  try {
    auto text = columnSetup.at(mapColumn(index.column())).context;
    switch (static_cast<Column>(index.column())) {
      case Column::BUTTON: {
        auto name = QString::fromStdString(
            getButtons().at(index.row()).name.get());
        text = tr("<b>%1</b><br>"
            "You can find it here:<br><br>"
            "<img src=':/res/buttons/%2.jpg' width='180' height='138'>").arg(
            text).arg(name);
        return text;
      }
      default:
        return text;
    }
  } catch (...) {
  }
  return {};
}

QVariant DeviceHardButtonModel::getSelectionItemsData(
    const QModelIndex &index) const
{
  auto *device = config.data().getDevice(id);
  try {
    auto &button = getButtons().at(index.row());
    switch (static_cast<Column>(index.column())) {
      case Column::COMMAND:
        return getAvailableCommands(device);
      case Column::BUTTON:
        return getUnusedButtons(button);
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

/*
 * Device Soft Buttons
 */

DeviceSoftButtonModel::DeviceSoftButtonModel(document::Config &config,
    uint32_t deviceId, QObject *parent) :
    ButtonBaseModel(config, document::data::Item::DEVICE_SOFT_BUTTON), id(
        deviceId)
{
}

bool DeviceSoftButtonModel::setData(const QModelIndex &index,
    const QVariant &value, int role)
{
  uint32_t devicePos;

  if (index.parent().isValid() || (role != Qt::EditRole)) {
    return false;
  }
  auto row = index.row();
  auto baseColumn = mapColumn(index.column());

  //checks
  if (!columnSetup.contains(baseColumn)) {
    return false;
  }
  if (columnSetup.at(baseColumn).isConst) { //caller should have used flags() method!
    return false;
  }
  if (row >= rowCount()) {
    return false;
  }
  auto currentValue = data(index, role);
  if (currentValue.isValid() && (currentValue == value)) {
    return true;
  }

  config.data().getDevice(id, &devicePos);

  switch (static_cast<Column>(index.column())) {
    case Column::COMMAND:
      return config.modify().setDeviceButtonAction(
          value.toString().toStdString(), devicePos, type, row);
    case Column::NAME:
      return config.modify().setDeviceButtonName(value.toString().toStdString(),
          devicePos, type, row);
    default:
      return false;
  }

  //don't emit dataChanged event, is done inside observers anyway
  return true;
}

ButtonBaseModel::Column models::DeviceSoftButtonModel::mapColumn(
    int viewColumn) const
{
  return mapColumn(static_cast<Column>(viewColumn));
}

ButtonBaseModel::Column models::DeviceSoftButtonModel::mapColumn(
    Column viewColumn) const
{
  switch (viewColumn) {
    case Column::POSITION:
      return ButtonBaseModel::Column::POSITION;
    case Column::COMMAND:
      return ButtonBaseModel::Column::COMMAND;
    case Column::NAME:
      return ButtonBaseModel::Column::NAME;
    default:
      return ButtonBaseModel::Column::COUNT;
  }
}

int DeviceSoftButtonModel::columnCount(const QModelIndex &parent) const
{
  return static_cast<int>(Column::COUNT);
}

const vector<document::data::item::Button>& DeviceSoftButtonModel::getButtons() const
{
  return config.data().getDevice(id)->getSoftButtons();
}

bool DeviceSoftButtonModel::addButton(int row)
{
  uint32_t devicePos;

  auto *device = config.data().getDevice(id, &devicePos);
  auto commands = getAvailableCommands(device);
  if (commands.size() == 0) {
    emit writeMsg(
        tr("No IR commands available to add. Add an IR command first"));
    return false;
  }

  //only append to the end
  row = rowCount();

  auto ret = config.modify().addDeviceButtonCommand(devicePos, type, row);
  if (ret != true) {
    return ret;
  }
  //for default -- first unused command.
  auto unusedCommands = getUnusedCommands(device);
  if (unusedCommands.size() == 0) {
    config.modify().setDeviceButtonAction(
        string(document::data::item::Button::UNUSED), devicePos, type, row);
    config.modify().setDeviceButtonName("", devicePos, type, row);
  } else {
    config.modify().setDeviceButtonAction(unusedCommands[0].toStdString(),
        devicePos, type, row);
    config.modify().setDeviceButtonName(unusedCommands[0].toStdString(),
        devicePos, type, row);
  }
  //assume infinite touch positions for the real world
  config.modify().setDeviceButtonPosition(row, devicePos, type, row);
  return true;
}

bool DeviceSoftButtonModel::removeButton(int row)
{
  uint32_t devicePos;

  config.data().getDevice(id, &devicePos);

  //only remove from the end
  row = rowCount() - 1;
  return config.modify().removeDeviceButtonCommand(devicePos, type, row);
}

QVariant DeviceSoftButtonModel::getDisplayData(const QModelIndex &index) const
{
  try {
    auto &button = getButtons().at(index.row());
    switch (static_cast<Column>(index.column())) {
      case Column::POSITION:
        return toPositionString(button.position.get());
      case Column::COMMAND:
        return QString::fromStdString(button.action.get());
      case Column::NAME:
        return QString::fromStdString(button.name.get());
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QVariant DeviceSoftButtonModel::getEditData(const QModelIndex &index) const
{
  try {
    auto &button = getButtons().at(index.row());
    switch (static_cast<Column>(index.column())) {
      case Column::POSITION:
        return toPositionString(button.position.get());
      case Column::COMMAND:
        return QString::fromStdString(button.action.get());
      case Column::NAME:
        return QString::fromStdString(button.name.get());
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QVariant DeviceSoftButtonModel::getTooltipData(const QModelIndex &index) const
{
  try {
    return columnSetup.at(mapColumn(index.column())).context;
  } catch (...) {
  }
  return {};
}

QVariant DeviceSoftButtonModel::getSelectionItemsData(
    const QModelIndex &index) const
{
  auto *device = config.data().getDevice(id);
  try {
    switch (static_cast<Column>(index.column())) {
      case Column::COMMAND: {
        auto cmds = getAvailableCommands(device);
        cmds.prepend(QString::fromUtf8(document::data::item::Button::UNUSED));
        return cmds;
      }
        break;
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QStringList models::DeviceSoftButtonModel::getUnusedCommands(
    const document::data::item::Device *device) const
{
  QStringList usedCommands;

  for (const auto &button : device->getHardButtons()) {
    usedCommands.push_back(QString::fromStdString(button.action.get()));
  }
  for (const auto &button : device->getSoftButtons()) {
    usedCommands.push_back(QString::fromStdString(button.action.get()));
  }
  auto availableCommands = getAvailableCommands(device);

  for (const auto &command : usedCommands) {
    availableCommands.removeAll(command);
  }
  return availableCommands;
}

/*
 * Activity Hard Buttons
 */

ActivityHardButtonModel::ActivityHardButtonModel(document::Config &config,
    uint32_t activityId, QObject *parent) :
    ButtonBaseModel(config, document::data::Item::ACTIVITY_HARD_BUTTON), id(
        activityId)
{
}

bool ActivityHardButtonModel::setData(const QModelIndex &index,
    const QVariant &value, int role)
{
  uint32_t activityPos;

  if (index.parent().isValid() || (role != Qt::EditRole)) {
    return false;
  }
  auto row = index.row();
  auto baseColumn = mapColumn(index.column());

  //checks
  if (!columnSetup.contains(baseColumn)) {
    return false;
  }
  if (columnSetup.at(baseColumn).isConst) { //caller should have used flags() method!
    return false;
  }
  if (row >= rowCount()) {
    return false;
  }
  auto currentValue = data(index, role);
  if (currentValue.isValid() && (currentValue == value)) {
    return true;
  }

  config.data().getActivity(id, &activityPos);

  switch (static_cast<Column>(index.column())) {
    case Column::DEVICE:
      return setDeviceData(activityPos, row, value);
    case Column::COMMAND:
      return config.modify().setActivityButtonAction(
          value.toString().toStdString(), activityPos, type, row);
    case Column::BUTTON:
      return config.modify().setActivityButtonName(
          value.toString().toStdString(), activityPos, type, row);
    default:
      return false;
  }

  //don't emit dataChanged event, is done inside observers anyway
  return true;
}

ButtonBaseModel::Column models::ActivityHardButtonModel::mapColumn(
    int viewColumn) const
{
  return mapColumn(static_cast<Column>(viewColumn));
}

ButtonBaseModel::Column models::ActivityHardButtonModel::mapColumn(
    Column viewColumn) const
{
  switch (viewColumn) {
    case Column::DEVICE:
      return ButtonBaseModel::Column::DEVICE;
    case Column::COMMAND:
      return ButtonBaseModel::Column::COMMAND;
    case Column::BUTTON:
      return ButtonBaseModel::Column::BUTTON;
    default:
      return ButtonBaseModel::Column::COUNT;
  }
}

int ActivityHardButtonModel::columnCount(const QModelIndex &parent) const
{
  return static_cast<int>(Column::COUNT);
}

const vector<document::data::item::Button>& ActivityHardButtonModel::getButtons() const
{
  return config.data().getActivity(id)->getHardButtons();
}

bool ActivityHardButtonModel::addButton(int row)
{
  uint32_t activityPos;
  uint32_t deviceId;
  QString commandToUse;

  auto devices = getAvailableDevices();
  if (devices.empty()) {
    tr("You need to create a device first!");
    return false;
  }
  try {
    //just assume we want the same device again
    deviceId = getButtons().at(row - 1).device.get();
  } catch (const out_of_range &ex) {
    deviceId = devices[0].first;
  }
  auto *device = config.data().getDevice(deviceId);
  if (device == nullptr) {
    return false;
  }

  auto commands = getAvailableCommands(device);
  auto buttons = getUnusedButtons();

  if (commands.size() == 0) {
    emit writeMsg(tr("No IR commands available to add for this device. "
        "Add an IR command first"));
    return false;
  }
  if (buttons.size() == 0) {
    emit writeMsg(tr("All hardware buttons are used. Can't add more"));
    return false;
  }
  auto buttonToUse = buttons[0];

  config.data().getActivity(id, &activityPos);
  auto ret = config.modify().addActivityButtonCommand(activityPos, type, row);
  if (ret != true) {
    return ret;
  }
  if (commands.contains(buttonToUse)) {
    commandToUse = buttonToUse;
  } else {
    commandToUse = commands[0];
  }
  config.modify().setActivityButtonDevice(deviceId, activityPos, type, row);
  config.modify().setActivityButtonAction(commandToUse.toStdString(),
      activityPos, type, row);
  config.modify().setActivityButtonName(buttonToUse.toStdString(), activityPos,
      type, row);
  return true;
}

bool ActivityHardButtonModel::removeButton(int row)
{
  uint32_t activityPos;

  config.data().getActivity(id, &activityPos);

  return config.modify().removeActivityButtonCommand(activityPos, type, row);
}

QVariant ActivityHardButtonModel::getDisplayData(const QModelIndex &index) const
{
  try {
    auto row = index.row();
    auto &button = getButtons().at(row);
    switch (static_cast<Column>(index.column())) {
      case Column::DEVICE: {
        auto deviceId = button.device.get();
        auto *device = config.data().getDevice(deviceId);
        if (device == nullptr) {
          return deviceId; //error
        }
        return QString::fromStdString(device->label.get());
      }
      case Column::COMMAND:
        return QString::fromStdString(button.action.get());
      case Column::BUTTON:
        return QString::fromStdString(button.name.get());
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QVariant ActivityHardButtonModel::getEditData(const QModelIndex &index) const
{
  try {
    auto &button = getButtons().at(index.row());
    switch (static_cast<Column>(index.column())) {
      case Column::DEVICE:
        return button.device.get();
      case Column::COMMAND:
        return QString::fromStdString(button.action.get());
      case Column::BUTTON:
        return QString::fromStdString(button.name.get());
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QVariant ActivityHardButtonModel::getTooltipData(const QModelIndex &index) const
{
  try {
    auto text = columnSetup.at(mapColumn(index.column())).context;
    switch (static_cast<Column>(index.column())) {
      case Column::BUTTON: {
        auto name = QString::fromStdString(
            getButtons().at(index.row()).name.get());
        text = tr("<b>%1</b><br>"
            "You can find it here:<br><br>"
            "<img src=':/res/buttons/%2.jpg' width='180' height='138'>").arg(
            text).arg(name);
        return text;
      }
      default:
        return text;
    }
  } catch (...) {
  }
  return {};
}

QVariant ActivityHardButtonModel::getSelectionItemsData(
    const QModelIndex &index) const
{
  try {
    auto row = index.row();
    auto &button = getButtons().at(row);
    switch (static_cast<Column>(index.column())) {
      case Column::DEVICE:
        return QVariant::fromValue(getAvailableDevices());
      case Column::COMMAND: {
        auto deviceId = button.device.get();
        auto *device = config.data().getDevice(deviceId);
        if (device == nullptr) {
          return {};
        }
        return getAvailableCommands(device);
      }
      case Column::BUTTON:
        return getUnusedButtons(button);
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

bool ActivityHardButtonModel::setDeviceData(uint32_t activityPos, int row, const QVariant &value)
{
  bool ok;
  auto deviceId = value.toUInt(&ok);
  if (!ok) {
    return false;
  }

  config.beginMacro(QObject::tr("Change Device"));

  auto res = config.modify().setActivityButtonDevice(value.toUInt(),
      activityPos, type, row);
  if (res != true) {
    config.endMacro();
    return false;
  }

  //check if currently set command is available for this device
  //if not, replace it with the first available in the new device
  auto *device = config.data().getDevice(deviceId);
  if (device == nullptr) {
    config.endMacro();
    return false;
  }
  try {
    auto currentCommand = QString::fromStdString(
        getButtons().at(row).action.get());
    auto availableCommands = getAvailableCommands(device);
    if (availableCommands.contains(currentCommand)) {
      //we are fine, just keep the command
      config.endMacro();
      return true;
    }
    if (availableCommands.empty()) {
      //device has no commands!
      emit writeMsg(tr("Device has no commands"));
      config.modify().setActivityButtonAction("", activityPos, type, row);
      config.endMacro();
      return true;
    }
    auto ret = config.modify().setActivityButtonAction(
        availableCommands[0].toStdString(), activityPos, type, row);
    config.endMacro();
    return ret;

  } catch (const out_of_range &ex) {
  }
  config.endMacro();
  return false;
}

/*
 * Activity Soft Buttons
 */

ActivitySoftButtonModel::ActivitySoftButtonModel(document::Config &config,
    uint32_t activityId, QObject *parent) :
    ButtonBaseModel(config, document::data::Item::ACTIVITY_SOFT_BUTTON), id(
        activityId)
{
}

bool ActivitySoftButtonModel::setData(const QModelIndex &index,
    const QVariant &value, int role)
{
  uint32_t activityPos;

  if (index.parent().isValid() || (role != Qt::EditRole)) {
    return false;
  }
  auto row = index.row();
  auto baseColumn = mapColumn(index.column());

  //checks
  if (!columnSetup.contains(baseColumn)) {
    return false;
  }
  if (columnSetup.at(baseColumn).isConst) { //caller should have used flags() method!
    return false;
  }
  if (row >= rowCount()) {
    return false;
  }
  auto currentValue = data(index, role);
  if (currentValue.isValid() && (currentValue == value)) {
    return true;
  }

  config.data().getActivity(id, &activityPos);

  switch (static_cast<Column>(index.column())) {
    case Column::DEVICE:
      return setDeviceData(activityPos, row, value);
    case Column::COMMAND:
      return setDeviceCommand(activityPos, row, value);
    case Column::NAME:
      return config.modify().setActivityButtonName(
          value.toString().toStdString(), activityPos, type, row);
    case Column::ICON:
      return config.modify().setActivityButtonFile(
          value.toString().toStdString(), activityPos, type, row);
    default:
      return false;
  }

  //don't emit dataChanged event, is done inside observers anyway
  return true;
}

ButtonBaseModel::Column models::ActivitySoftButtonModel::mapColumn(
    int viewColumn) const
{
  return mapColumn(static_cast<Column>(viewColumn));
}

ButtonBaseModel::Column models::ActivitySoftButtonModel::mapColumn(
    Column viewColumn) const
{
  switch (viewColumn) {
    case Column::POSITION:
      return ButtonBaseModel::Column::POSITION;
    case Column::DEVICE:
      return ButtonBaseModel::Column::DEVICE;
    case Column::COMMAND:
      return ButtonBaseModel::Column::COMMAND;
    case Column::NAME:
      return ButtonBaseModel::Column::NAME;
    case Column::ICON:
      return ButtonBaseModel::Column::ICON;
    default:
      return ButtonBaseModel::Column::COUNT;
  }
}

int ActivitySoftButtonModel::columnCount(const QModelIndex &parent) const
{
  return static_cast<int>(Column::COUNT);
}

const vector<document::data::item::Button>& ActivitySoftButtonModel::getButtons() const
{
  return config.data().getActivity(id)->getSoftButtons();
}

bool ActivitySoftButtonModel::addButton(int row)
{
  uint32_t activityPos;
  uint32_t deviceId;
  QString commandToUse;

  auto devices = getAvailableDevices();
  if (devices.empty()) {
    tr("You need to create a device first!");
    return false;
  }
  try {
    //just assume we want the same device again
    deviceId = getButtons().at(row - 1).device.get();
  } catch (const out_of_range &ex) {
    deviceId = devices[0].first;
  }
  auto *device = config.data().getDevice(deviceId);
  if (device == nullptr) {
    return false;
  }
  auto commands = getAvailableCommands(device);
  if (commands.size() == 0) {
    emit writeMsg(tr("No IR commands available to add for this device. "
        "Add an IR command first"));
    return false;
  }

  //only append to the end
  row = rowCount();

  auto *activity = config.data().getActivity(id, &activityPos);

  auto ret = config.modify().addActivityButtonCommand(activityPos, type, row);
  if (ret != true) {
    return ret;
  }
  config.modify().setActivityButtonDevice(deviceId, activityPos, type, row);
  config.modify().setActivityButtonFile("", activityPos, type, row);
  //for default -- first unused command.
  auto unusedCommands = getUnusedCommands(activity, device);
  if (unusedCommands.size() == 0) {
    config.modify().setActivityButtonAction(
        string(document::data::item::Button::UNUSED), activityPos, type, row);
    config.modify().setActivityButtonName("", activityPos, type, row);
  } else {
    config.modify().setActivityButtonAction(unusedCommands[0].toStdString(),
        activityPos, type, row);
    config.modify().setActivityButtonName(unusedCommands[0].toStdString(),
        activityPos, type, row);
  }
  //assume infinite touch positions for the real world
  config.modify().setActivityButtonPosition(row, activityPos, type, row);
  return true;
}

bool ActivitySoftButtonModel::removeButton(int row)
{
  uint32_t activityPos;

  config.data().getActivity(id, &activityPos);

  //only remove from the end
  row = rowCount() - 1;
  return config.modify().removeActivityButtonCommand(activityPos, type, row);
}

QVariant ActivitySoftButtonModel::getDisplayData(const QModelIndex &index) const
{
  try {
    auto row = index.row();
    auto &button = getButtons().at(row);
    switch (static_cast<Column>(index.column())) {
      case Column::POSITION:
        return toPositionString(button.position.get());
      case Column::DEVICE: {
        auto deviceId = button.device.get();
        auto *device = config.data().getDevice(deviceId);
        if (device == nullptr) {
          return ""; //entriy unused
        }
        return QString::fromStdString(device->label.get());
      }
      case Column::COMMAND:
        return QString::fromStdString(button.action.get());
      case Column::NAME:
        return QString::fromStdString(button.name.get());
      case Column::ICON:
        return QString::fromStdString(button.file.get());
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QVariant ActivitySoftButtonModel::getEditData(const QModelIndex &index) const
{
  try {
    auto &button = getButtons().at(index.row());
    switch (static_cast<Column>(index.column())) {
      case Column::POSITION:
        return toPositionString(button.position.get());
      case Column::DEVICE:
        return button.device.get();
      case Column::COMMAND:
        return QString::fromStdString(button.action.get());
      case Column::NAME:
        return QString::fromStdString(button.name.get());
      case Column::ICON:
        return QString::fromStdString(button.file.get());
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

QVariant ActivitySoftButtonModel::getTooltipData(const QModelIndex &index) const
{
  try {
    return columnSetup.at(mapColumn(index.column())).context;
  } catch (...) {
  }
  return {};
}

QVariant ActivitySoftButtonModel::getSelectionItemsData(
    const QModelIndex &index) const
{
  try {
    auto row = index.row();
    auto &button = getButtons().at(row);
    switch (static_cast<Column>(index.column())) {
      case Column::DEVICE:
        return QVariant::fromValue(getAvailableDevices());
      case Column::COMMAND: {
        auto deviceId = button.device.get();
        auto *device = config.data().getDevice(deviceId);
        if (device == nullptr) {
          return {QStringList( {QString::fromUtf8(document::data::item::Button::UNUSED)})};
        }
        auto cmds = getAvailableCommands(device);
        cmds.prepend(QString::fromUtf8(document::data::item::Button::UNUSED));
        return cmds;
      }
      case Column::ICON:
        return columnSetup.at(mapColumn(index.column())).selection;
      default:
        break;
    }
  } catch (const out_of_range &ex) {
  }
  return {};
}

bool ActivitySoftButtonModel::setDeviceData(uint32_t activityPos, int row,
    const QVariant &value)
{
  bool ok;
  auto deviceId = value.toUInt(&ok);
  if (!ok) {
    return false;
  }

  config.beginMacro(QObject::tr("Change Device"));

  auto res = config.modify().setActivityButtonDevice(value.toUInt(),
      activityPos, type, row);
  if (res != true) {
    config.endMacro();
    return false;
  }

  //check if currently set command is available for this device
  //if not, replace it with the first available in the new device
  auto *device = config.data().getDevice(deviceId);
  if (device == nullptr) {
    config.endMacro();
    return false;
  }
  try {
    auto currentCommand = QString::fromStdString(
        getButtons().at(row).action.get());
    auto availableCommands = getAvailableCommands(device);
    if (availableCommands.contains(currentCommand)) {
      //we are fine, just keep the command
      config.endMacro();
      return true;
    }
    if (availableCommands.empty()) {
      //device has no commands!
      emit writeMsg(tr("Device has no commands"));
      config.modify().setActivityButtonAction("", activityPos, type, row);
      config.endMacro();
      return true;
    }
    auto ret = config.modify().setActivityButtonAction(
        availableCommands[0].toStdString(), activityPos, type, row);
    config.endMacro();
    return ret;

  } catch (const out_of_range &ex) {
  }
  config.endMacro();
  return false;
}

bool ActivitySoftButtonModel::setDeviceCommand(uint32_t activityPos, int row,
    const QVariant &value)
{
  auto cmd = value.toString().toStdString();

  if (cmd == string(document::data::item::Button::UNUSED)) {
    config.modify().setActivityButtonDevice(0, activityPos, type, row);
  }
  return config.modify().setActivityButtonAction(cmd, activityPos, type, row);
}

QStringList models::ActivitySoftButtonModel::getUnusedCommands(
    const document::data::item::Activity *activity,
    const document::data::item::Device *device) const
{
  QStringList usedCommands;

  for (const auto &button : activity->getHardButtons()) {
    usedCommands.push_back(QString::fromStdString(button.action.get()));
  }
  for (const auto &button : activity->getSoftButtons()) {
    usedCommands.push_back(QString::fromStdString(button.action.get()));
  }
  auto availableCommands = getAvailableCommands(device);

  for (const auto &command : usedCommands) {
    availableCommands.removeAll(command);
  }
  return availableCommands;
}

}

