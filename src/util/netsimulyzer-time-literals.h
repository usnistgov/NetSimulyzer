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

#pragma once

/**
 * Operators for converting integer literals
 * to common time quantities
 */

#include "parser/model.h"
#include "src/util/common-times.h"
#include <cassert>
#include <limits>

namespace netsimulyzer {
/**
 * Covert the literal to hours in nanoseconds
 *
 * @param hours
 * The amount of time in hours. Should not be
 * larger than a `long long`. Will `assert`
 * in debug builds if so.
 *
 * @return
 * `hours` converted to nanoseconds
 *
 * @see src/util/common-times.h
 */
inline long long operator"" _h(unsigned long long int hours) {
  assert(std::numeric_limits<long long>::max() > hours);
  return static_cast<long long>(hours) * HOUR;
}

/**
 * Covert the literal to minutes in nanoseconds
 *
 * @param minutes
 * The amount of time in minutes. Should not be
 * larger than a `long long`. Will `assert`
 * in debug builds if so.
 *
 * @return
 * `minutes` converted to nanoseconds
 *
 * @see src/util/common-times.h
 */
inline long long operator"" _m(unsigned long long int minutes) {
  assert(std::numeric_limits<long long>::max() > minutes);
  return static_cast<long long>(minutes) * MINUTE;
}

/**
 * Covert the literal to seconds in nanoseconds
 *
 * @param seconds
 * The amount of time in seconds. Should not be
 * larger than a `long long`. Will `assert`
 * in debug builds if so.
 *
 * @return
 * `seconds` converted to nanoseconds
 *
 * @see src/util/common-times.h
 */
inline long long operator"" _s(unsigned long long int seconds) {
  assert(std::numeric_limits<long long>::max() > seconds);
  return static_cast<long long>(seconds) * SECOND;
}

/**
 * Covert the literal to milliseconds in nanoseconds
 *
 * @param milliseconds
 * The amount of time in milliseconds. Should not be
 * larger than a `long long`. Will `assert`
 * in debug builds if so.
 *
 * @return
 * `milliseconds` converted to nanoseconds
 *
 * @see src/util/common-times.h
 */
inline long long operator"" _ms(unsigned long long int milliseconds) {
  assert(std::numeric_limits<long long>::max() > milliseconds);
  return static_cast<long long>(milliseconds) * MILLISECOND;
}

/**
 * Covert the literal to microseconds in nanoseconds
 *
 * @param microseconds
 * The amount of time in microseconds. Should not be
 * larger than a `long long`. Will `assert`
 * in debug builds if so.
 *
 * @return
 * `microseconds` converted to nanoseconds
 *
 * @see src/util/common-times.h
 */
inline long long operator"" _us(unsigned long long int microseconds) {
  assert(std::numeric_limits<long long>::max() > microseconds);
  return static_cast<long long>(microseconds) * MICROSECOND;
}

} // namespace netsimulyzer
