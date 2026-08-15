// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <type_traits>
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <QString>
#include <QList>

#ifdef _WIN32
  //we do reflection on the enum items below. we need those _exact_ strings...
  //fixme check side-effects of just nuking the macros
  #undef DEFAULT
  #undef DISPLAY
  #undef PASSTHROUGH
#endif

namespace document
{
namespace data
{
//use enum values exactly as in UserConfiguration.xml!
//all enums _must_ have "Unknown" as last item!.
enum class Locale { dan, deu, enu, esp, fin, fra, ita, nid, nor, plk, ptg, rus, sve, Unknown };
enum class TimeFormat { Military, Unknown };
enum class PvrType { Generic, Unknown };
enum class TunerInput { Tuner, Unknown };
enum class DeviceType { Amplifier, AudioVideoSwitch, Cd, ClimateControl, Computer, DvdCd,
    DvdCdGame, DvdCdRadio, GameConsole, Light, MediaCenterPC, Projector, Pvr, Receiver,
    SetTopBox, Television, Vcr, Unknown };
enum class StateMachineDeviceType { Power, Input, Screen, TVInput, AntennaOutput, Unknown };
enum class ActionType { None, StartAction, FinishAction, SetAction, ChangeAction, NextAction, PrevAction, ResetAction, Unknown };
enum class Operation { ForceValue, SendCommand, SendDelay, SendFlush, SendNumber, SetValue, Unknown };
enum class Modifier { None, Press, Hold, Unknown };
enum class ActivityType { VirtualCdMulti, VirtualDvd, VirtualGameConsole,
    VirtualGeneric, VirtualMusicServer, VirtualPvr, VirtualRadioSimple,
    VirtualSatelliteMusic, VirtualTelevisionN, VirtualVcr, Unknown};
enum class DeviceRole { DEFAULT, DISPLAY, VOLUME, LIGHTCONTROL, LIGHTCONTROL2, LIGHTCONTROL3, LIGHTCONTROL4,
    PASSTHROUGH, PASSTHROUGH2, PASSTHROUGH3, PASSTHROUGH4, Unknown};
enum class ActivityStartPage { Transport, Numbers, GameController, Unknown };
enum class ChannelButtonBehaviour { BasicChannels, Unknown };
enum class GuideButtonMode { TunerProgramGuide, Unknown };
enum class MediaButtonMode { ShowMedia, Unknown };
enum class HardButtons { Menu, Exit, UpArrow,  DownArrow,  Info,  Guide,  Red,  Green,  Yellow,  Blue,  VolumeUp,  VolumeDown,
    DirectionLeft,  DirectionUp,  DirectionRight,  DirectionDown,  Select,  ChannelUp,  ChannelDown,  VolumeMute,  PrevChannel,
    Rewind,  SkipBack,  Record,  Play,  Pause,  FastForward,  SkipForward,  Stop,  Number1,  Number2,  Number3,  Number4,
    Number5,  Number6,  Number7,  Number8,  Number9,  NumberPlus,  Number0, NumberEnter, Unknown }; //those are sorted by position on the remote. don't change!

//non-remote enums
enum class CodeType { NEC, KASEIKYO, SIRCS, Samsung32, PhilipsRC5, PhilipsRC6, None, Proprietary, Unknown };


/**
 * stores enums as enum value and string.
 *
 * string allows to dump original value if not known at software build time.
 */
template<typename T>
class Enum
{
  static_assert(std::is_enum_v<T>, "T must be an enum");

  public:
    Enum(const std::string &s);
    Enum(const QString &s) : Enum(s.toStdString()) {};
    Enum(const char *s) : Enum(std::string(s)) {}
    Enum(T v);

    static bool isEnumValue(std::string s);
    static std::string toString(T v);
    static QString toQString(T v);
    static std::vector<std::string> toStringList();
    static QStringList toQStringList();

    std::vector<std::string> getStringList() const;
    QStringList getQStringList() const;

    std::string getString() const;
    QString getQString() const;
    T getValue() const;

  protected:
    T value;
    std::string src;
};

/** convert locale to native string */
inline const std::string getNativeString(Locale v)
{
    switch (v) {
        case Locale::dan: return "Dansk";
        case Locale::deu: return "Deutsch";
        case Locale::enu: return "English";
        case Locale::esp: return "Español";
        case Locale::fin: return "Suomi";
        case Locale::fra: return "Français";
        case Locale::ita: return "Italiano";
        case Locale::nid: return "Nederlands";
        case Locale::nor: return "Norsk";
        case Locale::plk: return "Polski";
        case Locale::ptg: return "Português";
        case Locale::rus: return "Русский";
        case Locale::sve: return "Svenska";
        case Locale::Unknown: return "Unknown";
    }
    return "Unknown";
}

//limit magic_enum to cpp file -> must provide template types here.
template class Enum<Locale>;
template class Enum<TimeFormat>;
template class Enum<PvrType>;
template class Enum<TunerInput>;
template class Enum<DeviceType>;
template class Enum<StateMachineDeviceType>;
template class Enum<ActionType>;
template class Enum<Operation>;
template class Enum<Modifier>;
template class Enum<ActivityType>;
template class Enum<DeviceRole>;
template class Enum<ActivityStartPage>;
template class Enum<ChannelButtonBehaviour>;
template class Enum<GuideButtonMode>;
template class Enum<MediaButtonMode>;
template class Enum<HardButtons>;
template class Enum<CodeType>;

}
}

//#pragma pop_macro("DEFAULT")
//#pragma pop_macro("DISPLAY")
