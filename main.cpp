/*
 * NIST-developed software is provided by NIST as a public service. You may use,
 * copy and distribute copies of the software in any medium, provided that you
 * keep intact this entire notice. You may improve,modify and create derivative
 * works of the software or any portion of the software, and you may copy and
 * distribute such modifications or works. Modified works should carry a notice
 * stating that you changed the software and should note the date and nature of
 * any such change. Please explicitly acknowledge the National Institute of
 * Standards and Technology as the source of the software.
 *
 * NIST-developed software is expressly provided "AS IS." NIST MAKES NO
 * WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF
 * LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT
 * AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE
 * OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT
 * ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY
 * REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF,
 * INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY,
 * OR USEFULNESS OF THE SOFTWARE.
 *
 * You are solely responsible for determining the appropriateness of using and
 * distributing the software and you assume all risks associated with its use,
 * including but not limited to the risks and costs of program errors,
 * compliance with applicable laws, damage to or loss of data, programs or
 * equipment, and the unavailability or interruption of operation. This
 * software is not intended to be used in any situation where a failure could
 * cause risk of injury or damage to property. The software developed by NIST
 * employees is not subject to copyright protection within the United States.
 *
 * Author: Evan Black <evan.black@nist.gov>
 */
// clang-format off
// TODO: Maybe unnecessary now?
#include <QOpenGLContext>
// clang-format on

#include "src/settings/SettingsManager.h"
#include "src/window/MainWindow.h"
#include "src/window/util/file-operations.h"
#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QSurfaceFormat>
#include <iostream>
#include <optional>
#include <project.h>
#include <string>

// Signals to Qt that titles beginning with ampersands (&)
// should be generated from the characters following the
// ampersand.
//
// Since this is not declared in any Qt header files, we have to do
// it ourselves.
// See: https://doc.qt.io/qt-5/qkeysequence.html#qt_set_sequence_auto_mnemonic
void qt_set_sequence_auto_mnemonic(bool b);

struct ParsedSettingsVersion {
  unsigned int major;
  unsigned int minor;
  unsigned int patch;
};

bool operator<(const ParsedSettingsVersion &left, const ParsedSettingsVersion &right) {
  if (left.major != right.major)
    return left.major < right.major;

  if (left.minor != right.minor)
    return left.minor < right.minor;

  return left.patch < right.patch;
}

ParsedSettingsVersion parseVersion(const std::string &s) {
  ParsedSettingsVersion result;
  std::size_t position = 0u;
  for (auto i = 0; i < 3; i++) {
    auto next = s.find('.', position);
    const auto &str = s.substr(position, next);

    switch (i) {
    case 0:
      result.major = std::stoi(str);
      break;
    case 1:
      result.minor = std::stoi(str);
      break;
    case 2:
      result.patch = std::stoi(str);
      break;
    }

    // +1 to move past the '.' character
    position = next + 1;
  }

  return result;
}

/**
 * Checks if a path is a readable directory
 *
 * @param path
 * The path to check
 *
 * @return
 * True if `path` is a readable directory,
 * False otherwise
 */
bool validateResourceDir(const QString &path) {
  if (path.isEmpty())
    return false;

  QFileInfo fileInfo{path};
  return fileInfo.isDir() && fileInfo.isReadable();
}

/**
 * Prompt the user for a resource directory.
 * If the selected directory is invalid, shows an error
 *
 * @return
 * The path from the user. Empty string if nothing was selected
 */
std::optional<QFileInfo> promptResourceDir() {
  auto selected = netsimulyzer::getExistingDirectory("Select 'resources' Directory");

  if (!validateResourceDir(selected)) {
    QMessageBox::critical(nullptr, "Invalid resource directory selected",
                          "An invalid resources directory was selected. "
                          "A valid 'resources' directory is necessary to run the application");

    return {};
  }

  return {QFileInfo{selected}};
}

/**
 * Check the application directory and the current working
 * directory (in that order) for a valid resource directory
 *
 * @return
 * A set optional with the path to the detected directory
 * if one matches the above criteria, an unset optional
 * otherwise
 */
std::optional<QFileInfo> autodetectResourceDir() {
  const auto addOneUp = [] (const QString &path) -> QString {
    QDir d{path};
    d.cdUp();
    return d.absolutePath();
  };

  std::array<QString, 4> checkPaths{
      QCoreApplication::applicationDirPath(),
      addOneUp(QCoreApplication::applicationDirPath()),
      QDir::currentPath(),
      addOneUp(QDir::currentPath())
  };

  QDir dir{"", "resources"};
  dir.setFilter(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot | QDir::Filter::Readable);

  for (const auto &path: checkPaths) {
    // Make sure the path exists first
    if (!dir.cd(path))
      continue;

    if (dir.count() == 0u)
      continue;

    dir.cd("resources");

    if (validateResourceDir(dir.absolutePath()))
      return {QFileInfo{dir.absolutePath()}};
  }

  return {};
}

int main(int argc, char *argv[]) {
  // Necessary for QSettings to save information
  // Setting these here will save us
  // rewriting them every time we construct
  // a QSettings object
  QCoreApplication::setOrganizationName("NIST");
  QCoreApplication::setOrganizationDomain("nist.gov");
  QCoreApplication::setApplicationName(NETSIMULYZER_APPLICATION_NAME);

  // Prevent up from using odd formats
  // Note: everything becomes a string as a result
  QSettings::setDefaultFormat(QSettings::Format::IniFormat);

  using Key = netsimulyzer::SettingsManager::Key;
  using RetrieveMode = netsimulyzer::SettingsManager::RetrieveMode;
  netsimulyzer::SettingsManager settings;

  if (settings.isDefined(Key::SettingsVersion)) {
    const auto settingsVersion = *settings.get<std::string>(Key::SettingsVersion);
    using SettingsManager = netsimulyzer::SettingsManager;

    if (settingsVersion != std::string{NETSIMULYZER_VERSION}) {
      std::cout << "Warning: upgrading from previous version's settings: " << settingsVersion << " to "
                << NETSIMULYZER_VERSION << '\n';

      const auto version = parseVersion(settingsVersion);
      if (version < ParsedSettingsVersion{1,0,6}) {
        std::cout << "Migrating: 1.0.6\n";
        QSettings qSettings;
        const auto oldVal = qSettings.value("renderer/showMotionTrails", false);
        if (oldVal.canConvert<bool>() && oldVal.toBool())
          settings.set(Key::RenderMotionTrails, SettingsManager::MotionTrailRenderMode::Always);
        else
          settings.setDefault(Key::RenderMotionTrails);
      }

      if (version < ParsedSettingsVersion{1, 0, 7}) {
        std::cout << "Migrating: 1.0.7\n";
        std::cout << "Setting theme to default\n";
        settings.set(Key::WindowTheme, settings.getDefault<SettingsManager::WindowTheme>(Key::WindowTheme));
      }

      // Clear the resource directory on update,
      // just in case the user did not delete the old one.
      // It will be re-detected below
      settings.clear(Key::ResourcePath);
      settings.set(Key::SettingsVersion, NETSIMULYZER_VERSION);
      settings.sync();
    }
  } else
    settings.set(Key::SettingsVersion, NETSIMULYZER_VERSION);

  QSurfaceFormat format;
  format.setVersion(3, 3);
  auto samples = *settings.get<int>(Key::NumberSamples, RetrieveMode::AllowDefault);
  format.setSamples(samples);
  format.setProfile(QSurfaceFormat::CoreProfile);
#ifndef NDEBUG
  // Only enable debug logging for debug builds
  format.setOption(QSurfaceFormat::DebugContext);
#endif
  QSurfaceFormat::setDefaultFormat(format);

  // Default QSurfaceFormat must be set before QApplication
  // on some platforms
  QApplication application(argc, argv);

  // Make sure the theme stylesheets are loaded
  // before we open anything
  settings.setTheme();

  // Set default window icon for the whole application
  // seems to be required on macOS, but shouldn't hurt
  // on other platforms as well
  QApplication::setWindowIcon(QIcon{":/application/application.png"});

  // Enable auto mnemonics on all platforms
  // since our widget titles use them
  qt_set_sequence_auto_mnemonic(true);

  // Must me checked after the QApplication is constructed
  // since this may create dialogs
  if (settings.isDefined(Key::ResourcePath)) {
    auto savedResourcePath = *settings.get<QString>(Key::ResourcePath);
    if (!validateResourceDir(savedResourcePath)) {
      // Old resource path is invalid, try detecting one

      auto detectedResourcePath = autodetectResourceDir();
      if (detectedResourcePath) {
        std::clog << "Assuming resource dir: " << detectedResourcePath->absoluteFilePath().toStdString() << '\n';
        settings.set(Key::ResourcePath, detectedResourcePath->absoluteFilePath());
      } else {
        // Auto detected paths also are invalid, we have to prompt
        QMessageBox::warning(nullptr, "Resource Path Not Accessible",
                             "The resource path :\"" + savedResourcePath + "\" is inaccessible, select a new one");

        auto newResourcePath = promptResourceDir();
        if (!newResourcePath)
          return 1;

        settings.set(Key::ResourcePath, newResourcePath->absoluteFilePath());
      }

      // Make sure any changes to the resource directory are committed
      settings.sync();
    }
  } else {
    // No resource directory defined, try to autodetect one
    auto detectedResourcePath = autodetectResourceDir();
    if (detectedResourcePath) {
      std::clog << "Assuming resource dir: " << detectedResourcePath->absoluteFilePath().toStdString() << '\n';
      settings.set(Key::ResourcePath, detectedResourcePath->absoluteFilePath());
    } else {
      QMessageBox::warning(nullptr, "Resource Path Not Found",
                           "The 'resources' directory was not found in the application directory. "
                           "Please set the location of the 'resources' directory.");

      auto selected = promptResourceDir();
      if (!selected)
        return 1;

      settings.set(Key::ResourcePath, selected->absoluteFilePath());
    }
    // Make sure any changes to the resource directory are committed
    settings.sync();
  }
  netsimulyzer::MainWindow mainWindow;
  mainWindow.show();
  return QApplication::exec();
}
