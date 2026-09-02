// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>

#ifdef _WIN32
#include <minizip/ioapi.h>
#include <minizip/iowin32.h>
#endif

#include "zip.h"


using namespace std;

namespace lib
{

static const uLong UTF8_FLAG = 0x0800;

unzFile openZipForRead(const QString &path)
{
#ifdef _WIN32
  zlib_filefunc64_def ffunc;
  fill_win32_filefunc64W(&ffunc);
  return unzOpen2_64(path.toStdWString().c_str(), &ffunc);
#else
  return unzOpen64(QFile::encodeName(path).constData());
#endif
}

zipFile openZipForWrite(const QString &path)
{
#ifdef _WIN32
  zlib_filefunc64_def ffunc;
  fill_win32_filefunc64W(&ffunc);
  return zipOpen2_64(path.toStdWString().c_str(), APPEND_STATUS_CREATE, nullptr, &ffunc);
#else
  return zipOpen64(QFile::encodeName(path).constData(), APPEND_STATUS_CREATE);
#endif
}

bool zipDirectory(zipFile &zf, const QString &baseDir,
    const QStringList executableFileNames)
{
  QDirIterator it(baseDir,
      QDir::Files | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot,
      QDirIterator::Subdirectories);

  while (it.hasNext()) {
    it.next();

    QString relPath = QDir(baseDir).relativeFilePath(it.filePath());
    relPath.replace(u'\\', u'/'); // zip spec uses forward slashes

    QFile file(it.filePath());
    if (!file.open(QIODevice::ReadOnly)) {
      qWarning() << "zipDirectory: cannot open" << it.filePath();
      return false;
    }
    QByteArray data = file.readAll();
    file.close();

    QByteArray entryName = relPath.toUtf8();
    zip_fileinfo fi = { };

    if (executableFileNames.contains(QFileInfo(it.filePath()).fileName())) {
      fi.external_fa = 0100775 << 16;
    }

    int err = zipOpenNewFileInZip4_64(zf, entryName.constData(), &fi, nullptr,
        0, nullptr, 0, nullptr,
        Z_DEFLATED,
        Z_DEFAULT_COMPRESSION, 0,                  // raw
        -MAX_WBITS,         // raw deflate (zip format)
        DEF_MEM_LEVEL,
        Z_DEFAULT_STRATEGY, nullptr, 0,         // no password
        (3 << 8) | 23,      // Unix, ZIP 2.3
        UTF8_FLAG,          // flagBase: mark filename as UTF-8
        1                   // zip64
        );

    if (err != ZIP_OK) {
      qWarning() << "zipDirectory: failed to open entry" << relPath;
      return false;
    }

    zipWriteInFileInZip(zf, data.constData(),
        static_cast<unsigned>(data.size()));
    zipCloseFileInZip(zf);
  }

  return true;
}

bool unzipToDirectory(unzFile &uf, const QString &destDir)
{
  unz_global_info64 gi;
  if (unzGetGlobalInfo64(uf, &gi) != UNZ_OK) {
    return false;
  }

  for (ZPOS64_T i = 0; i < gi.number_entry; ++i) {
    unz_file_info64 fi;
    char nameBuffer[2048] = { };

    if (unzGetCurrentFileInfo64(uf, &fi, nameBuffer, sizeof(nameBuffer) - 1,
        nullptr, 0, nullptr, 0) != UNZ_OK) {
      return false;
    }

    // decode filename: bit 11 set means UTF-8, otherwise fall back to local 8-bit
    QString entryName;
    if (fi.flag & UTF8_FLAG) {
      entryName = QString::fromUtf8(nameBuffer);
    } else {
      entryName = QString::fromLocal8Bit(nameBuffer);
    }
    entryName.replace(u'\\', u'/');

    // skip pure directory entries
    if (entryName.endsWith(u'/')) {
      if (i + 1 < gi.number_entry) {
        unzGoToNextFile(uf);
      }
      continue;
    }

    QString outPath = destDir + u'/' + entryName;
    QDir().mkpath(QFileInfo(outPath).absolutePath());

    if (unzOpenCurrentFile(uf) != UNZ_OK) {
      qWarning() << "unzipToDirectory: failed to open entry" << entryName;
      return false;
    }

    QFile outFile(outPath);
    if (outFile.open(QIODevice::WriteOnly)) {
      static const int CHUNK = 65536;
      QByteArray buf(CHUNK, Qt::Uninitialized);
      int bytesRead = 0;

      while ((bytesRead = unzReadCurrentFile(uf, buf.data(), CHUNK)) > 0) {
        outFile.write(buf.constData(), bytesRead);
      }
      outFile.close();
    } else {
      qWarning() << "unzipToDirectory: cannot write" << outPath;
      return false;
    }

    unzCloseCurrentFile(uf);

    if (i + 1 < gi.number_entry) {
      unzGoToNextFile(uf);
    }
  }

  return true;
}

} // namespace

