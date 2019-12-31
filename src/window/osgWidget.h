#pragma once

#include "../parser/model.h"
#include "util/PauseHandler.h"
#include <QOpenGLWidget>
#include <QPoint>
#include <QTimer>
#include <osg/ref_ptr>
#include <osgGA/GUIEventHandler>
#include <osgViewer/CompositeViewer>
#include <osgViewer/GraphicsWindow>
#include <osgViewer/Viewer>

namespace visualization {

class OSGWidget : public QOpenGLWidget {
  Q_OBJECT

public:
  explicit OSGWidget(const GlobalConfiguration &config, osg::ref_ptr<osg::Group> root = nullptr,
                     QWidget *parent = nullptr, Qt::WindowFlags f = nullptr);
  ~OSGWidget() override;

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
  double currentTime = 0.0;
  QTimer *timer = new QTimer;

  osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> graphicsWindow =
      new osgViewer::GraphicsWindowEmbedded(x(), y(), width(), height());

  osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer;
  osg::ref_ptr<visualization::PauseHandler> pauseHandler = new visualization::PauseHandler;
};

} // namespace visualization
