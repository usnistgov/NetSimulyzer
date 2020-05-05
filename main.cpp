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

#include "src/window/mainWindow.h"
#include "src/settings/SettingsManager.h"
#include <QApplication>
#include <QCoreApplication>
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QSurfaceFormat>
#include <iostream>
#include <string>
#include <project.h>

int main(int argc, char *argv[]) {
  QSurfaceFormat format;
  format.setVersion(3, 3);
  format.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(format);

  // Default QSurfaceFormat must be set before QApplication
  // on some platforms
  QApplication application(argc, argv);

  // Necessary for QSettings to save information
  // Setting these here will save us
  // rewriting them every time we construct
  // a QSettings object
  QCoreApplication::setOrganizationName("NIST");
  QCoreApplication::setOrganizationDomain("nist.gov");
  QCoreApplication::setApplicationName(VISUALIZER_APPLICATION_NAME);

  // Prevent up from using odd formats
  // Note: everything becomes a string as a result
  QSettings::setDefaultFormat(QSettings::Format::IniFormat);

  {
    QSettings settings;
    std::cout << "Settings file at: " << settings.fileName().toStdString() << '\n';
  }

  using Key = visualization::SettingsManager::Key;
  visualization::SettingsManager settings;

  if (settings.isDefined(Key::SettingsVersion)) {
    auto settingsVersion = *settings.get<std::string>(Key::SettingsVersion);

    if (settingsVersion != std::string{VISUALIZER_VERSION}) {
      // TODO: Settings Migration
      std::cout << "Warning: upgrading from previous version's settings: " << settingsVersion << " to "
                << VISUALIZER_VERSION << '\n';
      settings.set(Key::SettingsVersion, VISUALIZER_VERSION);
    }
  } else
    settings.set(Key::SettingsVersion, VISUALIZER_VERSION);

  if (!settings.isDefined(Key::ResourcePath)) {

    QDir dir{QCoreApplication::applicationDirPath(), "resources"};
    dir.setFilter(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot | QDir::Filter::Readable);

    // Try both the application directory & the working directory
    if (dir.count() == 0)
      dir.setPath(QDir::currentPath());

    if (dir.count() > 0) {
      auto entry = dir.entryInfoList()[0];
      std::cout << "Assuming resource dir at: " << entry.canonicalFilePath().toStdString() << '\n';
      settings.set(Key::ResourcePath, entry.canonicalFilePath());
    } else {
      QMessageBox::warning(nullptr, "Resource Path Not Found",
                           "The 'resources' directory was not found in the application directory. "
                           "Please set the location of the 'resources' directory in the following dialogue.");
      auto candidatePath =
#ifdef __APPLE__
          QFileDialog::getExistingDirectory(nullptr, "Select 'resources' Directory");
#else
          QFileDialog::getExistingDirectory(nullptr, "Select 'resources' Directory", "",
                                            QFileDialog::DontUseNativeDialog);
#endif

      QDir resources{candidatePath};
      if (!candidatePath.isEmpty() && resources.exists() && resources.isReadable())
        settings.set(Key::ResourcePath, resources.canonicalPath());
      else {
        QMessageBox::critical(nullptr, "Invalid 'resources' directory",
                              "An invalid 'resources' directory was selected. "
                              "A valid 'resources' directory is necessary to run the application");
        return 1;
      }
    }
  }

  std::cout << "Resources path: " << *settings.get<std::string>(Key::ResourcePath) << '\n';

  visualization::MainWindow mainWindow;
  mainWindow.show();
  return QApplication::exec();
}
