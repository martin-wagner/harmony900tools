// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QStringList>

#include <functional>

#include <zlib.h>
#include <minizip/zip.h>
#include <minizip/unzip.h>

namespace lib {

unzFile openZipForRead(const QString &path);
zipFile openZipForWrite(const QString &path);
gzFile openGzForRead(const QString &path);

bool zipDirectory(zipFile &zf, const QString &baseDir, const QStringList executableFileNames = {});
bool unzipToDirectory(unzFile &uf, const QString &destDir);

/**
 * Decompresses a single gzip file to destPath.
 *
 * progress(done, total) is called regularly with the compressed bytes
 * processed so far and the compressed file size. Returning false from it
 * cancels the operation. On failure or cancel the partial destination file
 * is removed.
 */
bool gunzipToFile(const QString &gzPath, const QString &destPath,
    const std::function<bool(uint64_t done, uint64_t total)> &progress = nullptr);

}
