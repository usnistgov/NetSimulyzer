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

#include "AboutDialog.h"
#include "ui_AboutDialog.h"
#include <QPixmap>
#include <QString>
#include <QSysInfo>
#include <project.h>

namespace netsimulyzer {
AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent) {
  ui.setupUi(this);

  ui.labelLogo->setPixmap(QPixmap{":/application/application.png"});
  ui.labelLogo->setFixedSize(256, 185);

  QString compliedArchitecture;
  // Only useful for detection for x86 based architectures
  if (sizeof(void *) == 8u) {
    compliedArchitecture = "x64";
  } else if (sizeof(void *) == 4u) {
    compliedArchitecture = "x86";
  }
  QString releaseType;
#ifdef NDEBUG
  releaseType = "Release";
#else
  releaseType = "Debug";
#endif

  QString compiler;
#if defined(__clang__) && defined(__clang_version__)
  compiler = "clang++ " + QString(__clang_version__);
#elif defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
  compiler = "g++ " + QString::number(__GNUC__) + '.' + QString::number(__GNUC_MINOR__) + '.' +
             QString::number(__GNUC_PATCHLEVEL__);
#elif defined(_MSC_VER)
  compiler = "MSVC " + QString::number(_MSC_VER);
#else
  compiler = "Unknown Compiler";
#endif

  ui.labelApplicationName->setText(NETSIMULYZER_VERSION);
  ui.labelVersion->setText("v" NETSIMULYZER_VERSION " (" + compliedArchitecture + ", " + releaseType + ')');
  ui.labelTarget->setText(NETSIMULYZER_VERSION " for " + QSysInfo::prettyProductName());
  ui.labelCompiler->setText(compiler);

  ui.plainTextEditLicense->setPlainText(
      "NIST Software Disclaimer\n"
      "\n"
      "NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the "
      "software in any medium, provided that you keep intact this entire notice. You may improve, modify and create "
      "derivative works of the software or any portion of the software, and you may copy and distribute such "
      "modifications or works. Modified works should carry a notice stating that you changed the software and should "
      "note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards "
      "and Technology as the source of the software.\n"
      "\n"
      "NIST-developed software is expressly provided \"AS IS.\" NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, "
      "IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, "
      "FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT "
      "THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST "
      "DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, "
      "INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.\n"
      "\n"
      "You are solely responsible for determining the appropriateness of using and distributing the software and you "
      "assume all risks associated with its use, including but not limited to the risks and costs of program errors, "
      "compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or "
      "interruption of operation. This software is not intended to be used in any situation where a failure could "
      "cause risk of injury or damage to property. The software developed by NIST employees is not subject to "
      "copyright protection within the United States.");
}

} // namespace netsimulyzer
