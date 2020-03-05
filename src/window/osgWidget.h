#pragma once

#include "../group/node/NodeGroup.h"
#include "util/PauseHandler.h"
#include <QOpenGLWidget>
#include <QPoint>
#include <QTimer>
#include <model.h>
#include <osg/ref_ptr>
#include <osgGA/GUIEventHandler>
#include <osgViewer/CompositeViewer>
#include <osgViewer/GraphicsWindow>
#include <osgViewer/Viewer>

namespace visualization {

class OSGWidget : public QOpenGLWidget {
  Q_OBJECT

public:
  explicit OSGWidget(QWidget *parent = nullptr, Qt::WindowFlags f = nullptr);
  ~OSGWidget() override;

  /**
   * Clear all elements and reset the time
   */
  void reset();

  void setConfiguration(GlobalConfiguration configuration);

  /**
   * Set the scene data for the viewer
   *
   * @param data
   * The new root node for the viewer
   *
   * @param nodes
   * Map tying NodesIds to their OSG Group
   */
  void setData(osg::ref_ptr<osg::Group> data, std::unordered_map<uint32_t, osg::ref_ptr<NodeGroup>> nodes);

  /**
   * Center the specified Node in the view.
   * If the Node with ID nodeId is not found,
   * then the camera is not changed
   *
   * @param nodeId
   * The ID of the Node to focus on
   */
  void focusNode(uint32_t nodeId);

signals:
  void timeAdvanced(double simulationTime);

protected:
  void paintEvent(QPaintEvent *paintEvent) override;
  void paintGL() override;
  void resizeGL(int width, int height) override;

  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;

  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

  bool event(QEvent *event) override;

private:
  virtual void onResize(int width, int height);

  [[nodiscard]] osgGA::EventQueue *getEventQueue() const;

  GlobalConfiguration config;
  std::unordered_map<uint32_t, osg::ref_ptr<NodeGroup>> nodeGroups;
  double currentTime = 0.0;
  QTimer *timer = new QTimer;

  osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> graphicsWindow =
      new osgViewer::GraphicsWindowEmbedded(x(), y(), width(), height());

  osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer;
  osg::ref_ptr<visualization::PauseHandler> pauseHandler = new visualization::PauseHandler;
};

} // namespace visualization
