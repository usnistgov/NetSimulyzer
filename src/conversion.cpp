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

namespace netsimulyzer {

glm::vec3 toRenderCoordinate(const parser::Ns3Coordinate &coordinate) {
  // Yes this is the right order
  return {coordinate.x, coordinate.z, -coordinate.y};
}

glm::vec3 toRenderColor(const parser::Ns3Color3 &color) {
  return {static_cast<float>(color.red) / 255.0f, static_cast<float>(color.green) / 255.0f,
          static_cast<float>(color.blue) / 255.0f};
}

QString toDisplayTime(double value) {
  auto convertedTime = static_cast<long>(value);

  // combine / and %
  auto result = std::div(convertedTime, 1000L);
  auto milliseconds = result.rem;

  result = std::div(result.quot, 60L);
  auto seconds = result.rem;

  result = std::div(result.quot, 60L);
  auto minutes = result.rem;

  // Dump the rest in as hours
  auto hours = result.quot;

  QString displayTime;
  if (convertedTime > 3'600'000) /* 1 Hour in ms */ {
    displayTime = QString{"%1:%2:%3.%4"}
                      .arg(hours)
                      .arg(minutes, 2, 10, QChar{'0'})
                      .arg(seconds, 2, 10, QChar{'0'})
                      .arg(milliseconds, 3, 10, QChar{'0'});
  } else if (convertedTime > 60'000L) /* 1 minute in ms */ {
    displayTime = QString{"%1:%2.%3"}
                      .arg(minutes, 2, 10, QChar{'0'})
                      .arg(seconds, 2, 10, QChar{'0'})
                      .arg(milliseconds, 3, 10, QChar{'0'});
  } else if (convertedTime > 1000L) {
    displayTime = QString{"%1.%2"}.arg(seconds, 2, 10, QChar{'0'}).arg(milliseconds, 3, 10, QChar{'0'});
  } else {
    displayTime.append('.' + QString::number(milliseconds));
  }

  return displayTime;
}

} // namespace netsimulyzer
