#pragma once
/**
 * File for QSettings keys
 */

#include <project.h>

namespace visualization {

/**
 * Version of the application that last updated the settings file
 */
const auto settingsVersion = "application/version";

/**
 * Setting that contains the canonical path to the `resources/` directory
 */
const auto resourcePathKey = "resources/resourcePath";

} // namespace visualization
