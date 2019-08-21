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
#include <osgUtil/RenderBin>

/**
 * Separate anonymous namespace for constructing parts of the building geometry
 */
namespace {

/**
 * Build the outer walls of the building
 *
 * @param building
 * The building we're building the outer walls for.
 * Defines the dimensions, location of the walls.
 *
 * @return
 * The constructed wall geometry
 */
osg::ref_ptr<osg::Geometry> makeOutsideWalls(const visualization::Building &building) {
  auto vertices = new osg::Vec3Array();
  vertices->push_back(osg::Vec3(building.xMin, building.yMin, building.zMin)); // 0
  vertices->push_back(osg::Vec3(building.xMax, building.yMin, building.zMin)); // 1
  vertices->push_back(osg::Vec3(building.xMax, building.yMin, building.zMax)); // 2
  vertices->push_back(osg::Vec3(building.xMin, building.yMin, building.zMax)); // 3
  vertices->push_back(osg::Vec3(building.xMin, building.yMax, building.zMin)); // 4
  vertices->push_back(osg::Vec3(building.xMax, building.yMax, building.zMin)); // 5
  vertices->push_back(osg::Vec3(building.xMax, building.yMax, building.zMax)); // 6
  vertices->push_back(osg::Vec3(building.xMin, building.yMax, building.zMax)); // 7

  auto drawElements = new osg::DrawElementsUInt(GL_TRIANGLES);
  drawElements->reserve(36); // 6 points per side, 6 sides

  // Front
  drawElements->push_back(0);
  drawElements->push_back(1);
  drawElements->push_back(2);

  drawElements->push_back(3);
  drawElements->push_back(0);
  drawElements->push_back(2);

  // Right
  drawElements->push_back(1);
  drawElements->push_back(5);
  drawElements->push_back(6);

  drawElements->push_back(2);
  drawElements->push_back(1);
  drawElements->push_back(6);

  // Back
  drawElements->push_back(4);
  drawElements->push_back(5);
  drawElements->push_back(6);

  drawElements->push_back(7);
  drawElements->push_back(4);
  drawElements->push_back(6);

  // Left
  drawElements->push_back(0);
  drawElements->push_back(4);
  drawElements->push_back(7);

  drawElements->push_back(3);
  drawElements->push_back(0);
  drawElements->push_back(7);

  // Bottom
  drawElements->push_back(0);
  drawElements->push_back(1);
  drawElements->push_back(5);

  drawElements->push_back(4);
  drawElements->push_back(0);
  drawElements->push_back(5);

  // Top
  drawElements->push_back(3);
  drawElements->push_back(2);
  drawElements->push_back(6);

  drawElements->push_back(7);
  drawElements->push_back(3);
  drawElements->push_back(6);

  osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
  geometry->setVertexArray(vertices);
  geometry->addPrimitiveSet(drawElements);

  return geometry;
}

/**
 * Build the interior 'floors' of the building (i.e. not the top/bottom).
 * Only useful if the building has more than one floor
 *
 * @param building
 * The building we're building the floors for.
 * Defines the dimensions, location of the floors.
 *
 * @return
 * The constructed floor geometry
 */
osg::ref_ptr<osg::Geometry> makeFloors(const visualization::Building &building) {
  osg::ref_ptr<osg::Geometry> floors = new osg::Geometry;

  osg::ref_ptr<osg::Vec3Array> floorVertices = new osg::Vec3Array();

  // All floors are exactly the same height
  // abs() the zMax and zMin just in case our coordinates are negative
  auto floorHeight = (abs(building.zMax) - abs(building.zMin)) / building.floors;

  // Start from the bottom and work our way to the top
  // drawing each floor.
  // Only loop until floors - 1 since there's no need to draw the roof this way
  for (auto currentFloor = 1; currentFloor < building.floors; currentFloor++) {
    auto currentHeight = floorHeight * currentFloor;
    floorVertices->push_back(osg::Vec3(building.xMin, building.yMin, currentHeight));
    floorVertices->push_back(osg::Vec3(building.xMax, building.yMin, currentHeight));
    floorVertices->push_back(osg::Vec3(building.xMax, building.yMax, currentHeight));

    floorVertices->push_back(osg::Vec3(building.xMin, building.yMax, currentHeight));
    floorVertices->push_back(osg::Vec3(building.xMin, building.yMin, currentHeight));
    floorVertices->push_back(osg::Vec3(building.xMax, building.yMax, currentHeight));
  }

  floors->setVertexArray(floorVertices.get());
  floors->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLES, 0, floorVertices->size()));

  return floors;
}

/**
 * Build the interior walls for the rooms on the X axis.
 * Only useful if the building has more than one room
 * on the X axis (roomsX)
 *
 * @param building
 * The building we're building the interior walls for.
 * Defines the dimensions, location of the walls.
 *
 * @return
 * The constructed wall geometry
 */
osg::ref_ptr<osg::Geometry> makeWallsX(const visualization::Building &building) {
  osg::ref_ptr<osg::Geometry> walls = new osg::Geometry;

  osg::ref_ptr<osg::Vec3Array> wallVertices = new osg::Vec3Array();

  // Find the size of each room
  auto roomLength = (building.xMax - building.xMin) / building.roomsX;

  for (auto currentRoom = 1; currentRoom < building.roomsX; currentRoom++) {
    auto currentWallPosition = roomLength * currentRoom + building.xMin;
    wallVertices->push_back(osg::Vec3(currentWallPosition, building.yMin, building.zMin));
    wallVertices->push_back(osg::Vec3(currentWallPosition, building.yMax, building.zMin));
    wallVertices->push_back(osg::Vec3(currentWallPosition, building.yMax, building.zMax));

    wallVertices->push_back(osg::Vec3(currentWallPosition, building.yMin, building.zMax));
    wallVertices->push_back(osg::Vec3(currentWallPosition, building.yMin, building.zMin));
    wallVertices->push_back(osg::Vec3(currentWallPosition, building.yMax, building.zMax));
  }

  walls->setVertexArray(wallVertices.get());
  walls->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLES, 0, wallVertices->size()));

  return walls;
}

/**
 * Build the interior walls for the rooms on the y axis.
 * Only useful if the building has more than one room
 * on the y axis (roomsY)
 *
 * @param building
 * The building we're building the interior walls for.
 * Defines the dimensions, location of the walls.
 *
 * @return
 * The constructed wall geometry
 */
osg::ref_ptr<osg::Geometry> makeWallsY(const visualization::Building &building) {
  osg::ref_ptr<osg::Geometry> walls = new osg::Geometry;

  osg::ref_ptr<osg::Vec3Array> wallVertices = new osg::Vec3Array();

  // Find the size of each room
  auto roomLength = (building.yMax - building.yMin) / building.roomsY;

  for (auto currentRoom = 1; currentRoom < building.roomsY; currentRoom++) {
    auto currentWallPosition = roomLength * currentRoom + building.yMin;

    wallVertices->push_back(osg::Vec3(building.xMin, currentWallPosition, building.zMin));
    wallVertices->push_back(osg::Vec3(building.xMax, currentWallPosition, building.zMin));
    wallVertices->push_back(osg::Vec3(building.xMax, currentWallPosition, building.zMax));

    wallVertices->push_back(osg::Vec3(building.xMin, currentWallPosition, building.zMax));
    wallVertices->push_back(osg::Vec3(building.xMin, currentWallPosition, building.zMin));
    wallVertices->push_back(osg::Vec3(building.xMax, currentWallPosition, building.zMax));
  }

  walls->setVertexArray(wallVertices.get());
  walls->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLES, 0, wallVertices->size()));
  return walls;
}

/**
 * Construct and assemble the entire building geometry.
 * Setup the transparency and the render bin for the
 * interior/exterior walls/floors
 *
 * @param building
 * The building to render
 *
 * @return
 * The ready to render building geometry
 */
osg::ref_ptr<osg::Geode> makeGeode(const visualization::Building &building) {
  const int outsideRenderBin = 1;
  const int insideRenderBin = 0;

  auto outsideGeometry = makeOutsideWalls(building);

  // Set the render order, so we can guarantee the outside walls are rendered
  // after the floor (so the floor is visible from outside the building).
  // Lower numbers are rendered first

  outsideGeometry->getOrCreateStateSet()->setRenderBinDetails(outsideRenderBin, "DepthSortedBin");

  osg::ref_ptr<osg::Geode> geo = new osg::Geode;
  geo->addDrawable(outsideGeometry);

  // Floors
  if (building.floors > 1) {
    auto floorGeometry = makeFloors(building);

    // make sure this render bin number is lower than the outside walls
    floorGeometry->getOrCreateStateSet()->setRenderBinDetails(insideRenderBin, "DepthSortedBin");
    geo->addDrawable(floorGeometry);
  }

  if (building.roomsX > 1) {
    auto roomWallsX = makeWallsX(building);

    // make sure this render bin number is lower than the outside walls
    roomWallsX->getOrCreateStateSet()->setRenderBinDetails(insideRenderBin, "DepthSortedBin");
    geo->addDrawable(roomWallsX);
  }

  if (building.roomsY > 1) {
    auto roomWallsY = makeWallsY(building);

    // make sure this render bin number is lower than the outside walls
    roomWallsY->getOrCreateStateSet()->setRenderBinDetails(insideRenderBin, "DepthSortedBin");
    geo->addDrawable(roomWallsY);
  }

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
