#include "SettingsManager.h"
#include <QMessageBox>
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
                          "An unrecognised value for 'BChart Sort Order':" + QString{value} + " was provided");
    std::abort();
  }
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

} // namespace netsimulyzer
