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

#include "PointLight.h"
#include <cassert>
#include <string>

namespace visualization {

std::size_t point_light::count = 0;

point_light::point_light() {
  if (point_light::count > 3)
    assert(!"Exceeded max point lights");
  prefix = "pointLights[" + std::to_string(index) + "].";
}

point_light::point_light(glm::vec3 color, float ambient_intensity, float diffuse_intensity, glm::vec3 position,
                         float constant, float linear, float exponent)
    : light(color, ambient_intensity, diffuse_intensity), position(position), constant(constant), linear(linear),
      exponent(exponent) {
  if (point_light::count > 3)
    assert(!"Exceeded max point lights");
  prefix = "pointLights[" + std::to_string(index) + "].";
}

void point_light::use(Shader &s) const {
  s.uniform(prefix + "base.color", color);
  s.uniform(prefix + "base.ambient_intensity", ambient_intensity);
  s.uniform(prefix + "base.diffuse_intensity", diffuse_intensity);
  s.uniform(prefix + "position", position);
  s.uniform(prefix + "constant", constant);
  s.uniform(prefix + "linear", linear);
  s.uniform(prefix + "exponent", exponent);
}

std::size_t point_light::getCount() {
  return point_light::count;
}

const glm::vec3 &point_light::get_position() const {
  return position;
}

void point_light::set_position(const glm::vec3 &value) {
  position = value;
}

float point_light::get_constant() const {
  return constant;
}

void point_light::set_constant(float value) {
  constant = value;
}

float point_light::get_linear() const {
  return linear;
}

void point_light::set_linear(float value) {
  linear = value;
}

float point_light::get_exponent() const {
  return exponent;
}
void point_light::set_exponent(float value) {
  exponent = value;
}

} // namespace visualization
