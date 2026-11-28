// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include <string>

#include <QList>
#include <QMap>
#include <QWidget>

#include "context.h"
#include "document/files/database/importer.h"
#include "ui/concordConnection.h"

class QCheckBox;
class QAction;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QProgressBar;
class QPushButton;
class QToolButton;

namespace database
{

/**
 * pickaxe search for the device database.
 *
 * the user selects one or more manufacturers (mandatory) and learns three
 * commands from a remote. with a single manufacturer the devices to check can
 * be filtered/selected, with several all their devices are checked. the
 * database then checks which of the devices contain matching commands. matching devices are listed
 * and can be imported.
 */
class DevicePickaxeWidget: public QWidget
{
  Q_OBJECT

  public:
    /** the learn actions are owned by the connection, the widget only uses them */
    explicit DevicePickaxeWidget(Context &ctx, QAction *actionLearnIrSingle,
        QAction *actionLearnIrStream, QWidget *parent = nullptr);

    /** sets the database to search in, nullptr clears the view */
    void setCatalogue(const document::files::Database *catalogue);

  public slots:
    /** connection state of the remote. without a remote the widget is disabled */
    void setLearningAvailable(bool available);

    /** answer to a learn action. ignored if we are not waiting for one */
    void onLearnedCommand(ConcordConnection::LearnedCommandMode m, const binary::TimingStream &timing, uint32_t carrier);

    /** learning failed (e.g. timeout). ignored if we are not waiting for one */
    void onLearnFailed(const QString &message);

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType) const;
    void writeMsg(const QString &message) const;
    void deviceSelected(const QString &manufacturerName, const QString &deviceName);

  private slots:
    void onManufacturerTextChanged(const QString &text);
    void onManufacturerSelectionChanged();
    void onDeviceTextChanged(const QString &text);
    void onDeviceSelectionChanged();
    void onResultSelectionChanged();
    void onButtonSelectionChanged(int row);
    void onLearnTriggered();
    void onPickaxeClicked();
    void onPickaxeProgress(int checked, int total);
    void onAbortClicked();
    void onImportClicked();
    void onClearManufacturerClicked();

  protected:
    Context &ctx;
    QAction *learnSingleAction = nullptr;
    QAction *learnStreamAction = nullptr;

  private:
    static constexpr int COMMAND_COUNT = 3;
    inline static const QString POWER_BUTTON = "PowerToggle";

  private:
    void createView();
    void createConnections();

    void updateManufacturers();
    void updateDevices();
    void updateDeviceControls();
    void updateDeviceInfo();
    void updateHint();
    void updateButtonChoices();
    void updateLearnControls();
    void updatePickaxeButton();
    void updateImportButton();
    void setSearchRunning(bool running);

    static void addDeviceItem(QListWidget *list, const document::files::Database::DeviceKey &key);
    static document::files::Database::DeviceKey getDeviceKey( const QListWidgetItem *item);
    static bool isLessDeviceKey(const document::files::Database::DeviceKey &a, const document::files::Database::DeviceKey &b);

    QStringList getSelectedManufacturers() const;
    QMap<QString, QStringList> getDevicesToCheck() const;
    document::files::Database::DeviceKey getSelectedResult() const;

    const document::files::Database *catalogue = nullptr;

    bool searchRunning = false;
    bool learningAvailable = false;
    /** row we wait for a learn answer, -1 if none */
    int learningRow = -1;
    /** row whose learn button was pressed last, -1 if none */
    int pendingRow = -1;
    std::array<std::optional<document::files::Database::LearnedCommand>, COMMAND_COUNT> learned;

    /** set by the abort button, checked during the search */
    bool pickaxeAborted = false;

    QLineEdit *manufacturerEdit = nullptr;
    QPushButton *clearManufacturerButton = nullptr;
    QListWidget *manufacturerList = nullptr;
    QLineEdit *deviceEdit = nullptr;
    QListWidget *deviceList = nullptr;
    QListWidget *resultList = nullptr;

    std::array<QComboBox*, COMMAND_COUNT> buttonCombo{};
    std::array<QToolButton*, COMMAND_COUNT> learnSingleButton{};
    std::array<QToolButton*, COMMAND_COUNT> learnStreamButton{};
    std::array<QLabel*, COMMAND_COUNT> learnStatus{};

    QPushButton *pickaxeButton = nullptr;
    QPushButton *abortButton = nullptr;
    QProgressBar *progressBar = nullptr;

    QLabel *explanationLabel = nullptr;
    QLabel *hintLabel = nullptr;
    QWidget *contentWidget = nullptr;
    QLabel *deviceInfoLabel = nullptr;
    QCheckBox *tryConvertProtocolCheck = nullptr;
    QPushButton *importButton = nullptr;
};

}
