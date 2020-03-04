#include "mainWindow.h"
#include "../group/building/BuildingGroup.h"
#include "../group/decoration/DecorationGroup.h"
#include "../group/node/NodeGroup.h"
#include "../parser/file-parser.h"
#include "osgWidget.h"
#include <QAction>
#include <QDebug>
#include <QFileDialog>
#include <deque>
#include <unordered_map>
#include <variant>

// Compile time checking for a member nodeId
template <typename T, typename = int> struct hasNodeId : std::false_type {}; // Default case

template <typename T>
struct hasNodeId<T, decltype((void)T::nodeId, int()) // Checking if ::nodeId exists, and its of type uint32_t. If
                                                     // it doesn't exist then this specialization is ignored (SFINAE)
                 > : std::true_type {};

template <typename T, typename = int> struct hasSeriesId : std::false_type {};
template <typename T> struct hasSeriesId<T, decltype((void)T::seriesId, int())> : std::true_type {};

template <typename T, typename = int> struct hasDecorationId : std::false_type {};
template <typename T> struct hasDecorationId<T, decltype((void)T::decorationId, int())> : std::true_type {};

namespace visualization {

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  setCentralWidget(&osg);

  charts = new ChartManager{ui->chartDock};
  ui->chartDock->setWidget(charts);

  QObject::connect(ui->chartDock, &QDockWidget::visibilityChanged, [this] (bool visible) {
    ui->actionCharts->setChecked(visible);
  });

  // For somewhat permanent messages (a message with no timeout)
  // We need to use a widget in the status bar.
  // Note: This message can still be temporarily overwritten,
  // should we choose to do so
  ui->statusbar->insertWidget(0, &statusLabel);

  QObject::connect(&osg, &OSGWidget::timeAdvanced, charts, &ChartManager::timeAdvanced);
  QObject::connect(&osg, &OSGWidget::timeAdvanced, this, &MainWindow::timeAdvanced);

  QObject::connect(ui->actionLoad, &QAction::triggered, this, &MainWindow::load);
  QObject::connect(ui->actionCharts, &QAction::triggered, this, &MainWindow::toggleCharts);
}

MainWindow::~MainWindow() {
  delete ui;
}
void MainWindow::timeAdvanced(double time) {
  statusLabel.setText(QString::number(time) + "ms");
}

void MainWindow::load() {
  auto fileName =
#ifdef __APPLE__
      QFileDialog::getOpenFileName(this, "Open File", ".", "JSON Files (*.json)");
#else
      QFileDialog::getOpenFileName(this, "Open File", ".", "JSON Files (*.json)", nullptr,
                                   QFileDialog::DontUseNativeDialog);
#endif

  if (fileName.isEmpty())
    return;

  osg.reset();

  FileParser parser;
  parser.parse(fileName.toStdString().c_str());

  osg.setConfiguration(parser.getConfiguration());

  // Nodes, Buildings, Decorations
  const auto &nodes = parser.getNodes();
  auto root = new osg::Group();

  std::unordered_map<uint32_t, osg::ref_ptr<visualization::NodeGroup>> nodeGroups;
  nodeGroups.reserve(nodes.size());
  for (auto &node : nodes) {
    auto nodeGroup = visualization::NodeGroup::MakeGroup(node);
    nodeGroups.insert({node.id, nodeGroup});
    root->addChild(nodeGroup);
  }

  const auto &buildings = parser.getBuildings();
  for (auto &building : buildings) {
    root->addChild(visualization::BuildingGroup::makeGroup(building));
  }

  const auto &decorations = parser.getDecorations();
  std::unordered_map<uint32_t, osg::ref_ptr<visualization::DecorationGroup>> decorationGroups;
  decorationGroups.reserve(decorations.size());
  for (const auto &decoration : decorations) {
    auto group = new visualization::DecorationGroup(decoration);
    decorationGroups.insert({decoration.id, group});
    root->addChild(group);
  }

  std::deque<visualization::ChartEvent> chartEvents;
  const auto &events = parser.getEvents();
  for (auto &event : events) {
    std::visit(
        [&nodeGroups, &decorationGroups, &chartEvents](auto &&arg) {
          // Strip off qualifiers, etc
          // so T holds just the type
          // so we can more easily match it
          using T = std::decay_t<decltype(arg)>;

          // Only enqueue events with nodeIds's
          if constexpr (hasNodeId<T>::value)
            nodeGroups[arg.nodeId]->enqueueEvent(arg);
          else if constexpr (hasSeriesId<T>::value)
            chartEvents.emplace_back(arg);
          else if constexpr (hasDecorationId<T>::value)
            decorationGroups[arg.decorationId]->enqueueEvent(arg);
        },
        event);
  }

  osg.setData(root);

  // Charts
  charts->reset();
  const auto &xySeries = parser.getXYSeries();
  for (const auto &series : xySeries) {
    charts->addSeries(series);
  }

  const auto &seriesCollections = parser.getSeriesCollections();
  for (const auto &series : seriesCollections) {
    charts->addSeries(series);
  }

  for (const auto &event : chartEvents) {
    charts->enqueueEvent(event);
  }

  ui->statusbar->showMessage("Successfully loaded scenario: " + fileName, 5000);
}

void MainWindow::toggleCharts() {
  ui->chartDock->toggleViewAction()->trigger();
}

} // namespace visualization
