#include "SettingsManager.h"
#include <iostream>

namespace visualization {

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

} // namespace visualization
