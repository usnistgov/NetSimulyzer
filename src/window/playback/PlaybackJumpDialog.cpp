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

#include "PlaybackJumpDialog.h"
#include "src/util/common-times.h"
#include "src/util/netsimulyzer-time-literals.h"
#include "ui_PlaybackJumpDialog.h"
#include <QMessageBox>
#include <QObject>
#include <QRegularExpression>
#include <QValidator>

namespace {
parser::nanoseconds parseInput(const QString &input) {
  long long hours{};
  long long minutes{};
  long long seconds{};
  long long nanoseconds{};
  const auto split = input.split(QRegularExpression{"([:|.])"});

  int index{};
  for (auto iter = split.rbegin(); iter != split.rend(); iter++, index++) {
    if (index == 0)
      nanoseconds = iter->toLongLong();
    else if (index == 1)
      seconds = iter->toLongLong();
    else if (index == 2)
      minutes = iter->toLongLong();
    else if (index == 3)
      hours = iter->toLongLong();
  }

  return hours * netsimulyzer::HOUR + minutes * netsimulyzer::MINUTE + seconds * netsimulyzer::SECOND + nanoseconds;
}

QString formatInputTime(parser::nanoseconds value, parser::nanoseconds maxTime) {
  using namespace netsimulyzer;

  auto result = std::div(value, 1000LL);
  const auto nanoseconds = result.rem;

  result = std::div(result.quot, 1000LL);
  const auto microseconds = result.rem;

  result = std::div(result.quot, 1000LL);
  const auto milliseconds = result.rem;

  result = std::div(result.quot, 60LL);
  const auto seconds = result.rem;

  result = std::div(result.quot, 60LL);
  const auto minutes = result.rem;

  // Dump the rest in as hours
  const auto hours = result.quot;

  if (maxTime > HOUR) {
    if (maxTime > 100_h) {
      return QString{"%1:%2:%3.%4%5%6"}
          .arg(hours, 3, 10, QChar{'0'})
          .arg(minutes, 2, 10, QChar{'0'})
          .arg(seconds, 2, 10, QChar{'0'})
          .arg(milliseconds, 3, 10, QChar{'0'})
          .arg(microseconds, 3, 10, QChar{'0'})
          .arg(nanoseconds, 3, 10, QChar{'0'});
    } else if (maxTime > 10_h) {
      return QString{"%1:%2:%3.%4%5%6"}
          .arg(hours, 2, 10, QChar{'0'})
          .arg(minutes, 2, 10, QChar{'0'})
          .arg(seconds, 2, 10, QChar{'0'})
          .arg(milliseconds, 3, 10, QChar{'0'})
          .arg(microseconds, 3, 10, QChar{'0'})
          .arg(nanoseconds, 3, 10, QChar{'0'});
    } else {
      return QString{"%1:%2:%3.%4%5%6"}
          .arg(hours, 1, 10, QChar{'0'})
          .arg(minutes, 2, 10, QChar{'0'})
          .arg(seconds, 2, 10, QChar{'0'})
          .arg(milliseconds, 3, 10, QChar{'0'})
          .arg(microseconds, 3, 10, QChar{'0'})
          .arg(nanoseconds, 3, 10, QChar{'0'});
    }
  } else if (maxTime > MINUTE) {
    if (maxTime > 10_m) {
      return QString{"%1:%2.%3%4%5"}
          .arg(minutes, 2, 10, QChar{'0'})
          .arg(seconds, 2, 10, QChar{'0'})
          .arg(milliseconds, 3, 10, QChar{'0'})
          .arg(microseconds, 3, 10, QChar{'0'})
          .arg(nanoseconds, 3, 10, QChar{'0'});
    } else {
      return QString{"%1:%2.%3%4%5"}
          .arg(minutes, 1, 10, QChar{'0'})
          .arg(seconds, 2, 10, QChar{'0'})
          .arg(milliseconds, 3, 10, QChar{'0'})
          .arg(microseconds, 3, 10, QChar{'0'})
          .arg(nanoseconds, 3, 10, QChar{'0'});
    }
  } else { // Seconds
    if (maxTime > 10_s) {
      return QString{"%1:%2.%3%4%5"}
          .arg(seconds, 2, 10, QChar{'0'})
          .arg(milliseconds, 3, 10, QChar{'0'})
          .arg(microseconds, 3, 10, QChar{'0'})
          .arg(nanoseconds, 3, 10, QChar{'0'});
    } else {
      return QString{"%1:%2.%3%4%5"}
          .arg(seconds, 1, 10, QChar{'0'})
          .arg(milliseconds, 3, 10, QChar{'0'})
          .arg(microseconds, 3, 10, QChar{'0'})
          .arg(nanoseconds, 3, 10, QChar{'0'});
    }
  }
}
} // namespace

namespace netsimulyzer {

PlaybackJumpDialog::PlaybackJumpDialog(QWidget *parent) : QDialog(parent) {
  ui.setupUi(this);

  ui.timeEdit->setValidator(&validator);

  QObject::connect(ui.btnJump, &QPushButton::clicked, [this]() {
    emit timeSelected(parseInput(ui.timeEdit->text()));
  });
}

void PlaybackJumpDialog::setMaxTime(parser::nanoseconds value) {
  maxTime = value;
  validator.setMaxTime(value);

  // Create an input mask based around the max time
  if (maxTime > HOUR) {
    if (maxTime >= 100_h)
      ui.timeEdit->setInputMask("000:00:00.000000000");
    else if (maxTime >= 10_h)
      ui.timeEdit->setInputMask("00:00:00.000000000");
    else
      ui.timeEdit->setInputMask("0:00:00.000000000");
  } else if (maxTime > MINUTE) {
    if (maxTime >= 10_m)
      ui.timeEdit->setInputMask("00:00.000000000");
    else
      ui.timeEdit->setInputMask("0:00.000000000");
  } else { // Seconds
    if (maxTime >= 10_s)
      ui.timeEdit->setInputMask("00.000000000");
    else
      ui.timeEdit->setInputMask("0.000000000");
  }
}

void PlaybackJumpDialog::setInputValue(parser::nanoseconds value) {
  ui.timeEdit->setText(formatInputTime(value, maxTime));
}

PlaybackJumpDialog::TimeValidator::TimeValidator(QObject *parent) : QValidator(parent) {
}

void PlaybackJumpDialog::TimeValidator::setMaxTime(parser::nanoseconds value) {
  maxTime = value;
}
void PlaybackJumpDialog::TimeValidator::fixup(QString &string) const {
  const auto parsed = parseInput(string);
  if (parsed > maxTime) {
    string.clear();
    const auto formattedMax = formatInputTime(maxTime, maxTime);
    string.append(formattedMax);
    return;
  } else if (parsed < 0LL) {
    string.clear();
    const auto formattedZero = formatInputTime(0LL, maxTime);
    string.append(formattedZero);
    return;
  }
}
QValidator::State PlaybackJumpDialog::TimeValidator::validate(QString &string, int &) const {
  const auto split = string.split(QRegularExpression{"([:|.])"});

  int index{};
  for (auto iter = split.rbegin(); iter != split.rend(); iter++, index++) {
    // Check if seconds & minutes are > 60
    if ((index == 1 || index == 2) && iter->toLongLong() > 60LL)
      return State::Invalid;
  }

  const auto parsed = parseInput(string);
  if (parsed > maxTime)
    return State::Invalid;

  return State::Acceptable;
}
} // namespace netsimulyzer
