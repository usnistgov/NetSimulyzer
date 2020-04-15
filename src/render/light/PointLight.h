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
#include "light.h"
#include <glm/glm.hpp>
#include <string>

namespace visualization {

class point_light : public light {
  glm::vec3 position{0.0f};
  float constant{1.0f};
  float linear{0.0f};
  float exponent{0.0f};
  std::size_t index = point_light::count++;

  std::string prefix;
  static std::size_t count;

public:
  point_light();
  point_light(glm::vec3 color, float ambient_intensity, float diffuse_intensity, glm::vec3 position, float constant,
              float linear, float exponent);

  // Allow Moves
  point_light(point_light &&) = default;
  point_light &operator=(point_light &&) = default;

  // Disallow copies
  point_light(const point_light &) = delete;
  point_light &operator=(const point_light &) = delete;

  void use(Shader &s) const override;

  [[nodiscard]] static std::size_t getCount();

  [[nodiscard]] const glm::vec3 &get_position() const;
  void set_position(const glm::vec3 &value);

  [[nodiscard]] float get_constant() const;
  void set_constant(float value);

  [[nodiscard]] float get_linear() const;
  void set_linear(float value);

  [[nodiscard]] float get_exponent() const;
  void set_exponent(float value);
};

} // namespace visualization
