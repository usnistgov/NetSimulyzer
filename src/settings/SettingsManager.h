#pragma once

#include "parser/model.h"
#include "src/util/palette.h"
#include <QColor>
#include <QSettings>
#include <QString>
#include <Qt>
#include <iostream>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace netsimulyzer {

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
    LastLoadPath,
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
    SceneKeyPlay,
    MainWindowState,
    NumberSamples,
    PlaybackTimeStepPreference,
    PlaybackTimeStepUnit,
    RenderBuildingMode,
    RenderBuildingOutlines,
    RenderGrid,
    RenderGridStep,
    RenderLabelScale,
    RenderMotionTrails,
    RenderMotionTrailLength,
    RenderLabels,
    RenderSkybox,
    RenderFloor,
    RenderBackgroundColor,
    RenderBackgroundColorCustom,
    ChartDropdownSortOrder,
    WindowTheme
  };

  enum class BuildingRenderMode : int { Transparent, Opaque };
  enum class LabelRenderMode : int { Always, EnabledOnly, Never };
  enum class ChartDropdownSortOrder : int { Alphabetical, Type, Id, None };
  enum class MotionTrailRenderMode : int { Always, EnabledOnly, Never };
  enum class TimeUnit : int { Milliseconds, Microseconds, Nanoseconds };
  enum class WindowTheme : int { Dark, Light, Native };
  enum class BackgroundColor : int { Black, White, Custom };

  /**
   * Convert an int to a `BuildingRenderMode` enum value.
   * Necessary since Qt will only allow sending registered types with signals/slots.
   *
   * @param value
   * An integer that corresponds to an enum value
   *
   * @return
   * The enum value corresponding to `value`
   */
  static BuildingRenderMode BuildingRenderModeFromInt(int value);

  /**
   * Convert an int to a `LabelRenderMode` enum value.
   * Necessary since Qt will only allow sending registered types with signals/slots.
   *
   * @param value
   * An integer that corresponds to an enum value
   *
   * @return
   * The enum value corresponding to `value`
   */
  static LabelRenderMode LabelRenderModeFromInt(int value);

  /**
   * Convert an int to a `ChartDropdownSortOrder` enum value.
   * Necessary since Qt will only allow sending registered types with signals/slots.
   *
   * @param value
   * An integer that corresponds to an enum value
   *
   * @return
   * The enum value corresponding to `value`
   */
  static ChartDropdownSortOrder ChartDropdownSortOrderFromInt(int value);

  /**
   * Convert an int to a `MotionTrailRenderMode` enum value.
   * Necessary since Qt will only allow sending registered types with signals/slots.
   *
   * @param value
   * An integer that corresponds to an enum value
   *
   * @return
   * The enum value corresponding to `value`
   */
  static MotionTrailRenderMode MotionTrailRenderModeFromInt(int value);

  /**
   * Convert an int to a `TimeUnit` enum value.
   * Necessary since Qt will only allow sending registered types with signals/slots.
   *
   * @param value
   * An integer that corresponds to an enum value
   *
   * @return
   * The enum value corresponding to `value`
   */
  static TimeUnit TimeUnitFromInt(int value);

  /**
   * Convert an int to a `WindowTheme` enum value.
   * Necessary since Qt will only allow sending registered types with signals/slots.
   *
   * @param value
   * An integer that corresponds to an enum value
   *
   * @return
   * The enum value corresponding to `value`
   */
  static WindowTheme WindowThemeFromInt(int value);

  /**
   * Convert an int to a `BackgroundColor` enum value.
   * Necessary since Qt will only allow sending registered types with signals/slots.
   *
   * @param value
   * An integer that corresponds to an enum value
   *
   * @return
   * The enum value corresponding to `value`
   */
  static BackgroundColor BackgroundColorFromInt(int value);

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
      {Key::LastLoadPath, {"application/lastLoadPath", {}}},
      {Key::ResourcePath, {"resources/resourcePath", {}}},
      {Key::MoveSpeed, {"camera/moveSpeed", 0.02f}},
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
      {Key::SceneKeyPlay, {"scene/keyPlay", Qt::Key_P}},
      {Key::MainWindowState, {"mainWindow/state", {}}},
      {Key::PlaybackTimeStepPreference, {"playback/timeStepPreference", 10'000'000LL}}, // 10ms in nanoseconds
      {Key::PlaybackTimeStepUnit, {"playback/timeStepUnit", "milliseconds"}},
      {Key::NumberSamples, {"renderer/numberSamples", 2}},
      {Key::RenderBuildingMode, {"renderer/buildingRenderMode", "transparent"}},
      {Key::RenderBuildingOutlines, {"renderer/showBuildingOutlines", true}},
      {Key::RenderLabelScale, {"renderer/labelScale", 0.1f}},
      {Key::RenderGrid, {"renderer/showGrid", true}},
      {Key::RenderGridStep, {"renderer/gridStepSize", 1}},
      {Key::RenderSkybox, {"renderer/enableSkybox", true}},
      {Key::RenderFloor, {"renderer/enableFloor", true}},
      {Key::RenderBackgroundColor, {"renderer/backgroundColor", "black"}},
      {Key::RenderBackgroundColorCustom, {"renderer/backgroundColorCustom", palette::Black}},
      {Key::RenderLabels, {"renderer/showLabels", "enabledOnly"}},
      {Key::RenderMotionTrails, {"renderer/showMotionTrails", "enabledOnly"}},
      {Key::RenderMotionTrailLength, {"renderer/motionTrailLength", 100}},
      {Key::ChartDropdownSortOrder, {"chart/dropdownSortOrder", "type"}},
      {Key::WindowTheme, {"window/theme", "dark"}}};

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
  [[nodiscard]] std::optional<T> get(Key key, RetrieveMode mode = RetrieveMode::AllowDefault) const {
    const auto &settingKey = getQtKey(key);
    const auto qtSetting = qtSettings.value(settingKey.key, settingKey.defaultValue);

    if (qtSetting.isValid() && !qtSetting.isNull() && qtSetting.template canConvert<T>())
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
  [[nodiscard]] T getDefault(SettingsManager::Key key) const {
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

  /**
   * Sets the application wide theme to the current setting.
   */
  void setTheme();

  /**
   * Sets the application wide theme.
   * This does not persist the selected theme as a setting
   *
   * @param theme
   * The theme to load into the application.
   */
  void setTheme(WindowTheme theme);

  [[nodiscard]]
  QColor getRenderBackgroundColor() const;
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
[[nodiscard]] inline std::string SettingsManager::getDefault(SettingsManager::Key key) const {
  const auto &settingKey = getQtKey(key);
  if (!settingKey.defaultValue.isValid()) {
    std::cerr << "Requested default for key: " << settingKey.key.toStdString() << " which has no default\n";
    std::abort();
  }

  return settingKey.defaultValue.toString().toStdString();
}

// Specialization for SceneWidget::BuildingRenderMode enum
// so each widget does not need to convert to/from the settings representation
template <>
[[nodiscard]] inline SettingsManager::BuildingRenderMode SettingsManager::getDefault(SettingsManager::Key key) const {
  const auto &settingKey = getQtKey(key);
  if (!settingKey.defaultValue.isValid()) {
    std::cerr << "Requested default for key: " << settingKey.key.toStdString() << " which has no default\n";
    std::abort();
  }

  // TODO: Use the map value
  return SettingsManager::BuildingRenderMode::Transparent;
}

// Specialization for LabelRenderMode enum
// so each widget does not need to convert to/from the settings representation
template <>
[[nodiscard]] inline SettingsManager::LabelRenderMode SettingsManager::getDefault(SettingsManager::Key) const {
  // TODO: Use the map value
  return SettingsManager::LabelRenderMode::EnabledOnly;
}

// Specialization for ChartManager::SortOrder enum
// so each widget does not need to convert to/from the settings representation
template <>
[[nodiscard]] inline SettingsManager::ChartDropdownSortOrder
SettingsManager::getDefault(SettingsManager::Key key) const {
  const auto &settingKey = getQtKey(key);
  if (!settingKey.defaultValue.isValid()) {
    std::cerr << "Requested default for key: " << settingKey.key.toStdString() << " which has no default\n";
    std::abort();
  }

  // TODO: Use the map value
  return SettingsManager::ChartDropdownSortOrder::Type;
}

// Specialization for MotionTrailRenderMode enum
// so each widget does not need to convert to/from the settings representation
template <>
[[nodiscard]] inline SettingsManager::MotionTrailRenderMode SettingsManager::getDefault(SettingsManager::Key) const {
  // TODO: Use the map value
  return SettingsManager::MotionTrailRenderMode::EnabledOnly;
}

// Specialization for TimeUnit enum
// so each widget does not need to convert to/from the settings representation
template <>
[[nodiscard]] inline SettingsManager::TimeUnit SettingsManager::getDefault(SettingsManager::Key key) const {
  const auto &settingKey = getQtKey(key);
  if (!settingKey.defaultValue.isValid()) {
    std::cerr << "Requested default for key: " << settingKey.key.toStdString() << " which has no default\n";
    std::abort();
  }

  // TODO: Use the map value
  return SettingsManager::TimeUnit::Milliseconds;
}

template <>
[[nodiscard]] inline SettingsManager::WindowTheme SettingsManager::getDefault(SettingsManager::Key) const {
  // TODO: Use the map value
  return SettingsManager::WindowTheme::Dark;
}

template <>
[[nodiscard]] inline SettingsManager::BackgroundColor SettingsManager::getDefault(SettingsManager::Key key) const {
  const auto &settingKey = getQtKey(key);
  if (!settingKey.defaultValue.isValid()) {
    std::cerr << "Requested default for key: " << settingKey.key.toStdString() << " which has no default\n";
    std::abort();
  }

  // TODO: Use the map value
  return SettingsManager::BackgroundColor::Black;
}

template <>
[[nodiscard]] inline std::optional<SettingsManager::BuildingRenderMode> SettingsManager::get(Key key,
                                                                                             RetrieveMode mode) const {
  const auto &settingKey = getQtKey(key);
  const auto qtSetting = qtSettings.value(settingKey.key);

  QString stringMode;

  if (qtSetting.isValid() && !qtSetting.isNull() && qtSetting.template canConvert<QString>())
    stringMode = qtSetting.toString();
  else if (mode == RetrieveMode::AllowDefault)
    stringMode = settingKey.defaultValue.toString();
  else
    return {};

  if (stringMode == "transparent")
    return {SettingsManager::BuildingRenderMode::Transparent};
  else if (stringMode == "opaque")
    return {SettingsManager::BuildingRenderMode::Opaque};
  else
    std::cerr << "Unrecognised 'BuildingRenderMode' provided '" << stringMode.toStdString() << "' value ignored!\n";

  // Final catch if the provided string value is invalid
  if (mode == RetrieveMode::AllowDefault)
    return getDefault<BuildingRenderMode>(Key::RenderBuildingMode);

  return {};
}

template <>
[[nodiscard]]
inline std::optional<SettingsManager::LabelRenderMode> SettingsManager::get(Key key, RetrieveMode mode) const {
  const auto &settingKey = getQtKey(key);
  const auto qtSetting = qtSettings.value(settingKey.key);

  QString stringMode;

  if (qtSetting.isValid() && !qtSetting.isNull() && qtSetting.template canConvert<QString>())
    stringMode = qtSetting.toString();
  else if (mode == RetrieveMode::AllowDefault)
    stringMode = settingKey.defaultValue.toString();
  else
    return {};

  if (stringMode == "always")
    return {SettingsManager::LabelRenderMode::Always};
  else if (stringMode == "enabledOnly")
    return {SettingsManager::LabelRenderMode::EnabledOnly};
  else if (stringMode == "never")
    return {SettingsManager::LabelRenderMode::Never};
  else
    std::cerr << "Unrecognised 'LabelRenderMode '" << stringMode.toStdString() << "' value ignored!\n";

  // Final catch if the provided string value is invalid
  if (mode == RetrieveMode::AllowDefault)
    return getDefault<SettingsManager::LabelRenderMode>(Key::RenderLabels);

  return {};
}

template <>
[[nodiscard]] inline std::optional<SettingsManager::MotionTrailRenderMode>
SettingsManager::get(Key key, RetrieveMode mode) const {
  const auto &settingKey = getQtKey(key);
  const auto qtSetting = qtSettings.value(settingKey.key);

  QString stringMode;

  if (qtSetting.isValid() && !qtSetting.isNull() && qtSetting.template canConvert<QString>())
    stringMode = qtSetting.toString();
  else if (mode == RetrieveMode::AllowDefault)
    stringMode = settingKey.defaultValue.toString();
  else
    return {};

  if (stringMode == "always")
    return {SettingsManager::MotionTrailRenderMode::Always};
  else if (stringMode == "enabledOnly")
    return {SettingsManager::MotionTrailRenderMode::EnabledOnly};
  else if (stringMode == "never")
    return {SettingsManager::MotionTrailRenderMode::Never};
  else
    std::cerr << "Unrecognised 'MotionTrailRenderMode '" << stringMode.toStdString() << "' value ignored!\n";

  // Final catch if the provided string value is invalid
  if (mode == RetrieveMode::AllowDefault)
    return getDefault<SettingsManager::MotionTrailRenderMode>(Key::RenderMotionTrails);

  return {};
}

template <>
[[nodiscard]] inline std::optional<SettingsManager::TimeUnit> SettingsManager::get(Key key, RetrieveMode mode) const {
  const auto &settingKey = getQtKey(key);
  const auto qtSetting = qtSettings.value(settingKey.key);

  QString stringMode;
  if (qtSetting.isValid() && !qtSetting.isNull() && qtSetting.template canConvert<QString>())
    stringMode = qtSetting.toString();
  else if (mode == RetrieveMode::AllowDefault)
    stringMode = settingKey.defaultValue.toString();
  else // Handle unset/empty string values
    return {};

  if (stringMode == "milliseconds")
    return {SettingsManager::TimeUnit::Milliseconds};
  else if (stringMode == "microseconds")
    return {SettingsManager::TimeUnit::Microseconds};
  else if (stringMode == "nanoseconds")
    return {SettingsManager::TimeUnit::Nanoseconds};
  else
    std::cerr << "Unrecognised 'TimeUnit' provided '" << stringMode.toStdString() << "' value ignored!\n";

  // Final catch if the provided string value is invalid
  if (mode == RetrieveMode::AllowDefault)
    return getDefault<SettingsManager::TimeUnit>(Key::PlaybackTimeStepUnit);

  return {};
}

template <>
[[nodiscard]] inline std::optional<SettingsManager::ChartDropdownSortOrder>
SettingsManager::get(Key key, RetrieveMode mode) const {
  const auto &settingKey = getQtKey(key);
  const auto qtSetting = qtSettings.value(settingKey.key);

  QString stringMode;

  if (qtSetting.isValid() && !qtSetting.isNull() && qtSetting.template canConvert<QString>())
    stringMode = qtSetting.toString();
  else if (mode == RetrieveMode::AllowDefault)
    stringMode = settingKey.defaultValue.toString();
  else
    return {};

  if (stringMode == "alphabetical")
    return {SettingsManager::ChartDropdownSortOrder::Alphabetical};
  else if (stringMode == "type")
    return {SettingsManager::ChartDropdownSortOrder::Type};
  else if (stringMode == "id")
    return {SettingsManager::ChartDropdownSortOrder::Id};
  else if (stringMode == "none")
    return {SettingsManager::ChartDropdownSortOrder::None};
  else
    std::cerr << "Unrecognised 'ChartDropdownSortOrder' provided '" << stringMode.toStdString() << "' value ignored!\n";

  // Final catch if the provided string value is invalid
  if (mode == RetrieveMode::AllowDefault)
    return getDefault<ChartDropdownSortOrder>(Key::ChartDropdownSortOrder);

  return {};
}

template <>
[[nodiscard]]
inline std::optional<SettingsManager::WindowTheme> SettingsManager::get(Key key, RetrieveMode mode) const {
  const auto &settingKey = getQtKey(key);
  const auto qtSetting = qtSettings.value(settingKey.key);

  QString stringMode;

  if (qtSetting.isValid() && !qtSetting.isNull() && qtSetting.template canConvert<QString>())
    stringMode = qtSetting.toString();
  else if (mode == RetrieveMode::AllowDefault)
    stringMode = settingKey.defaultValue.toString();
  else
    return {};

  if (stringMode == "dark")
    return {WindowTheme::Dark};
  else if (stringMode == "light")
    return {WindowTheme::Light};
  else if (stringMode == "native")
    return {WindowTheme::Native};
  else
    std::cerr << "Unrecognised 'WindowTheme' provided '" << stringMode.toStdString() << "' value ignored!\n";

  // Final catch if the provided string value is invalid
  if (mode == RetrieveMode::AllowDefault)
    return getDefault<WindowTheme>(Key::WindowTheme);

  return {};
}

template <>
[[nodiscard]] inline std::optional<SettingsManager::BackgroundColor> SettingsManager::get(Key key,
                                                                                          RetrieveMode mode) const {
  const auto &settingKey = getQtKey(key);
  const auto qtSetting = qtSettings.value(settingKey.key);

  QString stringMode;

  if (qtSetting.isValid() && !qtSetting.isNull() && qtSetting.template canConvert<QString>())
    stringMode = qtSetting.toString();
  else if (mode == RetrieveMode::AllowDefault)
    stringMode = settingKey.defaultValue.toString();
  else
    return {};

  if (stringMode == "black")
    return {BackgroundColor::Black};
  else if (stringMode == "white")
    return {BackgroundColor::White};
  else if (stringMode == "custom")
    return {BackgroundColor::Custom};
  else
    std::cerr << "Unrecognised 'BackgroundColor' provided '" << stringMode.toStdString() << "' value ignored!\n";

  // Final catch if the provided string value is invalid
  if (mode == RetrieveMode::AllowDefault)
    return getDefault<BackgroundColor>(Key::RenderBackgroundColor);

  return {};
}

template <>
inline void SettingsManager::set(SettingsManager::Key key, const SettingsManager::BuildingRenderMode &value) {
  const auto &settingKey = getQtKey(key);

  switch (value) {
  case SettingsManager::BuildingRenderMode::Transparent:
    qtSettings.setValue(settingKey.key, "transparent");
    break;
  case SettingsManager::BuildingRenderMode::Opaque:
    qtSettings.setValue(settingKey.key, "opaque");
    break;
  default:
    std::cerr << "Unrecognised 'BuildingRenderMode': " << static_cast<int>(value) << " value not saved!\n";
  }
}

template <>
inline void SettingsManager::set(SettingsManager::Key key, const SettingsManager::LabelRenderMode &value) {
  const auto &settingKey = getQtKey(key);

  switch (value) {
  case SettingsManager::LabelRenderMode::Always:
    qtSettings.setValue(settingKey.key, "always");
    break;
  case SettingsManager::LabelRenderMode::EnabledOnly:
    qtSettings.setValue(settingKey.key, "enabledOnly");
    break;
  case SettingsManager::LabelRenderMode::Never:
    qtSettings.setValue(settingKey.key, "never");
    break;
  default:
    std::cerr << "Unrecognised 'LabelRenderMode': " << static_cast<int>(value) << " value not saved!\n";
  }
}

template <>
inline void SettingsManager::set(SettingsManager::Key key, const SettingsManager::ChartDropdownSortOrder &value) {
  const auto &settingKey = getQtKey(key);

  switch (value) {
  case SettingsManager::ChartDropdownSortOrder::Alphabetical:
    qtSettings.setValue(settingKey.key, "alphabetical");
    break;
  case SettingsManager::ChartDropdownSortOrder::Type:
    qtSettings.setValue(settingKey.key, "type");
    break;
  case SettingsManager::ChartDropdownSortOrder::Id:
    qtSettings.setValue(settingKey.key, "id");
    break;
  case SettingsManager::ChartDropdownSortOrder::None:
    qtSettings.setValue(settingKey.key, "none");
    break;
  default:
    std::cerr << "Unrecognised 'ChartDropdownSortOrder': " << static_cast<int>(value) << " value not saved!\n";
  }
}

template <>
inline void SettingsManager::set(SettingsManager::Key key, const SettingsManager::MotionTrailRenderMode &value) {
  const auto &settingKey = getQtKey(key);

  switch (value) {
  case SettingsManager::MotionTrailRenderMode::Always:
    qtSettings.setValue(settingKey.key, "always");
    break;
  case SettingsManager::MotionTrailRenderMode::EnabledOnly:
    qtSettings.setValue(settingKey.key, "enabledOnly");
    break;
  case SettingsManager::MotionTrailRenderMode::Never:
    qtSettings.setValue(settingKey.key, "never");
    break;
  default:
    std::cerr << "Unrecognised 'MotionTrailRenderMode': " << static_cast<int>(value) << " value not saved!\n";
  }
}

template <>
inline void SettingsManager::set(SettingsManager::Key key, const SettingsManager::TimeUnit &value) {
  const auto &settingKey = getQtKey(key);

  switch (value) {
  case SettingsManager::TimeUnit::Milliseconds:
    qtSettings.setValue(settingKey.key, "milliseconds");
    break;
  case SettingsManager::TimeUnit::Microseconds:
    qtSettings.setValue(settingKey.key, "microseconds");
    break;
  case SettingsManager::TimeUnit::Nanoseconds:
    qtSettings.setValue(settingKey.key, "nanoseconds");
    break;
  default:
    std::cerr << "Unrecognised 'TimeUnit': " << static_cast<int>(value) << " value not saved!\n";
  }
}

template <>
inline void SettingsManager::set(SettingsManager::Key key, const SettingsManager::WindowTheme &value) {
  const auto &settingKey = getQtKey(key);

  switch (value) {
  case WindowTheme::Dark:
    qtSettings.setValue(settingKey.key, "dark");
    break;
  case WindowTheme::Light:
    qtSettings.setValue(settingKey.key, "light");
    break;
  case WindowTheme::Native:
    qtSettings.setValue(settingKey.key, "native");
    break;
  default:
    std::cerr << "Unrecognised 'WindowTheme': " << static_cast<int>(value) << " value not saved!\n";
  }
}

template <>
inline void SettingsManager::set(SettingsManager::Key key, const SettingsManager::BackgroundColor &value) {
  const auto &settingKey = getQtKey(key);

  switch (value) {
  case BackgroundColor::Black:
    qtSettings.setValue(settingKey.key, "black");
    break;
  case BackgroundColor::White:
    qtSettings.setValue(settingKey.key, "white");
    break;
  case BackgroundColor::Custom:
    qtSettings.setValue(settingKey.key, "custom");
    break;
  default:
    std::cerr << "Unrecognised 'BackgroundColor': " << static_cast<int>(value) << " value not saved!\n";
  }
}

} // namespace netsimulyzer
