# NIST-developed software is provided by NIST as a public service. You may use,
# copy and distribute copies of the software in any medium, provided that you
# keep intact this entire notice. You may improve,modify and create derivative
# works of the software or any portion of the software, and you may copy and
# distribute such modifications or works. Modified works should carry a notice
# stating that you changed the software and should note the date and nature of
# any such change. Please explicitly acknowledge the National Institute of
# Standards and Technology as the source of the software.
#
# NIST-developed software is expressly provided "AS IS." NIST MAKES NO
# WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF
# LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT
# AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE
# OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT
# ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY
# REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF,
# INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY,
# OR USEFULNESS OF THE SOFTWARE.
#
# You are solely responsible for determining the appropriateness of using and
# distributing the software and you assume all risks associated with its use,
# including but not limited to the risks and costs of program errors,
# compliance with applicable laws, damage to or loss of data, programs or
# equipment, and the unavailability or interruption of operation. This
# software is not intended to be used in any situation where a failure could
# cause risk of injury or damage to property. The software developed by NIST
# employees is not subject to copyright protection within the United States.
#
# Author: Evan Black <evan.black@nist.gov>

target_sources(netsimulyzer PRIVATE
        group/area/Area.h group/area/Area.cpp
        group/building/Building.h group/building/Building.cpp
        group/decoration/Decoration.h group/decoration/Decoration.cpp
        group/link/LogicalLink.h group/link/LogicalLink.cpp
        group/link/WiredLink.h group/link/WiredLink.cpp
        group/node/Node.h group/node/Node.cpp
        group/node/TrailBuffer.h group/node/TrailBuffer.cpp
        render/camera/ArcCamera.h render/camera/ArcCamera.cpp
        render/camera/Camera.h render/camera/Camera.cpp
        render/font/character.h
        render/font/undefined-medium-font.h
        render/font/FontManager.h render/font/FontManager.cpp
        render/framebuffer/PickingFramebuffer.h render/framebuffer/PickingFramebuffer.cpp
        render/helper/Floor.h render/helper/Floor.cpp
        render/Light.h
        render/material/material.h
        render/mesh/Mesh.h render/mesh/Mesh.cpp
        render/mesh/Vertex.h
        render/model/Model.h render/model/Model.cpp
        render/model/ModelCache.h render/model/ModelCache.cpp
        render/renderer/Renderer.h render/renderer/Renderer.cpp
        render/shader/Shader.h render/shader/Shader.cpp
        render/helper/CoordinateGrid.h render/helper/CoordinateGrid.cpp
        render/helper/SkyBox.h render/helper/SkyBox.cpp
        render/texture/texture.h
        render/texture/TextureCache.h render/texture/TextureCache.cpp
        settings/SettingsManager.h settings/SettingsManager.cpp
        util/common-times.h
        util/netsimulyzer-time-literals.h
        util/palette.h
        util/undo-events.h
        window/about/AboutDialog.cpp window/about/AboutDialog.h window/about/AboutDialog.ui
        window/LoadWorker.h window/LoadWorker.cpp
        window/MainWindow.cpp window/MainWindow.h window/MainWindow.ui
        window/scene/SceneWidget.h window/scene/SceneWidget.cpp
        window/settings/SettingsDialog.h window/settings/SettingsDialog.cpp window/settings/SettingsDialog.ui
        window/util/file-operations.h window/util/file-operations.cpp
        window/chart/ChartManager.cpp window/chart/ChartManager.h
        window/chart/ChartWidget.cpp window/chart/ChartWidget.h window/chart/ChartWidget.ui
        window/chart/ControlsChartView.cpp window/chart/ControlsChartView.h
        window/controls/SingleKeySequenceEdit/SingleKeySequenceEdit.h window/controls/SingleKeySequenceEdit/SingleKeySequenceEdit.cpp
        window/log/ScenarioLogWidget.h window/log/ScenarioLogWidget.cpp window/log/ScenarioLogWidget.ui
        window/node/NodeWidget.cpp window/node/NodeWidget.h window/node/NodeWidget.ui
        window/detail/DetailWidget.h window/detail/DetailWidget.cpp window/detail/DetailWidget.ui
        window/playback/PlaybackJumpDialog.cpp window/playback/PlaybackJumpDialog.h window/playback/PlaybackJumpDialog.ui
        window/playback/PlaybackTimeStepDialog.cpp window/playback/PlaybackTimeStepDialog.h window/playback/PlaybackTimeStepDialog.ui
        window/playback/PlaybackWidget.cpp window/playback/PlaybackWidget.h window/playback/PlaybackWidget.ui
        conversion.h conversion.cpp)
