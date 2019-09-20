#pragma once

#include "../parser/model.h"
#include <QMainWindow>
#include <QMdiArea>
#include <osg/Group>
#include <osg/ref_ptr>

namespace visualization {

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(const GlobalConfiguration &config, osg::ref_ptr<osg::Group> root = new osg::Group,
                      QWidget *parent = nullptr, Qt::WindowFlags flags = nullptr);
  ~MainWindow() override;
};

} // namespace visualization
