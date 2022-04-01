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

#include "conversion.h"
#include "src/settings/SettingsManager.h"

namespace netsimulyzer {

glm::vec3 toRenderCoordinate(const parser::Ns3Coordinate &coordinate) {
  // Yes this is the right order
  return {coordinate.x, coordinate.z, -coordinate.y};
}

glm::vec3 toRenderColor(const parser::Ns3Color3 &color) {
  return {static_cast<float>(color.red) / 255.0f, static_cast<float>(color.green) / 255.0f,
          static_cast<float>(color.blue) / 255.0f};
}

glm::vec3 toRenderArray(const std::array<float, 3> &array) {
  return {array[0], array[2], array[1]};
}

QString toDisplayTime(parser::nanoseconds value, SettingsManager::TimeUnit granularity) {

  // combine / and %
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

  QString displayTimeHigh;
  QString displayTimeLow;
  if (value >= 3'600'000'000'000LL) /* 1 Hour in ns */ {
    // clang-format off
    displayTimeHigh = QString{"%1:%2:%3"}
                      .arg(hours)
                      .arg(minutes, 2, 10, QChar{'0'})
                      .arg(seconds, 2, 10, QChar{'0'});
    // clang-format on
  } else if (value >= 60'000'000'000LL) /* 1 minute in ns */ {
    // clang-format off
    displayTimeHigh = QString{"%1:%2"}
                      .arg(minutes, 2, 10, QChar{'0'})
                      .arg(seconds, 2, 10, QChar{'0'});
    // clang-format on
  } else if (value >= 1'000'000LL) {
    // clang-format off
    displayTimeHigh = QString{"%1"}
                      .arg(seconds, 2, 10, QChar{'0'});
    // clang-format on
  } else {
    displayTimeHigh = QString{"0"};
  }

  if (granularity == SettingsManager::TimeUnit::Nanoseconds) {
    // clang-format off
    displayTimeLow = QString{".%1%2%3"}
                         .arg(milliseconds, 3, 10, QChar{'0'})
                         .arg(microseconds, 3, 10, QChar{'0'})
                         .arg(nanoseconds, 3, 10, QChar{'0'});
    // clang-format on
  } else if (granularity == SettingsManager::TimeUnit::Microseconds) {
    // clang-format off
    displayTimeLow = QString{".%1%2"}
                         .arg(milliseconds, 3, 10, QChar{'0'})
                         .arg(microseconds, 3, 10, QChar{'0'});
    // clang-format on
  } else {
    // clang-format off
    displayTimeLow = QString{".%1"}
                         .arg(milliseconds, 3, 10, QChar{'0'});
    // clang-format on
  }

  return displayTimeHigh + displayTimeLow;
}

} // namespace netsimulyzer
