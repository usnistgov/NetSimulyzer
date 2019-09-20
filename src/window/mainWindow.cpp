#include "mainWindow.h"
#include "osgWidget.h"
#include <QDebug>
#include <QMdiSubWindow>
#include <QMenuBar>

namespace visualization {

MainWindow::MainWindow(const GlobalConfiguration &config, osg::ref_ptr<osg::Group> root, QWidget *parent,
                       Qt::WindowFlags flags)
    : QMainWindow(parent, flags) {
  this->setCentralWidget(new OSGWidget(config, root, this));
}

MainWindow::~MainWindow() = default;

} // namespace visualization
