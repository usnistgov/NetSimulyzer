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

#include "BuildingGroup.h"
#include <osg/Array>
#include <osg/BlendFunc>
#include <osg/Geometry>
#include <osg/Material>

namespace {

osg::ref_ptr<osg::Geode> makeGeode(const visualization::Building &building) {
  osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array();

  // Front
  vertices->push_back(osg::Vec3(building.xMin, building.yMin, building.zMin)); // 0
  vertices->push_back(osg::Vec3(building.xMax, building.yMin, building.zMin)); // 1
  vertices->push_back(osg::Vec3(building.xMax, building.yMin, building.zMax)); // 2

  vertices->push_back(osg::Vec3(building.xMin, building.yMin, building.zMax)); // 3
  vertices->push_back(osg::Vec3(building.xMin, building.yMin, building.zMin)); // 0
  vertices->push_back(osg::Vec3(building.xMax, building.yMin, building.zMax)); // 2

  // Right
  vertices->push_back(osg::Vec3(building.xMax, building.yMin, building.zMin)); // 1
  vertices->push_back(osg::Vec3(building.xMax, building.yMax, building.zMin)); // 5
  vertices->push_back(osg::Vec3(building.xMax, building.yMax, building.zMax)); // 6

  vertices->push_back(osg::Vec3(building.xMax, building.yMin, building.zMax)); // 2
  vertices->push_back(osg::Vec3(building.xMax, building.yMin, building.zMin)); // 1
  vertices->push_back(osg::Vec3(building.xMax, building.yMax, building.zMax)); // 6

  // Back
  vertices->push_back(osg::Vec3(building.xMin, building.yMax, building.zMin)); // 4
  vertices->push_back(osg::Vec3(building.xMax, building.yMax, building.zMin)); // 5
  vertices->push_back(osg::Vec3(building.xMax, building.yMax, building.zMax)); // 6

  vertices->push_back(osg::Vec3(building.xMin, building.yMax, building.zMax)); // 7
  vertices->push_back(osg::Vec3(building.xMin, building.yMax, building.zMin)); // 4
  vertices->push_back(osg::Vec3(building.xMax, building.yMax, building.zMax)); // 6

  // Left
  vertices->push_back(osg::Vec3(building.xMin, building.yMin, building.zMin)); // 0
  vertices->push_back(osg::Vec3(building.xMin, building.yMax, building.zMin)); // 4
  vertices->push_back(osg::Vec3(building.xMin, building.yMax, building.zMax)); // 7

  vertices->push_back(osg::Vec3(building.xMin, building.yMin, building.zMax)); // 3
  vertices->push_back(osg::Vec3(building.xMin, building.yMin, building.zMin)); // 0
  vertices->push_back(osg::Vec3(building.xMin, building.yMax, building.zMax)); // 7

  // Bottom
  vertices->push_back(osg::Vec3(building.xMin, building.yMin, building.zMin)); // 0
  vertices->push_back(osg::Vec3(building.xMax, building.yMin, building.zMin)); // 1
  vertices->push_back(osg::Vec3(building.xMax, building.yMax, building.zMin)); // 5

  vertices->push_back(osg::Vec3(building.xMin, building.yMax, building.zMin)); // 4
  vertices->push_back(osg::Vec3(building.xMin, building.yMin, building.zMin)); // 0
  vertices->push_back(osg::Vec3(building.xMax, building.yMax, building.zMin)); // 5

  // Top
  vertices->push_back(osg::Vec3(building.xMin, building.yMin, building.zMax)); // 3
  vertices->push_back(osg::Vec3(building.xMax, building.yMin, building.zMax)); // 2
  vertices->push_back(osg::Vec3(building.xMax, building.yMax, building.zMax)); // 6

  vertices->push_back(osg::Vec3(building.xMin, building.yMax, building.zMax)); // 7
  vertices->push_back(osg::Vec3(building.xMin, building.yMin, building.zMax)); // 3
  vertices->push_back(osg::Vec3(building.xMax, building.yMax, building.zMax)); // 6

  osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
  geometry->setVertexArray(vertices.get());
  geometry->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLES, 0, vertices->size()));

  osg::ref_ptr<osg::Geode> geo = new osg::Geode;
  geo->addDrawable(geometry);

  return geo;
}

} // namespace

namespace visualization {

osg::ref_ptr<BuildingGroup> BuildingGroup::makeGroup(const visualization::Building &config) {
  osg::ref_ptr<BuildingGroup> building = new BuildingGroup();

  building->geode = makeGeode(config);
  if (config.opacity < 1) {
    auto stateSet = building->getOrCreateStateSet();
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    auto attr = stateSet->getAttribute(osg::StateAttribute::MATERIAL);
    osg::ref_ptr<osg::Material> material;

    if (attr)
      material = dynamic_cast<osg::Material *>(attr);
    else
      material = new osg::Material();

    material->setAlpha(osg::Material::FRONT_AND_BACK, static_cast<float>(config.opacity));
    stateSet->setAttributeAndModes(material, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    building->setStateSet(stateSet);

    building->getOrCreateStateSet()->setAttributeAndModes(
        new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA));
  }

  building->visible = new osg::Switch();
  building->visible->addChild(building->geode, config.visible);

  building->addChild(building->visible);
  return building;
}

} // namespace visualization
