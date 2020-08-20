#pragma once

#include <QSettings>
#include <QString>
#include <Qt>
#include <iostream>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace visualization {

/**
 * Manager that wraps the QSettings Class
 */
class SettingsManager {
public:
  /**
   * Configurable Values, potentially defined with defaults
   */
  enum class Key {
    SettingsVersion,
    ResourcePath,
    MoveSpeed,
    KeyboardTurnSpeed,
    MouseTurnSpeed,
    FieldOfView,
    AllowCameraEvents,
    CameraMouseControls,
    CameraKeyForward,
    CameraKeyBackwards,
    CameraKeyLeft,
    CameraKeyRight,
    CameraKeyLeftTurn,
    CameraKeyRightTurn,
    CameraKeyUp,
    CameraKeyDown,
    MainWindowState,
    NumberSamples
  };

  /**
   * Defines when retrieving a value fails,
   * should it's default value be provided in the return
   * instead of an empty std::optional
   */
  enum class RetrieveMode { AllowDefault, DisallowDefault };

private:
  struct SettingValue {
    QString key;
    QVariant defaultValue;
  };

  /**
   * Map of the keys to the QSetting key & potentially a default value
   *
   * If a key has no reasonable default,
   * provide an invalid QVariant as the default
   * (i.e. `{}`)
   */
  const static inline std::unordered_map<SettingsManager::Key, SettingValue> qtKeyMap{
      {Key::SettingsVersion, {"application/version", {}}},
      {Key::ResourcePath, {"resources/resourcePath", {}}},
      {Key::MoveSpeed, {"camera/moveSpeed", 0.05f}},
      {Key::KeyboardTurnSpeed, {"camera/keyboardTurnSpeed", 0.1f}},
      {Key::MouseTurnSpeed, {"camera/mouseTurnSpeed", 0.5f}},
      {Key::FieldOfView, {"camera/fieldOfView", 45.0f}},
      {Key::AllowCameraEvents, {"camera/allowEvents", true}},
      {Key::CameraMouseControls, {"camera/mouseControls", true}},
      {Key::CameraKeyForward, {"camera/keyForward", Qt::Key_W}},
      {Key::CameraKeyBackwards, {"camera/keyBackward", Qt::Key_S}},
      {Key::CameraKeyLeft, {"camera/keyLeft", Qt::Key_A}},
      {Key::CameraKeyRight, {"camera/keyRight", Qt::Key_D}},
      {Key::CameraKeyLeftTurn, {"camera/keyLeftTurn", Qt::Key_Q}},
      {Key::CameraKeyRightTurn, {"camera/keyRightTurn", Qt::Key_E}},
      {Key::CameraKeyUp, {"camera/keyUp", Qt::Key_Z}},
      {Key::CameraKeyDown, {"camera/keyDown", Qt::Key_X}},
      {Key::MainWindowState, {"mainWindow/state", {}}},
      {Key::NumberSamples, {"renderer/numberSamples", 2}}};

  /**
   * Get the Qt key for the corresponding enum value.
   * If one is not defined, the program will abort.
   *
   * @param key
   * The enum value to pull the QSetting key for
   *
   * @return
   * The Qt Key, with a potential default.
   * Check if the `defaultValue` is valid
   * before accessing
   */
  [[nodiscard]] const SettingsManager::SettingValue &getQtKey(SettingsManager::Key key) const;

  /**
   * The Qt Settings backing store
   */
  QSettings qtSettings;

public:
  /**
   * Checks if a key was previously defined in the settings file.
   *
   * @param key
   * The Key to check for in the settings file
   *
   * @return
   * True if the key is in the backing store, False otherwise
   */
  [[nodiscard]] bool isDefined(SettingsManager::Key key) const;

  /**
   * Retrieve a value, or potentially its default from the settings file
   *
   * @tparam T
   * The Type the value should be.
   * If the setting cannot be converted to this type,
   * the default or an empty `std::optional`will be returned, based on `mode`
   *
   * @param key
   * The key the value was set with
   *
   * @param mode
   * If the value does not exist in the backing store,
   * or the value cannot be converted to `T` then
   * Should the default be provided (given one exists).
   *
   * If `mode` is `AllowDefault` then the default may
   * be subtitled for an invalid value.
   *
   * If `mode` is `DisallowDefault`,
   * then an empty `std::optional`
   * will be returned instead
   *
   * @return
   * A `std::optional` with the stored value
   * if the Key was found & can be converted to `T`,
   * Otherwise the default, or an empty `std::optional`
   * depending on `mode`
   */
  template <class T>
  [[nodiscard]] std::optional<T> get(Key key, RetrieveMode mode = RetrieveMode::DisallowDefault) const {
    const auto &settingKey = getQtKey(key);
    const auto qtSetting = qtSettings.value(settingKey.key, settingKey.defaultValue);

    if (qtSetting.isValid() && qtSetting.template canConvert<T>())
      return {qtSetting.template value<T>()};
    else if (mode == RetrieveMode::AllowDefault && settingKey.defaultValue.isValid() &&
             settingKey.defaultValue.template canConvert<T>())
      return {settingKey.defaultValue.template value<T>()};

    return {};
  }

  /**
   * Save a setting in the settings file
   *
   * @tparam T
   * The type of value to store.
   * Should be a type recognised by
   * `QVariant`, or `std::string`
   *
   * @param key
   * The key to store `value` under.
   * Will overwrite any previous values
   *
   * @param value
   * The new value to store
   */
  template <class T>
  void set(SettingsManager::Key key, const T &value) {
    const auto &settingKey = getQtKey(key);
    qtSettings.setValue(settingKey.key, value);
  }

  /**
   * Set a key to it's default value defined in `qtKeyMap`
   *
   * @param key
   * The key to set to it's default.
   * If the key has no default, the
   * program will abort
   */
  void setDefault(SettingsManager::Key key);

  /**
   * Retrieves the default value for a key.
   * If `key` has no default, the program aborts
   *
   * @tparam T
   * The type to convert the value to.
   * Should be a recognised type to `QVariant`
   * or `std::string`
   *
   * @param key
   * The key to pull the default of
   *
   * @return
   * The default value of `key` converted to `T`
   */
  template <class T>
  [[nodiscard]] T getDefault(SettingsManager::Key key) {
    const auto &settingKey = getQtKey(key);
    if (!settingKey.defaultValue.isValid()) {
      std::cerr << "Requested default for key: " << settingKey.key.toStdString() << " which has no default\n";
      std::abort();
    }

    return settingKey.defaultValue.template value<T>();
  }

  /**
   * Clears `key` from the settings.
   *
   * If you mean to set a setting back to its default, see `setDefault()`
   *
   * @param key
   * The key for the setting to clear
   */
  void clear(SettingsManager::Key key);

  /**
   * Sync settings to storage
   */
  void sync();
};

// Specialize so we don't have to convert to/from QString all the time
// Specified after the class because GCC is broken
template <>
[[nodiscard]] inline std::optional<std::string> SettingsManager::get(Key key, RetrieveMode mode) const {
  const auto &settingKey = getQtKey(key);
  const auto qtSetting = qtSettings.value(settingKey.key, settingKey.defaultValue);

  if (qtSetting.isValid() && qtSetting.template canConvert<QString>())
    return {qtSetting.toString().toStdString()};
  else if (mode == RetrieveMode::AllowDefault && settingKey.defaultValue.isValid() &&
           settingKey.defaultValue.canConvert<QString>())
    return {settingKey.defaultValue.toString().toStdString()};

  return {};
}

template <>
inline void SettingsManager::set(SettingsManager::Key key, const std::string &value) {
  const auto &settingKey = getQtKey(key);
  qtSettings.setValue(settingKey.key, QString::fromStdString(value));
}

template <>
[[nodiscard]] inline std::string SettingsManager::getDefault(SettingsManager::Key key) {
  const auto &settingKey = getQtKey(key);
  if (!settingKey.defaultValue.isValid()) {
    std::cerr << "Requested default for key: " << settingKey.key.toStdString() << " which has no default\n";
    std::abort();
  }

  return settingKey.defaultValue.toString().toStdString();
}

} // namespace visualization
