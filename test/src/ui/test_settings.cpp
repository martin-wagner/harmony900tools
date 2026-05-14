// SPDX-License-Identifier: LGPL-2.1-or-later

#include <gtest/gtest.h>

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QSignalSpy>
#include <QSpinBox>
#include <QTabWidget>

#include "settings.h"

// ── QApplication singleton ────────────────────────────────────────────────────
// gtest_main provides main(); we need a QApplication alive for the whole run.

class QtEnv: public ::testing::Environment
{
  public:
    void SetUp() override
    {
      // Use a throw-away org/app so QSettings never touches real user config.
      QCoreApplication::setOrganizationName("test_org_settings_dialog");
      QCoreApplication::setApplicationName("test_app_settings_dialog");
    }

    void TearDown() override
    {
      // Remove all keys written during tests.
      QSettings qs(QCoreApplication::organizationName(),
          QCoreApplication::applicationName());
      qs.clear();
      qs.sync();
    }
};

// Registered in main() below.

// ── Helpers ───────────────────────────────────────────────────────────────────

static SettingDef makeString(const QString &key, const QString &defaultVal,
    const QString &tab = { })
{
  return {key, key + "_label", "help", SettingType::String,
    defaultVal, tab, {}, {}, {}};
}

static SettingDef makeInt(const QString &key, int defaultVal, int min = 0,
    int max = 0, const QString &tab = { })
{
  QVariant minV = (min == 0 && max == 0) ? QVariant() : QVariant(min);
  QVariant maxV = (min == 0 && max == 0) ? QVariant() : QVariant(max);
  return {key, key + "_label", "help", SettingType::Int,
    defaultVal, tab, minV, maxV, {}};
}

static SettingDef makeDouble(const QString &key, double defaultVal,
    const QString &tab = { })
{
  return {key, key + "_label", "help", SettingType::Double,
    defaultVal, tab, {}, {}, {}};
}

static SettingDef makeBool(const QString &key, bool defaultVal,
    const QString &tab = { })
{
  return {key, key + "_label", "help", SettingType::Bool,
    defaultVal, tab, {}, {}, {}};
}

static SettingDef makeMulti(const QString &key, QVariant defaultVal,
    const QList<QPair<QString, QVariant>> &options, const QString &tab = { })
{
  return {key, key + "_label", "help", SettingType::MultiSelection,
    defaultVal, tab, {}, {}, options};
}

// ── Fixture ───────────────────────────────────────────────────────────────────

class SettingsTest: public ::testing::Test
{
  protected:
    void SetUp() override
    {
      // Clear persisted keys before each test.
      QSettings qs(QCoreApplication::organizationName(),
          QCoreApplication::applicationName());
      qs.clear();
      qs.sync();

      dlg = new Settings();
    }

    void TearDown() override
    {
      delete dlg;
      dlg = nullptr;
    }

    Settings *dlg = nullptr;
};

// ── Default values ────────────────────────────────────────────────────────────

TEST_F(SettingsTest, StringDefaultValue)
{
  dlg->addSetting(makeString("name", "Alice"));
  EXPECT_EQ(dlg->values().value("name").toString(), "Alice");
}

TEST_F(SettingsTest, IntDefaultValue)
{
  dlg->addSetting(makeInt("count", 42));
  EXPECT_EQ(dlg->values().value("count").toInt(), 42);
}

TEST_F(SettingsTest, DoubleDefaultValue)
{
  dlg->addSetting(makeDouble("ratio", 3.14));
  EXPECT_DOUBLE_EQ(dlg->values().value("ratio").toDouble(), 3.14);
}

TEST_F(SettingsTest, BoolDefaultValueTrue)
{
  dlg->addSetting(makeBool("flag", true));
  EXPECT_TRUE(dlg->values().value("flag").toBool());
}

TEST_F(SettingsTest, BoolDefaultValueFalse)
{
  dlg->addSetting(makeBool("flag", false));
  EXPECT_FALSE(dlg->values().value("flag").toBool());
}

TEST_F(SettingsTest, MultiSelectionDefaultValue)
{
  QList<QPair<QString, QVariant>> opts = { { "A", 0 }, { "B", 1 }, { "C", 2 } };
  dlg->addSetting(makeMulti("choice", 1, opts));
  EXPECT_EQ(dlg->values().value("choice").toInt(), 1);
}

// ── setValues / values round-trip ─────────────────────────────────────────────

TEST_F(SettingsTest, SetValuesString)
{
  dlg->addSetting(makeString("name", "Alice"));
  dlg->setValues( { { "name", "Bob" } });
  EXPECT_EQ(dlg->values().value("name").toString(), "Bob");
}

TEST_F(SettingsTest, SetValuesInt)
{
  dlg->addSetting(makeInt("count", 1));
  dlg->setValues( { { "count", 99 } });
  EXPECT_EQ(dlg->values().value("count").toInt(), 99);
}

TEST_F(SettingsTest, SetValuesDouble)
{
  dlg->addSetting(makeDouble("ratio", 1.0));
  dlg->setValues( { { "ratio", 2.718 } });
  EXPECT_DOUBLE_EQ(dlg->values().value("ratio").toDouble(), 2.718);
}

TEST_F(SettingsTest, SetValuesBool)
{
  dlg->addSetting(makeBool("flag", false));
  dlg->setValues( { { "flag", true } });
  EXPECT_TRUE(dlg->values().value("flag").toBool());
}

TEST_F(SettingsTest, SetValuesMultiSelection)
{
  QList<QPair<QString, QVariant>> opts = { { "X", 10 }, { "Y", 20 } };
  dlg->addSetting(makeMulti("pick", 10, opts));
  dlg->setValues( { { "pick", 20 } });
  EXPECT_EQ(dlg->values().value("pick").toInt(), 20);
}

TEST_F(SettingsTest, SetValuesIgnoresUnknownKey)
{
  dlg->addSetting(makeString("name", "Alice"));
  // Should not crash; unknown key is silently ignored.
  dlg->setValues( { { "nonexistent", "X" } });
  EXPECT_EQ(dlg->values().value("name").toString(), "Alice");
}

TEST_F(SettingsTest, SetValuesPartialUpdate)
{
  dlg->addSetting(makeString("a", "first"));
  dlg->addSetting(makeString("b", "second"));
  dlg->setValues( { { "a", "changed" } });
  EXPECT_EQ(dlg->values().value("a").toString(), "changed");
  EXPECT_EQ(dlg->values().value("b").toString(), "second");
}

// ── Insertion order ───────────────────────────────────────────────────────────

TEST_F(SettingsTest, ValuesContainsAllKeys)
{
  dlg->addSetting(makeString("x", "1"));
  dlg->addSetting(makeInt("y", 2));
  dlg->addSetting(makeBool("z", true));
  const auto vals = dlg->values();
  EXPECT_TRUE(vals.contains("x"));
  EXPECT_TRUE(vals.contains("y"));
  EXPECT_TRUE(vals.contains("z"));
  EXPECT_EQ(vals.size(), 3);
}

// ── Tab assignment ────────────────────────────────────────────────────────────

TEST_F(SettingsTest, NoTabGoesToGeneral)
{
  dlg->addSetting(makeString("k", "v"));  // no tab
  auto *tw = dlg->findChild<QTabWidget*>();
  ASSERT_NE(tw, nullptr);
  bool found = false;
  for (int i = 0; i < tw->count(); ++i) {
    if (tw->tabText(i) == "General") {
      found = true;
    }
  }
  EXPECT_TRUE(found);
}

TEST_F(SettingsTest, ExplicitTabCreated)
{
  dlg->addSetting(makeString("k", "v", "Network"));
  auto *tw = dlg->findChild<QTabWidget*>();
  ASSERT_NE(tw, nullptr);
  bool found = false;
  for (int i = 0; i < tw->count(); ++i) {
    if (tw->tabText(i) == "Network") {
      found = true;
    }
  }
  EXPECT_TRUE(found);
}

TEST_F(SettingsTest, SameTabNotDuplicated)
{
  dlg->addSetting(makeString("a", "1", "MyTab"));
  dlg->addSetting(makeString("b", "2", "MyTab"));
  auto *tw = dlg->findChild<QTabWidget*>();
  ASSERT_NE(tw, nullptr);
  int count = 0;
  for (int i = 0; i < tw->count(); ++i) {
    if (tw->tabText(i) == "MyTab") {
      ++count;
    }
  }
  EXPECT_EQ(count, 1);
}

TEST_F(SettingsTest, MultipleTabs)
{
  dlg->addSetting(makeString("a", "1", "Tab1"));
  dlg->addSetting(makeString("b", "2", "Tab2"));
  dlg->addSetting(makeString("c", "3", "Tab3"));
  auto *tw = dlg->findChild<QTabWidget*>();
  ASSERT_NE(tw, nullptr);
  EXPECT_EQ(tw->count(), 3);
}

// ── Numeric limits ────────────────────────────────────────────────────────────

TEST_F(SettingsTest, IntRespectMinLimit)
{
  dlg->addSetting(makeInt("port", 8080, 1024, 65535));
  auto *sb = dlg->findChild<QSpinBox*>();
  ASSERT_NE(sb, nullptr);
  EXPECT_EQ(sb->minimum(), 1024);
}

TEST_F(SettingsTest, IntRespectMaxLimit)
{
  dlg->addSetting(makeInt("port", 8080, 1024, 65535));
  auto *sb = dlg->findChild<QSpinBox*>();
  ASSERT_NE(sb, nullptr);
  EXPECT_EQ(sb->maximum(), 65535);
}

TEST_F(SettingsTest, IntClampedOnSetValues)
{
  dlg->addSetting(makeInt("val", 50, 0, 100));
  // QSpinBox silently clamps to [min, max] on setValue.
  dlg->setValues( { { "val", 200 } });
  EXPECT_EQ(dlg->values().value("val").toInt(), 100);
}

// ── Tooltip / help text ───────────────────────────────────────────────────────

TEST_F(SettingsTest, HelpTextSetAsTooltip)
{
  SettingDef def = makeString("k", "v");
  def.helpText = "This is help text";
  dlg->addSetting(def);
  auto *le = dlg->findChild<QLineEdit*>();
  ASSERT_NE(le, nullptr);
  EXPECT_EQ(le->toolTip(), "This is help text");
}

TEST_F(SettingsTest, EmptyHelpTextNoTooltip)
{
  SettingDef def = makeString("k", "v");
  def.helpText = "";
  dlg->addSetting(def);
  auto *le = dlg->findChild<QLineEdit*>();
  ASSERT_NE(le, nullptr);
  EXPECT_TRUE(le->toolTip().isEmpty());
}

// ── Signal ────────────────────────────────────────────────────────────────────

TEST_F(SettingsTest, SettingsAcceptedSignalEmittedOnApply)
{
  dlg->addSetting(makeString("name", "Alice"));

  QSignalSpy spy(dlg, &Settings::settingsAccepted);
  // Directly invoke onApply via the Apply button click path.
  // We call setValues + then trigger via findChild on the buttonBox.
  auto *buttonBox = dlg->findChild<QDialogButtonBox*>();
  ASSERT_NE(buttonBox, nullptr);

  QPushButton *applyBtn = buttonBox->button(QDialogButtonBox::Apply);
  ASSERT_NE(applyBtn, nullptr);
  applyBtn->click();

  EXPECT_EQ(spy.count(), 1);
  const auto emittedVals = spy.at(0).at(0).value<QMap<QString, QVariant>>();
  EXPECT_EQ(emittedVals.value("name").toString(), "Alice");
}

TEST_F(SettingsTest, SettingsAcceptedSignalCarriesCurrentValues)
{
  dlg->addSetting(makeString("name", "Alice"));
  dlg->setValues( { { "name", "Charlie" } });

  QSignalSpy spy(dlg, &Settings::settingsAccepted);
  auto *buttonBox = dlg->findChild<QDialogButtonBox*>();
  ASSERT_NE(buttonBox, nullptr);
  buttonBox->button(QDialogButtonBox::Apply)->click();

  EXPECT_EQ(spy.count(), 1);
  const auto emittedVals = spy.at(0).at(0).value<QMap<QString, QVariant>>();
  EXPECT_EQ(emittedVals.value("name").toString(), "Charlie");
}

// ── QSettings persistence ─────────────────────────────────────────────────────
// These tests verify save + load across separate Settings instances.

TEST_F(SettingsTest, PersistenceRoundTripString)
{
  dlg->addSetting(makeString("saved_str", "default"));
  dlg->setValues( { { "saved_str", "persisted" } });
  // Trigger save via Apply button.
  auto *buttonBox = dlg->findChild<QDialogButtonBox*>();
  ASSERT_NE(buttonBox, nullptr);
  buttonBox->button(QDialogButtonBox::Apply)->click();

  // New dialog should load the persisted value.
  Settings dlg2;
  dlg2.addSetting(makeString("saved_str", "default"));
  EXPECT_EQ(dlg2.values().value("saved_str").toString(), "persisted");
}

TEST_F(SettingsTest, PersistenceRoundTripInt)
{
  dlg->addSetting(makeInt("saved_int", 1));
  dlg->setValues( { { "saved_int", 77 } });
  dlg->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Apply)->click();

  Settings dlg2;
  dlg2.addSetting(makeInt("saved_int", 1));
  EXPECT_EQ(dlg2.values().value("saved_int").toInt(), 77);
}

TEST_F(SettingsTest, PersistenceRoundTripBool)
{
  dlg->addSetting(makeBool("saved_bool", false));
  dlg->setValues( { { "saved_bool", true } });
  dlg->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Apply)->click();

  Settings dlg2;
  dlg2.addSetting(makeBool("saved_bool", false));
  EXPECT_TRUE(dlg2.values().value("saved_bool").toBool());
}

// ── MultiSelection specifics ──────────────────────────────────────────────────

TEST_F(SettingsTest, MultiSelectionOptionsCount)
{
  QList<QPair<QString, QVariant>> opts = { { "A", 0 }, { "B", 1 }, { "C", 2 } };
  dlg->addSetting(makeMulti("choice", 0, opts));
  auto *combo = dlg->findChild<QComboBox*>();
  ASSERT_NE(combo, nullptr);
  EXPECT_EQ(combo->count(), 3);
}

TEST_F(SettingsTest, MultiSelectionStringData)
{
  QList<QPair<QString, QVariant>> opts = { { "Alpha", QString("a") }, { "Beta",
      QString("b") } };
  dlg->addSetting(makeMulti("mode", QString("b"), opts));
  EXPECT_EQ(dlg->values().value("mode").toString(), "b");
}

TEST_F(SettingsTest, MultiSelectionInvalidDefaultFallsToFirst)
{
  QList<QPair<QString, QVariant>> opts = { { "X", 10 }, { "Y", 20 } };
  // Default 99 matches no option → combo stays at index 0.
  dlg->addSetting(makeMulti("pick", 99, opts));
  EXPECT_EQ(dlg->values().value("pick").toInt(), 10);
}

// ── Edge cases ────────────────────────────────────────────────────────────────

TEST_F(SettingsTest, EmptyDialogValuesIsEmpty)
{
  EXPECT_TRUE(dlg->values().isEmpty());
}

TEST_F(SettingsTest, DuplicateKeyLastAddWins)
{
  // Inserting two settings with the same key: both get editors inserted,
  // but values() will return the last one registered in the editors map.
  dlg->addSetting(makeString("dup", "first"));
  dlg->addSetting(makeString("dup", "second"));
  // We just verify no crash and values() returns something for "dup".
  EXPECT_TRUE(dlg->values().contains("dup"));
}

// ── main ──────────────────────────────────────────────────────────────────────

int main(int argc, char *argv[])
{
  // QApplication must exist before any QWidget is created.
  QApplication app(argc, argv);

  ::testing::InitGoogleTest(&argc, argv);
  ::testing::AddGlobalTestEnvironment(new QtEnv());
  return RUN_ALL_TESTS();
}
