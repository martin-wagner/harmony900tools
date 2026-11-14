// SPDX-License-Identifier: LGPL-2.1-or-later

#include <iostream>
#include <QDesktopServices>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QVersionNumber>
#include <QPushButton>
#include <nlohmann/json.hpp>

#include "updatechecker.h"
#include "version.h"

using json = nlohmann::json;

UpdateChecker::UpdateChecker(QObject *parent) :
    QObject(parent)
{
}

void UpdateChecker::checkForUpdates()
{
  if (checkInProgress) {
    return;
  }
  checkInProgress = true;

  QNetworkRequest request(releaseApiUrl);
  request.setHeader(QNetworkRequest::UserAgentHeader, "harmony900tools");
  request.setRawHeader("Accept", "application/vnd.github+json");
  request.setTransferTimeout(10000);

  auto *reply = networkManager.get(request);

  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    handleReply(reply);
    reply->deleteLater();
  });
}

void UpdateChecker::handleReply(QNetworkReply *reply)
{
  json releases;
  checkInProgress = false;
  qsizetype parsedLength;

  if (reply->error() != QNetworkReply::NoError) {
    auto responseBody = reply->readAll();
    auto httpStatus =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    std::cout << "Update check failed:\n" << "  Error: "
        << reply->errorString().toStdString() << '\n' << "  HTTP status: "
        << httpStatus << '\n' << "  Response: " << responseBody.toStdString()
        << std::endl;
    return;
  }

  auto jsonText = reply->readAll().toStdString();
  std::cout << "json: " << jsonText << std::endl;
  try {
    releases = json::parse(jsonText);
  } catch (const json::exception&) {
    return;
  }

  if (!releases.is_array()) {
    return;
  }

#if 0 //testing
  const QVersionNumber currentVersion(0, 0, 0);
#else
  const QVersionNumber currentVersion(BuildInfo::versionMajor,
      BuildInfo::versionMinor, BuildInfo::versionPatch);
#endif
  for (const auto &release : releases) {
    if (!release.is_object()) {
      continue;
    }
    parsedLength = 0;
    auto tagName = QString::fromStdString(release.value("tag_name", ""));
    auto releaseUrl = QString::fromStdString(release.value("html_url", ""));
    auto releaseNotes = QString::fromStdString(release.value("body", ""));
    auto versionText =
        tagName.startsWith('v', Qt::CaseInsensitive) ? tagName.mid(1) : tagName;

    const QVersionNumber latestVersion = QVersionNumber::fromString(versionText,
        &parsedLength);

    if (latestVersion.isNull() || (parsedLength != versionText.size())) {
      continue;
    }

    const QUrl url(releaseUrl);
    if ((url.scheme() != "https") || (url.host() != "github.com")) {
      continue;
    }

    if (QVersionNumber::compare(latestVersion, currentVersion) <= 0) {
      continue;
    }

    QMessageBox messageBox;
    messageBox.setIcon(QMessageBox::Information);
    messageBox.setWindowTitle(tr("Update available"));
    messageBox.setText(tr("A new version is available."));
    messageBox.setInformativeText(
        tr("Installed version: %1\nAvailable version: %2").arg(
            currentVersion.toString()).arg(latestVersion.toString()));
    messageBox.setDetailedText(
        releaseNotes.isEmpty() ?
            tr("No release notes available.") : releaseNotes);
    QPushButton *openButton = messageBox.addButton(tr("Open releases page"),
        QMessageBox::AcceptRole);
    messageBox.addButton(QMessageBox::Close);
    messageBox.exec();
    if (messageBox.clickedButton() == openButton) {
      QDesktopServices::openUrl(releasesiUrl);
    }
    return;
  }
}
