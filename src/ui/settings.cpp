#include "settings.h"

#include <QTabWidget>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QAbstractButton>
#include <QFrame>
#include <QMessageBox>

#include "lib/settings.h"

static const QString kDefaultTab = QStringLiteral("General");

// ── ctor ──────────────────────────────────────────────────────────────────────

Settings::Settings(QWidget *parent) :
    QDialog(parent), settings(lib::getQSettings())
{
  createView();
  setWindowTitle(tr("Settings"));
  resize(480, 360);
}

// ── public ────────────────────────────────────────────────────────────────────

void Settings::addSetting(const SettingDef &def)
{
  defs.append(def);

  const QString tabName = def.tab.isEmpty() ? kDefaultTab : def.tab;
  QFormLayout *form = layoutForTab(tabName);

  QWidget *editor = nullptr;

  switch (def.type) {
    case SettingType::String: {
      auto *le = new QLineEdit(this);
      le->setPlaceholderText(def.defaultValue.toString());
      le->setText(def.defaultValue.toString());
      if (!def.helpText.isEmpty()) {
        le->setToolTip(def.helpText);
      }
      editor = le;
      break;
    }
    case SettingType::Int: {
      auto *sb = new QSpinBox(this);
      sb->setMinimum(
          def.minValue.isValid() ?
              def.minValue.toInt() : std::numeric_limits<int>::min() / 2);
      sb->setMaximum(
          def.maxValue.isValid() ?
              def.maxValue.toInt() : std::numeric_limits<int>::max() / 2);
      sb->setValue(def.defaultValue.toInt());
      if (!def.helpText.isEmpty()) {
        sb->setToolTip(def.helpText);
      }
      editor = sb;
      break;
    }
    case SettingType::Double: {
      auto *sb = new QDoubleSpinBox(this);
      sb->setDecimals(6);
      sb->setMinimum(def.minValue.isValid() ? def.minValue.toDouble() : -1e15);
      sb->setMaximum(def.maxValue.isValid() ? def.maxValue.toDouble() : 1e15);
      sb->setValue(def.defaultValue.toDouble());
      if (!def.helpText.isEmpty()) {
        sb->setToolTip(def.helpText);
      }
      editor = sb;
      break;
    }
    case SettingType::Bool: {
      auto *cb = new QCheckBox(this);
      cb->setChecked(def.defaultValue.toBool());
      if (!def.helpText.isEmpty()) {
        cb->setToolTip(def.helpText);
      }
      editor = cb;
      break;
    }
    case SettingType::MultiSelection: {
      auto *combo = new QComboBox(this);
      for (const auto &opt : def.options) {
        combo->addItem(opt.first, opt.second);
      }
      // select item matching default value
      for (int i = 0; i < combo->count(); ++i) {
        if (combo->itemData(i) == def.defaultValue) {
          combo->setCurrentIndex(i);
          break;
        }
      }
      if (!def.helpText.isEmpty()) {
        combo->setToolTip(def.helpText);
      }
      editor = combo;
      break;
    }
  }

  if (editor != nullptr) {
    auto *rowLabel = new QLabel(def.label, this);
    if (!def.helpText.isEmpty()) {
      rowLabel->setToolTip(def.helpText);
    }
    form->addRow(rowLabel, editor);
    editors.insert(def.key, editor);

    //load from qsettings
    if (settings.contains(def.key)) {
      QMap<QString, QVariant> persisted;
      persisted.insert(def.key, settings.value(def.key));
      setValues(persisted);
    }
  }
}

QMap<QString, QVariant> Settings::values() const
{
  QMap<QString, QVariant> result;
  for (const auto &def : defs) {
    result.insert(def.key, widgetValue(def.key));
  }
  return result;
}

void Settings::setValues(const QMap<QString, QVariant> &vals)
{
  for (const auto &def : defs) {
    if (!vals.contains(def.key)) {
      continue;
    }
    QWidget *w = editors.value(def.key, nullptr);
    if (w == nullptr) {
      continue;
    }
    const QVariant &v = vals[def.key];

    if (auto *le = qobject_cast<QLineEdit*>(w)) {
      le->setText(v.toString());
    } else if (auto *sb = qobject_cast<QSpinBox*>(w)) {
      sb->setValue(v.toInt());
    } else if (auto *sb = qobject_cast<QDoubleSpinBox*>(w)) {
      sb->setValue(v.toDouble());
    } else if (auto *cb = qobject_cast<QCheckBox*>(w)) {
      cb->setChecked(v.toBool());
    } else if (auto *combo = qobject_cast<QComboBox*>(w)) {
      for (int i = 0; i < combo->count(); ++i) {
        if (combo->itemData(i) == v) {
          combo->setCurrentIndex(i);
          break;
        }
      }
    }
  }
}

// ── private slots ─────────────────────────────────────────────────────────────

void Settings::onApply()
{
  saveToQSettings();
  emit settingsAccepted(values());
}

void Settings::onRestoreDefaults()
{
  QMessageBox::StandardButton reply;
  reply = QMessageBox::question(this, tr("Reset"),
      tr("Reset settings to default?"), QMessageBox::Yes | QMessageBox::No);
  if (reply != QMessageBox::Yes) {
    return;
  }
  for (const auto &def : defs) {
    applyDef(def, true);
  }
}

void Settings::onButtonClicked(QAbstractButton *button)
{
  if (buttonBox == nullptr) {
    return;
  }
  const QDialogButtonBox::StandardButton std = buttonBox->standardButton(
      button);

  if (std == QDialogButtonBox::Ok) {
    onApply();
    accept();
  } else if (std == QDialogButtonBox::Apply) {
    onApply();
  } else if (std == QDialogButtonBox::Cancel) {
    reject();
  }
}

// ── private helpers ───────────────────────────────────────────────────────────

void Settings::createView()
{
  auto *mainLayout = new QVBoxLayout(this);

  tabWidget = new QTabWidget(this);
  mainLayout->addWidget(tabWidget, 1);

  // separator
  auto *line = new QFrame(this);
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  mainLayout->addWidget(line);

  // bottom row: restore defaults  +  ok/cancel/apply
  auto *bottomLayout = new QHBoxLayout();

  auto *restoreBtn = new QPushButton(tr("Restore Defaults"), this);
  connect(restoreBtn, &QPushButton::clicked, this,
      &Settings::onRestoreDefaults);
  bottomLayout->addWidget(restoreBtn);
  bottomLayout->addStretch();

  buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply,
      this);
  connect(buttonBox, &QDialogButtonBox::clicked, this,
      &Settings::onButtonClicked);
  bottomLayout->addWidget(buttonBox);

  mainLayout->addLayout(bottomLayout);
}

QWidget* Settings::getOrCreateTab(const QString &tabName)
{
  if (tabs.contains(tabName)) {
    return tabs[tabName];
  }

  auto *page = new QWidget(this);
  auto *pageLayout = new QVBoxLayout(page);

  auto *form = new QFormLayout();
  form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
  pageLayout->addLayout(form);
  pageLayout->addStretch();

  tabWidget->addTab(page, tabName);
  tabs.insert(tabName, page);
  return page;
}

QFormLayout* Settings::layoutForTab(const QString &tabName)
{
  QWidget *page = getOrCreateTab(tabName);
  // The form is the first item in the page's VBoxLayout.
  auto *vbox = qobject_cast<QVBoxLayout*>(page->layout());
  return qobject_cast<QFormLayout*>(vbox->itemAt(0)->layout());
}

void Settings::applyDef(const SettingDef &def, bool useDefault)
{
  QWidget *w = editors.value(def.key, nullptr);
  if (w == nullptr) {
    return;
  }
  const QVariant &v = def.defaultValue;

  if (auto *le = qobject_cast<QLineEdit*>(w)) {
    le->setText(useDefault ? v.toString() : le->text());
  } else if (auto *sb = qobject_cast<QSpinBox*>(w)) {
    if (useDefault) {
      sb->setValue(v.toInt());
    }
  } else if (auto *sb = qobject_cast<QDoubleSpinBox*>(w)) {
    if (useDefault) {
      sb->setValue(v.toDouble());
    }
  } else if (auto *cb = qobject_cast<QCheckBox*>(w)) {
    if (useDefault) {
      cb->setChecked(v.toBool());
    }
  } else if (auto *combo = qobject_cast<QComboBox*>(w)) {
    if (useDefault) {
      for (int i = 0; i < combo->count(); ++i) {
        if (combo->itemData(i) == v) {
          combo->setCurrentIndex(i);
          break;
        }
      }
    }
  }
}

void Settings::saveToQSettings()
{
  const QMap<QString, QVariant> vals = values();
  for (auto it = vals.constBegin(); it != vals.constEnd(); ++it) {
    settings.setValue(it.key(), it.value());
  }
  settings.sync();
}

void Settings::loadFromQSettings()
{
  QMap<QString, QVariant> vals;
  for (const auto &def : defs) {
    if (settings.contains(def.key)) {
      vals.insert(def.key, settings.value(def.key));
    }
  }
  setValues(vals);
}

QVariant Settings::widgetValue(const QString &key) const
{
  QWidget *w = editors.value(key, nullptr);
  if (w == nullptr) {
    return QVariant();
  }

  if (auto *le = qobject_cast<QLineEdit*>(w)) {
    return le->text();
  } else if (auto *sb = qobject_cast<QSpinBox*>(w)) {
    return sb->value();
  } else if (auto *sb = qobject_cast<QDoubleSpinBox*>(w)) {
    return sb->value();
  } else if (auto *cb = qobject_cast<QCheckBox*>(w)) {
    return cb->isChecked();
  } else if (auto *combo = qobject_cast<QComboBox*>(w)) {
    return combo->currentData();
  }

  return QVariant();
}
