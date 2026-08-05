// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "document/data/catalogue.h"
#include "base.h"

namespace document
{
  class Config;
  namespace data {
    class ConfigData;
    namespace item {
      class Button;
    }
  }
}

namespace models
{

//this can't use Enum<type>, as some begin with numbers!
inline static const QStringList icons {
    "3musicNote", "filmstrip", "pause", "replayTV", "slowMotion", "teletext_on",
    "55_sbNowPlaying", "heart", "playList", "rewind", "slowPlay", "teletext_red_default",
    "A", "list", "play", "sbAdd", "SoundEffect", "teletext_yellow_default",
    "aspect", "live", "podcasts", "sbFavorites", "SoundMode", "thumbsdown_default",
    "back", "mail", "powerOFF", "sbNowPlaying", "square", "thumbsup_default",
    "B", "mediaCenter", "powerOnOff", "sbPodcast", "star", "tivo",
    "Cart", "myDVR", "powerON", "sbRepeat", "start", "topMenu",
    "cbox_circle_default", "myMusic", "PS-Circle", "sbSearch", "stop2", "triangle",
    "cbox_square_default", "myPictures", "PS-Cross", "sbShuffle", "stop", "VOD",
    "cbox_triangle_default", "myRadio", "PS-Square", "search", "Subtitle", "xBox_Button",
    "circle", "myTV", "PS_Triangle", "shuffle", "switchDisplay", "X",
    "clock", "myVideo", "purple", "skipBack", "teletext_blue_default", "Y",
    "closedCaption", "nowplaying", "record", "skipForward", "teletext_green_default",
    "eject", "off", "repeat", "sky", "teletext_off",
    "fastforward", "on", "replay", "sleep", "teletext_onoff"
};

//Model for buttons
class ButtonBaseModel: public QAbstractItemModel
{
  Q_OBJECT
  public:
    enum class Column {
      DEVICE,
      COMMAND,
      BUTTON,
      NAME,
      ICON,
      POSITION,

      COUNT
    };
    virtual Column mapColumn(int viewColumn) const = 0;

    const std::map<Column, Setup> columnSetup = {
        { Column::DEVICE,        { "Device",   "Device to use for this button", "int", false,  {}, } },
        { Column::COMMAND,       { "Command",  "The IR command you want to send", "Enum", false, {}, } },
        { Column::BUTTON,        { "Button",   "Link to this button on the remote", "Enum", false, {}, } },
        { Column::NAME,          { "Name",     "Name on the screen", "QString", false, {}, } },
        { Column::ICON,          { "Icon",     "Use this icon instead of name", "Enum", false, {QVariant(icons)}, } },
        { Column::POSITION,      { "Position", "Where to place the button on the screen", "Enum", true, {}, } },
    };

    inline static const int SOFTBUTTONS_PER_PAGE = 6;

  public:
    ButtonBaseModel(document::Config &config, document::data::Item item, QObject *parent = nullptr);
    ~ButtonBaseModel() override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = { }) const override;
    virtual QModelIndex parent(const QModelIndex &index) const override;

    virtual int rowCount(const QModelIndex &parent = { }) const override;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    virtual bool insertColumns(int position, int columns, const QModelIndex &parent = { }) override;
    virtual bool removeColumns(int position, int columns, const QModelIndex &parent = { }) override;
    virtual bool insertRows(int position, int rows, const QModelIndex &parent = { }) override;
    virtual bool removeRows(int position, int rows, const QModelIndex &parent = { }) override;

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

  protected:
    //accessors
    virtual const std::vector<document::data::item::Button> &getButtons() const = 0;
    virtual bool addButton(int row) = 0;
    virtual bool removeButton(int row) = 0;
    virtual QVariant getDisplayData(const QModelIndex &index) const = 0;
    virtual QVariant getEditData(const QModelIndex &index) const = 0;
    virtual QVariant getTooltipData(const QModelIndex &index) const = 0;
    virtual QVariant getBackgroundData(const QModelIndex &index) const = 0;
    virtual QVariant getForegroundData(const QModelIndex &index) const = 0;
    virtual QVariant getSelectionItemsData(const QModelIndex &index) const = 0;

  protected:
    document::Config &config;
    document::data::Item item;

    void createActions();

  protected:
    QStringList getUnusedButtons(const document::data::item::Button &button) const;
    QStringList getUnusedButtons() const;
    QString toPositionString(int pos) const;
    QStringList getAvailableCommands(const document::data::item::Device *device) const;
    QList<QPair<uint32_t, QString>> getAvailableDevices() const;

  private slots:
    void itemChangedObserver(document::data::Item item, int pos);
    void itemAboutToBeAddedObserver(document::data::Item item, int pos);
    void itemAddedObserver(document::data::Item item, int pos);
    void itemAboutToBeRemovedObserver(document::data::Item item, int pos);
    void itemRemovedObserver(document::data::Item item, int pos);
};

//Model for device hardware buttons
class DeviceHardButtonModel: public ButtonBaseModel
{
  Q_OBJECT
  public:
    DeviceHardButtonModel(document::Config &config, uint32_t deviceId, QObject *parent = nullptr);

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    enum class Column {
      COMMAND,
      BUTTON,

      COUNT
    };
    virtual ButtonBaseModel::Column mapColumn(int viewColumn) const;
    virtual ButtonBaseModel::Column mapColumn(Column viewColumn) const;

    virtual int columnCount(const QModelIndex &parent = { }) const override;

  protected:
    //implement
    const std::vector<document::data::item::Button> &getButtons() const override;
    bool addButton(int row) override;
    bool removeButton(int row) override;
    QVariant getDisplayData(const QModelIndex &index) const override;
    QVariant getEditData(const QModelIndex &index) const override;
    QVariant getTooltipData(const QModelIndex &index) const override;
    QVariant getBackgroundData(const QModelIndex &index) const override;
    QVariant getForegroundData(const QModelIndex &index) const override;
    QVariant getSelectionItemsData(const QModelIndex &index) const override;

  protected:
    const document::data::item::Device *device;
    uint32_t devicePos;
    const document::data::item::ButtonType type = document::data::item::ButtonType::Hard;
};

//Model for device touch buttons
class DeviceSoftButtonModel: public ButtonBaseModel
{
  Q_OBJECT
  public:
    DeviceSoftButtonModel(document::Config &config, uint32_t deviceId, QObject *parent = nullptr);

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    enum class Column {
      POSITION,
      COMMAND,
      NAME,

      COUNT
    };
    virtual ButtonBaseModel::Column mapColumn(int viewColumn) const;
    virtual ButtonBaseModel::Column mapColumn(Column viewColumn) const;

    virtual int columnCount(const QModelIndex &parent = { }) const override;

  protected:
    //implement
    const std::vector<document::data::item::Button> &getButtons() const override;
    bool addButton(int row) override;
    bool removeButton(int row) override;
    QVariant getDisplayData(const QModelIndex &index) const override;
    QVariant getEditData(const QModelIndex &index) const override;
    QVariant getTooltipData(const QModelIndex &index) const override;
    QVariant getBackgroundData(const QModelIndex &index) const override;
    QVariant getForegroundData(const QModelIndex &index) const override;
    QVariant getSelectionItemsData(const QModelIndex &index) const override;

  private:
    QStringList getUnusedCommands(const document::data::item::Device *device) const;

  protected:
    const document::data::item::Device *device;
    uint32_t devicePos;
    const document::data::item::ButtonType type = document::data::item::ButtonType::Soft;
};

//Model for activity hardware buttons
class ActivityHardButtonModel: public ButtonBaseModel
{
  Q_OBJECT
  public:
    ActivityHardButtonModel(document::Config &config, uint32_t activityId, QObject *parent = nullptr);

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    enum class Column {
      DEVICE,
      COMMAND,
      BUTTON,

      COUNT
    };
    virtual ButtonBaseModel::Column mapColumn(int viewColumn) const;
    virtual ButtonBaseModel::Column mapColumn(Column viewColumn) const;

    virtual int columnCount(const QModelIndex &parent = { }) const override;

  protected:
    //implement
    const std::vector<document::data::item::Button> &getButtons() const override;
    bool addButton(int row) override;
    bool removeButton(int row) override;
    QVariant getDisplayData(const QModelIndex &index) const override;
    QVariant getEditData(const QModelIndex &index) const override;
    QVariant getTooltipData(const QModelIndex &index) const override;
    QVariant getBackgroundData(const QModelIndex &index) const override;
    QVariant getForegroundData(const QModelIndex &index) const override;
    QVariant getSelectionItemsData(const QModelIndex &index) const override;

  protected:
    bool setDeviceData(int row, const QVariant &value);

  protected:
    const document::data::item::Activity *activity;
    uint32_t activityPos;
    const document::data::item::ButtonType type = document::data::item::ButtonType::Hard;
};


class ActivitySoftButtonModel: public ButtonBaseModel{};

}

