// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <array>
#include <cstdint>
#include <optional>

#include <QFutureWatcher>
#include <QList>
#include <QWidget>

#include "context.h"
#include "document/files/database/importer.h"

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QProgressBar;
class QPushButton;

namespace database
{

/**
 * pickaxe search for the device database.
 *
 * the user selects a manufacturer (mandatory), optionally filters/selects
 * devices and learns three commands from a remote. the database then checks
 * which of the devices contain matching commands. matching devices are listed
 * and can be imported.
 */
class DevicePickaxeWidget: public QWidget
{
  Q_OBJECT

  public:
    explicit DevicePickaxeWidget(Context &ctx, QWidget *parent = nullptr);
    ~DevicePickaxeWidget() override;

    /** sets the database to search in, nullptr clears the view */
    void setCatalogue(const document::files::Database *catalogue);

  public slots:
    /** connection state of the remote. without a remote the widget is disabled */
    void setLearningAvailable(bool available);

    /** answer to requestLearn(). ignored if we are not waiting for one */
    void onLearnedCommand(const binary::TimingStream &timing, uint32_t carrier);

    /** learning failed (e.g. timeout). ignored if we are not waiting for one */
    void onLearnFailed(const QString &message);

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType) const;
    void writeMsg(const QString &message) const;
    void deviceSelected(const QString &manufacturerName, const QString &deviceName);

    /** asks the remote to learn one ir frame, answer via onLearnedCommand / onLearnFailed */
    void requestLearn();

  private slots:
    void onManufacturerTextChanged(const QString &text);
    void onManufacturerSelectionChanged();
    void onDeviceTextChanged(const QString &text);
    void onDeviceSelectionChanged();
    void onResultSelectionChanged();
    void onButtonSelectionChanged(int row);
    void onLearnClicked(int row);
    void onPickaxeClicked();
    void onPickaxeProgress(int checked, int total);
    void onPickaxeFinished();
    void onImportClicked();
    void onClearManufacturerClicked();

  protected:
    Context &ctx;

  private:
    static constexpr int COMMAND_COUNT = 3;
    inline static const QString POWER_BUTTON = "Power Toggle";

  private:
    void createView();
    void createConnections();

    void updateManufacturers();
    void updateDevices();
    void updateDeviceInfo();
    void updateHint();
    void updateButtonChoices();
    void updateLearnControls();
    void updatePickaxeButton();
    void updateImportButton();
    void setSearchRunning(bool running);
    QString getSelectedManufacturer() const;
    QStringList getDevicesToCheck() const;
    QString getSelectedResult() const;

    const document::files::Database *catalogue = nullptr;

    QString resultManufacturer;
    bool searchRunning = false;
    bool learningAvailable = false;
    /** row we wait for a learn answer, -1 if none */
    int learningRow = -1;
    std::array<std::optional<document::files::Database::LearnedCommand>, COMMAND_COUNT> learned;

    QFutureWatcher<QList<document::files::Database::Device>> *pickaxeWatcher = nullptr;

    QLineEdit *manufacturerEdit = nullptr;
    QPushButton *clearManufacturerButton = nullptr;
    QListWidget *manufacturerList = nullptr;
    QLineEdit *deviceEdit = nullptr;
    QListWidget *deviceList = nullptr;
    QListWidget *resultList = nullptr;

    std::array<QComboBox*, COMMAND_COUNT> buttonCombo{};
    std::array<QPushButton*, COMMAND_COUNT> learnButton{};
    std::array<QLabel*, COMMAND_COUNT> learnStatus{};

    QPushButton *pickaxeButton = nullptr;
    QProgressBar *progressBar = nullptr;

    QLabel *explanationLabel = nullptr;
    QLabel *hintLabel = nullptr;
    QWidget *contentWidget = nullptr;
    QLabel *deviceInfoLabel = nullptr;
    QCheckBox *tryConvertProtocolCheck = nullptr;
    QPushButton *importButton = nullptr;
};

}
