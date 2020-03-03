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
#include <cstdint>
#include <osg/Vec3d>
#include <string>
#include <vector>

namespace visualization {

struct GlobalConfiguration {
  double millisecondsPerFrame = 1.0;
};

struct Node {
  uint32_t id = 0;
  std::string model;
  double scale = 1.0;
  double opacity = 1.0;
  bool visible = true;
  osg::Vec3d position;
  osg::Vec3d orientation;
};

struct Building {
  uint32_t id = 0;
  double opacity = 1.0;
  bool visible = true;
  uint16_t floors = 0;
  uint16_t roomsX = 0;
  uint16_t roomsY = 0;
  double xMin = 0.0;
  double xMax = 0.0;

  double yMin = 0.0;
  double yMax = 0.0;

  double zMin = 0.0;
  double zMax = 0.0;
};

struct Decoration {
  uint32_t id;
  std::string model;
  osg::Vec3d position;
  osg::Vec3d orientation;
  double opacity = 1.0;
  double scale = 1.0;
};

struct ValueAxis {
  enum class BoundMode { Fixed, HighestValue };
  enum class Scale { Linear, Logarithmic };

  std::string name;
  BoundMode boundMode = BoundMode::HighestValue;
  Scale scale = Scale::Linear;
  double min = 0.0;
  double max = 0.0;
};

struct XYSeries {
  enum class Connection { None, Line, Spline };
  enum class LabelMode { Hidden, Shown };

  uint32_t id = 0u;
  std::string name;
  Connection connection = Connection::Line;
  LabelMode labelMode = LabelMode::Shown;
  uint8_t red = 0u;
  uint8_t green = 0u;
  uint8_t blue = 0u;
  uint8_t alpha = 255u;
  ValueAxis xAxis;
  ValueAxis yAxis;
};

struct SeriesCollection {
  uint32_t id = 0u;
  std::string name;
  std::vector<uint32_t> series;
  ValueAxis xAxis;
  ValueAxis yAxis;
};

} // namespace visualization
