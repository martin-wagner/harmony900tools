// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QString>

#include <minizip/zip.h>
#include <minizip/unzip.h>

namespace lib {

bool zipDirectory(zipFile zf, const QString &baseDir);
bool unzipToDirectory(unzFile uf, const QString &destDir);

}

