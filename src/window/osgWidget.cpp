#include "osgWidget.h"

#include "hud/hud.h"
#include <QDebug>
#include <QKeyEvent>
#include <QPainter>
#include <osg/Camera>
#include <osg/DisplaySettings>
#include <osgGA/EventQueue>
#include <osgGA/OrbitManipulator>
#include <osgViewer/View>
#include <osgViewer/ViewerEventHandlers>
#include <stdexcept>
#include <vector>

namespace visualization {

OSGWidget::OSGWidget(const GlobalConfiguration &config, osg::ref_ptr<osg::Group> root, QWidget *parent,
                     Qt::WindowFlags f)
    : QOpenGLWidget(parent, f), config(config) {
  const auto aspectRatio = static_cast<float>(width()) / static_cast<float>(height());

  // Cheap hack to get Qt to repaint at a reasonable rate
  QObject::connect(timer, SIGNAL(timeout()), this, SLOT(update()));
  timer->start(1000 / 60);

  auto *camera = new osg::Camera;
  camera->setViewport(0, 0, width() * devicePixelRatio(), height() * devicePixelRatio());
  camera->setProjectionMatrixAsPerspective(30.f, aspectRatio, 1.0f, 1000.f);
  camera->setGraphicsContext(graphicsWindow);
  camera->setClearColor({0.2f, 0.2f, 0.4f, 1.0f});

  viewer->setCamera(camera);
  viewer->setSceneData(root);

  auto *manipulator = new osgGA::OrbitManipulator;
  // Disable throwing since the scene is only drawn on demand
  manipulator->setAllowThrow(false);

  viewer->addEventHandler(pauseHandler);

  auto viewport = camera->getViewport();
  osg::ref_ptr<visualization::HudCamera> hud = new visualization::HudCamera(viewport->width(), viewport->height());
  root->addChild(hud);
  viewer->addEventHandler(new visualization::HudResizeHandler(hud));

  viewer->setCameraManipulator(manipulator);
  viewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
  viewer->realize();

  // Make sure we get keyboard events
  setFocusPolicy(Qt::StrongFocus);
  setMinimumSize(640, 480);
}

OSGWidget::~OSGWidget() = default;

void OSGWidget::paintEvent(QPaintEvent * /* paintEvent */) {
  makeCurrent();

  QPainter painter(this);
  paintGL();
  painter.end();

  doneCurrent();
}

void OSGWidget::paintGL() {
  if (!pauseHandler->isPaused()) {
    currentTime += config.millisecondsPerFrame;
    emit timeAdvanced(currentTime);
  }

  viewer->frame(currentTime);
}

void OSGWidget::resizeGL(int width, int height) {
  getEventQueue()->windowResize(x(), y(), width, height);
  graphicsWindow->resized(x(), y(), width, height);

  onResize(width, height);
}

void OSGWidget::keyPressEvent(QKeyEvent *event) {
  auto keyString = event->text();
  const auto *keyData = keyString.toLocal8Bit().data();
  getEventQueue()->keyPress(osgGA::GUIEventAdapter::KeySymbol(*keyData));
}

void OSGWidget::keyReleaseEvent(QKeyEvent *event) {
  auto keyString = event->text();
  const auto *keyData = keyString.toLocal8Bit().data();

  getEventQueue()->keyRelease(osgGA::GUIEventAdapter::KeySymbol(*keyData));
}

void OSGWidget::mouseMoveEvent(QMouseEvent *event) {
  getEventQueue()->mouseMotion(static_cast<float>(event->x() * devicePixelRatio()),
                               static_cast<float>(event->y() * devicePixelRatio()));
}

void OSGWidget::mousePressEvent(QMouseEvent *event) {
  // 1 = left mouse button
  // 2 = middle mouse button
  // 3 = right mouse button
  auto button = 0u;
  switch (event->button()) {
  case Qt::LeftButton:
    button = 1u;
    break;
  case Qt::MiddleButton:
    button = 2u;
    break;
  case Qt::RightButton:
    button = 3u;
    break;
  default:
    break;
  }

  getEventQueue()->mouseButtonPress(static_cast<float>(event->x() * devicePixelRatio()),
                                    static_cast<float>(event->y() * devicePixelRatio()), button);
}

void OSGWidget::mouseReleaseEvent(QMouseEvent *event) {
  // 1 = left mouse button
  // 2 = middle mouse button
  // 3 = right mouse button
  auto button = 0u;
  switch (event->button()) {
  case Qt::LeftButton:
    button = 1u;
    break;
  case Qt::MiddleButton:
    button = 2u;
    break;
  case Qt::RightButton:
    button = 3u;
    break;
  default:
    break;
  }

  getEventQueue()->mouseButtonRelease(static_cast<float>(devicePixelRatio() * event->x()),
                                      static_cast<float>(devicePixelRatio() * event->y()), button);
}

void OSGWidget::wheelEvent(QWheelEvent *event) {
  event->accept();
  auto motion = event->delta() > 0 ? osgGA::GUIEventAdapter::SCROLL_UP : osgGA::GUIEventAdapter::SCROLL_DOWN;
  getEventQueue()->mouseScroll(motion);
}

bool OSGWidget::event(QEvent *event) {
  auto handled = QOpenGLWidget::event(event);

  // This ensures that the OSG widget is always going to be repainted after the
  // user performed some interaction. Doing this in the event handler ensures
  // that we don't forget about some event and prevents duplicate code.
  switch (event->type()) {
  case QEvent::KeyPress:
    [[fallthrough]];
  case QEvent::KeyRelease:
    [[fallthrough]];
  case QEvent::MouseButtonDblClick:
    [[fallthrough]];
  case QEvent::MouseButtonPress:
    [[fallthrough]];
  case QEvent::MouseButtonRelease:
    [[fallthrough]];
  case QEvent::MouseMove:
    [[fallthrough]];
  case QEvent::Wheel:
    update();
    break;
  default:
    break;
  }

  return handled;
}

void OSGWidget::onResize(int width, int height) {
  auto camera =viewer->getCamera();
  camera->setViewport(0, 0, width * devicePixelRatio(), height * devicePixelRatio());
}

osgGA::EventQueue *OSGWidget::getEventQueue() const {
  osgGA::EventQueue *eventQueue = graphicsWindow->getEventQueue();

  if (eventQueue)
    return eventQueue;
  else
    throw std::runtime_error("Unable to obtain valid event queue");
}

} // namespace visualization
