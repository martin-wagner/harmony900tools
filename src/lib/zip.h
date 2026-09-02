// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QStringList>

#include <minizip/zip.h>
#include <minizip/unzip.h>

namespace lib {

unzFile openZipForRead(const QString &path);
zipFile openZipForWrite(const QString &path);

bool zipDirectory(zipFile &zf, const QString &baseDir, const QStringList executableFileNames = {});
bool unzipToDirectory(unzFile &uf, const QString &destDir);

}

