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

#include "SpotLight.h"
#include <cassert>
#include <cmath>
#include <glm/glm.hpp>

namespace visualization {

std::size_t SpotLight::count = 0u;

SpotLight::SpotLight(float edge) : edge(edge), processedEdge(std::cos(glm::radians(edge))) {
  prefix = "spotLights[" + std::to_string(index) + "].";
}

void SpotLight::use(Shader &s) const {
  s.set_uniform_vector_3f(prefix + "pointLight.base.color", color);
  s.set_uniform_vector_1f(prefix + "pointLight.base.ambient_intensity", ambient_intensity);
  s.set_uniform_vector_1f(prefix + "pointLight.base.diffuse_intensity", diffuse_intensity);

  s.set_uniform_vector_3f(prefix + "pointLight.position", position);
  s.set_uniform_vector_1f(prefix + "pointLight.constant", constant);
  s.set_uniform_vector_1f(prefix + "pointLight.linear", linear);
  s.set_uniform_vector_1f(prefix + "pointLight.exponent", exponent);

  s.set_uniform_vector_1f(prefix + "edge", processedEdge);
  s.set_uniform_vector_3f(prefix + "direction", direction);
}

void SpotLight::set_edge(float value) {
  edge = value;
  processedEdge = std::cos(glm::radians(edge));
}

float SpotLight::get_edge() const {
  return edge;
}

void SpotLight::set_direction(const glm::vec3 &value) {
  direction = glm::normalize(value);
}

const glm::vec3 &SpotLight::get_direction() const {
  return direction;
}

const glm::vec3 &SpotLight::get_color() const {
  return color;
}
void SpotLight::set_color(const glm::vec3 &value) {
  color = value;
}

float SpotLight::get_ambient_intensity() const {
  return ambient_intensity;
}

void SpotLight::set_ambient_intensity(float value) {
  ambient_intensity = value;
}

void SpotLight::set_diffuse_intensity(float value) {
  diffuse_intensity = value;
}

float SpotLight::get_diffuse_intensity() const {
  return diffuse_intensity;
}

std::size_t SpotLight::getCount() {
  return SpotLight::count;
}

const glm::vec3 &SpotLight::get_position() const {
  return position;
}

void SpotLight::set_position(const glm::vec3 &value) {
  position = value;
}

float SpotLight::get_constant() const {
  return constant;
}

void SpotLight::set_constant(float value) {
  constant = value;
}

float SpotLight::get_linear() const {
  return linear;
}

void SpotLight::set_linear(float value) {
  linear = value;
}

float SpotLight::get_exponent() const {
  return exponent;
}

void SpotLight::set_exponent(float value) {
  exponent = value;
}

} // namespace visualization
