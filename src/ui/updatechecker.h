// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>

class QNetworkReply;

class UpdateChecker: public QObject
{
  Q_OBJECT

  public:
    explicit UpdateChecker(QObject *parent = nullptr);

    void checkForUpdates();

  private:
    void handleReply(QNetworkReply *reply);

    QNetworkAccessManager networkManager;
    bool checkInProgress = false;

//    const QUrl releaseApiUrl {
//      "https://api.github.com/repos/martin-wagner/harmony900tools/releases/latest" };
    //this includes pre-releases
    const QUrl releaseApiUrl {
      "https://api.github.com/repos/martin-wagner/harmony900tools/releases" };
    const QUrl releasesiUrl {
      "https://github.com/martin-wagner/harmony900tools/releases" };
};
