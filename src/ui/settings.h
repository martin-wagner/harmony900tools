#pragma once

#include <QDialog>
#include <QString>
#include <QVariant>
#include <QList>
#include <QMap>

class QTabWidget;
class QDialogButtonBox;
class QFormLayout;
class QWidget;
class QAbstractButton;

// ── Setting descriptor ────────────────────────────────────────────────────────
enum class SettingType
{
  String, Int, Double, Bool, MultiSelection   // combo box
};

struct SettingDef
{
    QString key;              // unique key used in value map
    QString label;            // display label
    QString helpText;         // context / tooltip help
    SettingType type;
    QVariant defaultValue;
    QString tab;              // empty → "General"

    // numeric limits (ignored for other types)
    QVariant minValue;
    QVariant maxValue;

    // multiselection options  (SettingType::MultiSelection only)
    // each entry: display string + associated QVariant (int or string)
    QList<QPair<QString, QVariant>> options;
};

// ── Dialog ────────────────────────────────────────────────────────────────────

class Settings: public QDialog
{
  Q_OBJECT

  public:
    explicit Settings(QWidget *parent = nullptr);

    // Add a setting. Call before show().
    void addSetting(const SettingDef &def);

    // Read / write current values (key → value).
    QMap<QString, QVariant> values() const;
    QVariant value(const QString &key) const;
    void setValues(const QMap<QString, QVariant> &vals);

  signals:
    // Emitted on OK and Apply with the current value map.
    void settingsAccepted(const QMap<QString, QVariant> &values);

  private slots:
    void onApply();
    void onRestoreDefaults();
    void onButtonClicked(QAbstractButton *button);

  private:
    void createView();
    QWidget* getOrCreateTab(const QString &tabName);
    QFormLayout* layoutForTab(const QString &tabName);
    void applyDef(const SettingDef &def, bool useDefault);

    // Returns the current widget value for a key.
    QVariant widgetValue(const QString &key) const;

    QTabWidget *tabWidget = nullptr;
    QDialogButtonBox *buttonBox = nullptr;

    QList<SettingDef> defs;

    // tab name → tab page widget
    QMap<QString, QWidget*> tabs;

    // key → editor widget (QLineEdit / QSpinBox / QDoubleSpinBox /
    //                       QCheckBox / QComboBox)
    QMap<QString, QWidget*> editors;
};
