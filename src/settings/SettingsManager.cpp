#include "SettingsManager.h"
#include "src/conversion.h"
#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <iostream>

namespace netsimulyzer {

SettingsManager::BuildingRenderMode SettingsManager::BuildingRenderModeFromInt(int value) {
  switch (value) {
  case static_cast<int>(SettingsManager::BuildingRenderMode::Transparent):
    return SettingsManager::BuildingRenderMode::Transparent;
  case static_cast<int>(SettingsManager::BuildingRenderMode::Opaque):
    return SettingsManager::BuildingRenderMode::Opaque;
  default:
    QMessageBox::critical(nullptr, "Invalid value provided for 'Building Render Mode'!",
                          "An unrecognised value for 'Building Render Mode':" + QString{value} + " was provided");
    std::abort();
  }
}

SettingsManager::LabelRenderMode SettingsManager::LabelRenderModeFromInt(int value) {
  switch (value) {
  case static_cast<int>(LabelRenderMode::Always):
    return LabelRenderMode::Always;
  case static_cast<int>(LabelRenderMode::EnabledOnly):
    return LabelRenderMode::EnabledOnly;
  case static_cast<int>(LabelRenderMode::Never):
    return LabelRenderMode::Never;
  default:
    QMessageBox::critical(nullptr, "Invalid value provided for 'Label Render Mode'!",
                          "An unrecognised value for 'Label Render Mode':" + QString{value} + " was provided");
    std::abort();
  }
}

SettingsManager::ChartDropdownSortOrder SettingsManager::ChartDropdownSortOrderFromInt(int value) {
  using SortOrder = SettingsManager::ChartDropdownSortOrder;

  switch (value) {
  case static_cast<int>(SortOrder::Alphabetical):
    return SortOrder::Alphabetical;
  case static_cast<int>(SortOrder::Type):
    return SortOrder::Type;
  case static_cast<int>(SortOrder::Id):
    return SortOrder::Id;
  case static_cast<int>(SortOrder::None):
    return SortOrder::None;
  default:
    QMessageBox::critical(nullptr, "Invalid value provided for 'Chart Sort Order'!",
                          "An unrecognised value for 'Chart Sort Order':" + QString{value} + " was provided");
    std::abort();
  }
}

SettingsManager::MotionTrailRenderMode SettingsManager::MotionTrailRenderModeFromInt(int value) {
  using MotionTrailRenderMode = SettingsManager::MotionTrailRenderMode;

  switch (value) {
  case static_cast<int>(MotionTrailRenderMode::Always):
    return MotionTrailRenderMode::Always;
  case static_cast<int>(MotionTrailRenderMode::EnabledOnly):
    return MotionTrailRenderMode::EnabledOnly;
  case static_cast<int>(MotionTrailRenderMode::Never):
    return MotionTrailRenderMode::Never;
  default:
    QMessageBox::critical(nullptr, "Invalid value provided for 'Motion Trail Render Mode'!",
                          "An unrecognised value for 'Motion Trail Render Mode':" + QString{value} + " was provided");
    std::abort();
  }
}

SettingsManager::TimeUnit SettingsManager::TimeUnitFromInt(int value) {
  switch (value) {
  case static_cast<int>(SettingsManager::TimeUnit::Nanoseconds):
    return SettingsManager::TimeUnit::Nanoseconds;
  case static_cast<int>(SettingsManager::TimeUnit::Microseconds):
    return SettingsManager::TimeUnit::Microseconds;
  case static_cast<int>(SettingsManager::TimeUnit::Milliseconds):
    return SettingsManager::TimeUnit::Milliseconds;
  default:
    QMessageBox::critical(nullptr, "Invalid value provided for 'Time Unit'!",
                          "An unrecognised value for 'Time Unit': " + QString::number(value) + " was provided");
    QApplication::exit(1);
    break;
  }

  // Should never happen, but just in case
  return SettingsManager::TimeUnit::Nanoseconds;
}

SettingsManager::WindowTheme SettingsManager::WindowThemeFromInt(int value) {
  using WindowTheme = SettingsManager::WindowTheme;

  switch (value) {
  case static_cast<int>(WindowTheme::Dark):
    return WindowTheme::Dark;
  case static_cast<int>(WindowTheme::Light):
    return WindowTheme::Light;
  case static_cast<int>(WindowTheme::Native):
    return WindowTheme::Native;
  default:
    QMessageBox::critical(nullptr, "Invalid value provided for 'WindowTheme'!",
                          "An unrecognised value for 'WindowTheme': " + QString::number(value) + " was provided");
    QApplication::exit(1);
    break;
  }

  // Should never happen, but just in case
  return WindowTheme::Dark;
}

const SettingsManager::SettingValue &SettingsManager::getQtKey(SettingsManager::Key key) const {
  auto iterator = SettingsManager::qtKeyMap.find(key);
  if (iterator == SettingsManager::qtKeyMap.end()) {
    std::cerr << "No Qt Key defined for enum value: " << static_cast<int>(key) << '\n';
    std::abort();
  }

  return iterator->second;
}

bool SettingsManager::isDefined(SettingsManager::Key key) const {
  return qtSettings.contains(getQtKey(key).key);
}

void SettingsManager::setDefault(SettingsManager::Key key) {
  const auto &settingKey = getQtKey(key);

  if (!settingKey.defaultValue.isValid()) {
    std::cerr << "Attempting to default key: " << settingKey.key.toStdString() << " which has no default!\n";
    std::abort();
  }

  qtSettings.setValue(settingKey.key, settingKey.defaultValue);
}

void SettingsManager::clear(SettingsManager::Key key) {
  qtSettings.remove(getQtKey(key).key);
}

void SettingsManager::sync() {
  qtSettings.sync();
}

void SettingsManager::setTheme() {
  setTheme(get<SettingsManager::WindowTheme>(Key::WindowTheme).value());
}

void SettingsManager::setTheme(SettingsManager::WindowTheme theme) {
  const auto application = static_cast<QApplication *>(QCoreApplication::instance());

  switch (theme) {
  case WindowTheme::Dark: {
    QFile themeQss{":qdarkstyle/dark/darkstyle.qss"};
    themeQss.open(QFile::ReadOnly | QFile::Text);
    application->setStyleSheet(QTextStream{&themeQss}.readAll());
    break;
  }
  case WindowTheme::Light: {
    QFile themeQss{":qdarkstyle/light/lightstyle.qss"};
    themeQss.open(QFile::ReadOnly | QFile::Text);
    application->setStyleSheet(QTextStream{&themeQss}.readAll());
    break;
  }
  case WindowTheme::Native:
    application->setStyleSheet("");
    return;
  }
}

} // namespace netsimulyzer
