// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QLocale"

#include "version.h"
#include "create.h"
#include "lib/uid.h"
#include "lib/timestamp.h"
#include "lib/username.h"
#include "document/data/catalogue.h"

using namespace std;

namespace document
{
namespace files
{

Create::Create(data::ConfigData &c) :
    c(c)
{
}

void Create::write(data::CmdCatalogue *worker)
{
  QString firstName;
  QString lastName;

  worker->setUserId(0x1ee7);
  addId(0x1ee7);
  worker->setUserMetadata();

  lib::getUserFirstLastName(firstName, lastName);
  worker->setUserFirstName(firstName);
  worker->setUserLastName(lastName);
  emit writeLog(LogLevel::Debug, tr("User: %1 %2").arg(firstName).arg(lastName),
      ContentType::PlainText);

  QLocale locale = QLocale::system();
  QString languageName = QLocale::languageToString(locale.language());
  emit writeLog(LogLevel::Debug, tr("Language: %1").arg(languageName),
      ContentType::PlainText);
  switch (locale.language()) {
    case QLocale::German:
      worker->setUserLocale(data::Enum<data::Locale>(data::Locale::deu));
      //24h clock. why military???
      worker->setUserTimeFormat(
          data::Enum<data::TimeFormat>(data::TimeFormat::Military));
      break;
    case QLocale::English:
      worker->setUserLocale(data::Enum<data::Locale>(data::Locale::enu));
      //fixme what is the name for 12h/am/pm clock?
      worker->setUserTimeFormat(
          data::Enum<data::TimeFormat>(data::TimeFormat::Military));
      break;
    default: {
      emit writeLog(LogLevel::Warning,
          tr("System Language unknown, falling back to english"),
          ContentType::PlainText);
      worker->setUserLocale(data::Enum<data::Locale>(data::Locale::enu));
      break;
    }
  }

  worker->setUserTrainingWheels(true);

  worker->setControllerId(0);
  worker->setControllerMetadata("Protocol Code", "Logitech", "Harmony 1000-ish",
      "Harmony 1000-ish");
}

void Create::addId(uint32_t id)
{
  lib::UidGenerator::getInstance().markUsed(id);
}

}
}

