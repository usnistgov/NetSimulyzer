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

target_sources(visualization PRIVATE
        group/building/Building.h group/building/Building.cpp
        group/decoration/Decoration.h group/decoration/Decoration.cpp
        group/node/Node.h group/node/Node.cpp
        render/camera/Camera.h render/camera/Camera.cpp
        render/helper/Floor.h render/helper/Floor.cpp
        render/Light.h
        render/material/material.h
        render/mesh/Mesh.h render/mesh/Mesh.cpp
        render/mesh/Vertex.h
        render/model/Model.h render/model/Model.cpp
        render/model/ModelCache.h render/model/ModelCache.cpp
        render/renderer/Renderer.h render/renderer/Renderer.cpp
        render/shader/Shader.h render/shader/Shader.cpp
        render/texture/SkyBox.h render/texture/SkyBox.cpp
        render/texture/texture.h
        render/texture/TextureCache.h render/texture/TextureCache.cpp
        settings/SettingsManager.h settings/SettingsManager.cpp
        window/LoadWorker.h window/LoadWorker.cpp
        window/mainWindow.cpp window/mainWindow.h window/mainWindow.ui
        window/render/RenderWidget.h window/render/RenderWidget.cpp
        window/chart/ChartManager.cpp window/chart/ChartManager.h window/chart/chartManager.ui
        window/controls/SingleKeySequenceEdit/SingleKeySequenceEdit.h window/controls/SingleKeySequenceEdit/SingleKeySequenceEdit.cpp
        window/log/ScenarioLogWidget.h window/log/ScenarioLogWidget.cpp window/log/ScenarioLogWidget.ui
        window/camera/CameraConfigurationDialogue.h window/camera/CameraConfigurationDialogue.cpp window/camera/CameraConfigurationDialogue.ui
        window/node/NodeWidget.cpp window/node/NodeWidget.h window/node/NodeWidget.ui
        coordinate.h coordinate.cpp)
