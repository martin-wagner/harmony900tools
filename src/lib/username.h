// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QString>
#include <QStringList>

#if defined(Q_OS_WIN)
#include <windows.h>
#include <secext.h>
#elif defined(Q_OS_LINUX)
#include <pwd.h>
#include <unistd.h>
#endif

/**
 * @brief Best-effort, cross-platform (Windows/Linux) retrieval of the
 *        verbose (display) user name, split into first/last name.
 *
 * There is no reliable structured first/last name field on either
 * platform, so the underlying full name string is split on whitespace
 * as a heuristic. If only one token is found, it is placed in the
 * first name and the last name is left empty.
 *
 * Header-only. On Windows, requires linking against Secur32.lib.
 */
namespace lib
{
/**
 * @brief Retrieves the raw verbose/display user name from the OS.
 * @return The full name string, or an empty string if unavailable.
 */
inline QString getFullUserName()
{
#if defined(Q_OS_WIN)
  wchar_t buffer[1024];
  ULONG size = sizeof(buffer) / sizeof(buffer[0]);

  if (GetUserNameExW(NameDisplay, buffer, &size) != 0) {
    return QString::fromWCharArray(buffer);
  }

  return QString();
#elif defined(Q_OS_LINUX)
  struct passwd *pw = getpwuid(getuid());

  if (pw != nullptr && pw->pw_gecos != nullptr) {
    QString gecos = QString::fromLocal8Bit(pw->pw_gecos);
    // GECOS field can contain comma-separated extra info (room, phone, ...);
    // the full name is conventionally the first entry.
    QString name = gecos.section(',', 0, 0).trimmed();
    return name;
  }

  return QString();
#else
  return QString();
#endif
}

/**
 * @brief Splits a full name string into first and last name, best effort.
 * @param fullName The full name string (e.g. from getFullUserName()).
 * @param firstName Output parameter, receives the first name (or the
 *                  only token found).
 * @param lastName Output parameter, receives the last name, or is left
 *                 empty if only one token was found.
 */
inline void splitFullName(const QString &fullName, QString &firstName,
    QString &lastName)
{
  firstName.clear();
  lastName.clear();

  QString normalized = fullName;
  normalized.replace(',', ' ');

  QStringList tokens = normalized.split(' ', Qt::SkipEmptyParts);

  if (tokens.isEmpty()) {
    return;
  }

  if (tokens.size() >= 1) {
    firstName = tokens.first();
  }
  if (tokens.size() >= 2) {
    lastName = tokens.last();
  }
}

/**
 * @brief Convenience wrapper: retrieves the OS verbose user name and
 *        splits it into first/last name, best effort.
 * @param firstName Output parameter, receives the first name.
 * @param lastName Output parameter, receives the last name, or is left
 *                 empty if only one token was found.
 * @return true if a full name could be retrieved from the OS, false otherwise.
 */
inline bool getUserFirstLastName(QString &firstName, QString &lastName)
{
  QString fullName = getFullUserName();

  if (fullName.isEmpty()) {
    firstName.clear();
    lastName.clear();
    return false;
  }

  splitFullName(fullName, firstName, lastName);
  return true;
}
}
