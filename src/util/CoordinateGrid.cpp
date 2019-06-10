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

#include "CoordinateGrid.h"
#include <osg/Geometry>
#include <osg/PrimitiveSet>
#include <osg/ref_ptr>

namespace visualization {

CoordinateGrid::CoordinateGrid(int size, int steps, osg::Vec4 centerLineColor, osg::Vec4 nonCenterLineColor) {
  getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

  auto stepSize = size / steps;

  // Split the size so our center lands at halfway through the size
  auto halfSize = size / 2;

  auto points = new osg::Vec3Array;
  auto centerPoints = new osg::Vec3Array;

  // Horizontal lines
  for (auto step = -halfSize; step <= halfSize; step += stepSize) {
    if (step == 0) {
      centerPoints->push_back(osg::Vec3(-halfSize, step, 0));
      centerPoints->push_back(osg::Vec3(halfSize, step, 0));
    } else {
      points->push_back(osg::Vec3(-halfSize, step, 0));
      points->push_back(osg::Vec3(halfSize, step, 0));
    }
  }

  // Vertical Lines
  for (auto step = -halfSize; step <= halfSize; step += stepSize) {
    if (step == 0) {
      centerPoints->push_back(osg::Vec3(step, -halfSize, 0));
      centerPoints->push_back(osg::Vec3(step, halfSize, 0));
    } else {
      points->push_back(osg::Vec3(step, -halfSize, 0));
      points->push_back(osg::Vec3(step, halfSize, 0));
    }
  }

  // Define the center/nonCenter lines in separate Geometries
  // to make coloring them much easier

  auto nonCenterGrid = new osg::Geometry;
  nonCenterGrid->setVertexArray(points);
  nonCenterGrid->addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, points->size()));
  {
    auto nonCenterColorArray = new osg::Vec4Array;
    nonCenterColorArray->push_back(nonCenterLineColor);
    nonCenterGrid->setColorArray(nonCenterColorArray, osg::Array::BIND_OVERALL);
  }
  addChild(nonCenterGrid);

  auto centerGrid = new osg::Geometry;
  centerGrid->setVertexArray(centerPoints);
  centerGrid->addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, centerPoints->size()));
  {
    auto centerColorArray = new osg::Vec4Array;
    centerColorArray->push_back(centerLineColor);
    centerGrid->setColorArray(centerColorArray, osg::Array::BIND_OVERALL);
  }
  addChild(centerGrid);
}

} // namespace visualization
