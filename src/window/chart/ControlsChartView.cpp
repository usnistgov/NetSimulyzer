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

#include "ControlsChartView.h"
#include "src/window/chart/ChartWidget.h"
#include <QClipboard>
#include <QDateTime>
#include <QFileDialog>
#include <QGuiApplication>
#include <QMenu>
#include <QPixmap>
#include <fmt/format.h>
#include <variant>

void ControlsChartView::keyPressEvent(QKeyEvent *event) {
  const auto ctrl = event->modifiers() & Qt::KeyboardModifier::ControlModifier;
  const auto alt = event->modifiers() & Qt::KeyboardModifier::AltModifier;

  switch (event->key()) {
  case Qt::Key_Plus:
    [[fallthrough]];
  case Qt::Key_Equal: // Allow the + key next to Backspace to be used (without Shift)
    if (ctrl)         // Horizontal zoom
      xAxis->scaleRange(zoomFactor);
    else if (alt) // Vertical Zoom
      yAxis->scaleRange(zoomFactor);
    else { // Horizontal & Vertical zoom
      xAxis->scaleRange(zoomFactor);
      yAxis->scaleRange(zoomFactor);
    }
    replot();
    break;
  case Qt::Key_Minus:
    if (ctrl) // Horizontal zoom
      xAxis->scaleRange(1.0 / zoomFactor);
    else if (alt) // Vertical Zoom
      yAxis->scaleRange(1.0 / zoomFactor);
    else { // Horizontal & Vertical zoom
      xAxis->scaleRange(1.0 / zoomFactor);
      yAxis->scaleRange(1.0 / zoomFactor);
    }
    replot();
    break;
  case Qt::Key_R: {
    const auto &range = chartWidget->getTieRange();
    xAxis->setRange(range.x);
    yAxis->setRange(range.y);
    replot();
  } break;
  case Qt::Key_Left:
    xAxis->moveRange(-scrollMagnitude);
    replot();
    break;
  case Qt::Key_Right:
    xAxis->moveRange(scrollMagnitude);
    replot();
    break;
  case Qt::Key_Up:
    yAxis->moveRange(scrollMagnitude);
    replot();
    break;
  case Qt::Key_Down:
    yAxis->moveRange(-scrollMagnitude);
    replot();
    break;
  }
  QCustomPlot::keyPressEvent(event);
}

void ControlsChartView::wheelEvent(QWheelEvent *event) {
  const auto delta = event->angleDelta().y();
  if (delta == 0) {
    QCustomPlot::wheelEvent(event);
    return;
  }

  const auto horizontal = event->modifiers() & Qt::KeyboardModifier::ControlModifier;
  const auto vertical = event->modifiers() & Qt::KeyboardModifier::AltModifier;

  if (delta > 0) {
    if (horizontal)
      xAxis->scaleRange(zoomFactor);
    else if (vertical)
      yAxis->scaleRange(zoomFactor);
    else { // Horizontal & Vertical zoom
      xAxis->scaleRange(zoomFactor);
      yAxis->scaleRange(zoomFactor);
    }
    replot();
  } else { // delta < 0
    if (horizontal)
      xAxis->scaleRange(1.0 / zoomFactor);
    else if (vertical)
      yAxis->scaleRange(1.0 / zoomFactor);
    else {
      xAxis->scaleRange(1.0 / zoomFactor);
      yAxis->scaleRange(1.0 / zoomFactor);
    }
    replot();
  }
  QCustomPlot::wheelEvent(event);
}

void ControlsChartView::contextMenuEvent(QContextMenuEvent *event) {
  QMenu menu;
  menu.addAction("Save Chart Image", [this]() {
    const auto image = grab();
    const auto now = QDateTime::currentDateTime();
    const auto suggestedFilename = QString{"NetSimulyzer Chart from "} + now.toString("yyyy-MM-dd HH-mm-ss") + ".png";

    auto fileName = QFileDialog::getSaveFileName(this, "Save Chart Image", suggestedFilename, "Images (*.png *.jpeg)");
    if (fileName.isEmpty()) {
      return;
    }

    // If we don't have a filetype, add one
    if (!fileName.contains(".")) {
      fileName.append(".png");
    }

    image.save(fileName);
  });

  menu.addAction("Copy Chart Image to Clipboard", [this]() {
    auto image = grab();
    auto clipboard = QGuiApplication::clipboard();
    clipboard->setPixmap(image);
  });

  if (plottableCount() > 0) {
    menu.addAction("Export to Gnuplot", [this] {
      exportToGnuplot();
    });
  }

  menu.exec(event->globalPos());
}

void ControlsChartView::setPlotPlotVisibility(ControlsChartView::PlotVisibility mode) {
  const auto isVisible = mode == PlotVisibility::Shown;

  for (auto rc = 0; rc < plotLayout()->rowCount(); rc++) {
    plotLayout()->elementAt(rc)->setVisible(isVisible);
  }

  replot();
}

ControlsChartView::ControlsChartView(QWidget *parent)
    : QCustomPlot(parent), title(std::make_unique<QCPTextElement>(this, "")) {

  // No `iRangeZoom`, since we handle that by hand
  setInteractions(QCP::iRangeDrag);
  legend->setVisible(false);

  plotLayout()->insertRow(0);
  plotLayout()->addElement(0, 0, title.get());

  const auto legendRow = plotLayout()->rowCount();
  plotLayout()->insertRow(legendRow);
  plotLayout()->addElement(legendRow, 0, legend);
  plotLayout()->setRowStretchFactor(legendRow, 0.001);

  setPlotPlotVisibility(PlotVisibility::Hidden);
}

void ControlsChartView::setChartWidget(netsimulyzer::ChartWidget *value) {
  chartWidget = value;
}

void ControlsChartView::exportToGnuplot() {
  using namespace netsimulyzer;

  const auto &tieVariant = chartWidget->getManager().getSeries(chartWidget->getCurrentSeries());
  if (std::holds_alternative<ChartManager::XYSeriesTie>(tieVariant))
    exportToGnuplot(std::get<ChartManager::XYSeriesTie>(tieVariant));
  else if (std::holds_alternative<ChartManager::SeriesCollectionTie>(tieVariant))
    exportToGnuplot(std::get<ChartManager::SeriesCollectionTie>(tieVariant));
}

std::optional<QDir> ControlsChartView::getExportDir(const QString &title) {
  const auto exportTarget =
      QFileDialog::getExistingDirectory(this, "Export: '" + title + '\'', QCoreApplication::applicationDirPath());
  if (exportTarget.isEmpty())
    return {};
  QDir dir{exportTarget};

  // Handle if we've already exported something with the same name
  // to the same directory. Creates a name like: "Series (1)"
  // if an export named "Series" already existed
  QString targetExportTitle{title};
  auto exportDuplicateCount = 0u;
  while (dir.entryList().contains(targetExportTitle)) {
    exportDuplicateCount++;
    targetExportTitle = QString{title + " (" + QString::number(exportDuplicateCount) + ")"};
  }

  if (!dir.mkdir(targetExportTitle)) {
    QMessageBox::critical(this, "Failed to export " + title, "Failed to export " + title + '.');
    return {};
  }
  dir.cd(targetExportTitle);
  return dir;
}

void ControlsChartView::exportToGnuplot(const netsimulyzer::ChartManager::XYSeriesTie &tie) {
  const auto plotTitle = QString::fromStdString(tie.model.name);

  const auto dir = getExportDir(plotTitle);
  if (!dir)
    return;

  // Gnuplot script
  QFile gnuFile{dir->absolutePath() + '/' + plotTitle + ".gnu"};
  if (!gnuFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::critical(this, "Failed to create Gnuplot script", "Failed to create Gnuplot script.");
    return;
  }

  // Make the script executable by the user that exported it
  gnuFile.setPermissions(gnuFile.permissions() | QFileDevice::ExeUser);

  QTextStream script{&gnuFile};
  script << "#!/usr/bin/gnuplot --persist\n"; // TODO: Verify this location
  script << "set title \"" << plotTitle << "\"\n";

  QFile datFile{dir->absolutePath() + '/' + plotTitle + ".dat"};

  if (!datFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::critical(this, "failed to write data file", "failed to open " + datFile.fileName());
    return;
  }

  QTextStream stream(&datFile);
  for (const auto &[_, key, value] : *tie.data.data()) {
    stream << key << '\t' << value << '\n';
  }

  script << "set style line 1\\\n";
  script << "    linecolor rgb '" << tie.pen.color().name() << "'\\\n";
  script << "    linetype 1 linewidth 1\\\n";
  script << "    pointtype 7\n"; // See: 'test' command in Gnuplot for reference
  script << "\n";

  if (!tie.model.xAxis.name.empty())
    script << "set xlabel \"" << QString::fromStdString(tie.model.xAxis.name) << "\"\n";

  if (!tie.model.yAxis.name.empty())
    script << "set ylabel \"" << QString::fromStdString(tie.model.yAxis.name) << "\"\n";

  const auto range = chartWidget->getTieRange();
  script << "set xrange [" << range.x.lower << ':' << range.x.upper << "]\n";
  script << "set yrange [" << range.y.lower << ':' << range.y.upper << "]\n";

  QString scatterStyle;
  if (tie.model.connection == parser::XYSeries::Connection::None)
    scatterStyle = "points";
  else if (tie.model.pointMode != parser::XYSeries::PointMode::None)
    scatterStyle = "linespoints";
  else
    scatterStyle = "lines";

  script << "plot '" << plotTitle << ".dat' with " << scatterStyle << " linestyle 1";
}

void ControlsChartView::exportToGnuplot(const netsimulyzer::ChartManager::SeriesCollectionTie &tie) {
  auto &manager = chartWidget->getManager();
  const auto plotTitle = QString::fromStdString(tie.model.name);

  const auto dir = getExportDir(plotTitle);
  if (!dir)
    return;

  // Gnuplot script
  QFile gnuFile{dir->absolutePath() + '/' + plotTitle + ".gnu"};
  if (!gnuFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::critical(this, "Failed to create Gnuplot script", "Failed to create Gnuplot script.");
    return;
  }

  // Make the script executable by the user that exported it
  gnuFile.setPermissions(gnuFile.permissions() | QFileDevice::ExeUser);

  QTextStream script{&gnuFile};
  script << "#!/usr/bin/gnuplot --persist\n"; // TODO: Verify this location
  script << "set title \"" << plotTitle << "\"\n";

  std::vector<QString> plotCommands;
  plotCommands.reserve(tie.model.series.size());

  for (const auto seriesId : tie.model.series) {
    // Only XYSeries are allowed in collections
    const auto subSeries = std::get<netsimulyzer::ChartManager::XYSeriesTie>(manager.getSeries(seriesId));
    const auto subSeriesTitle = QString::fromStdString(subSeries.model.name);

    QFile datFile{dir->absolutePath() + '/' + subSeriesTitle + ".dat"};
    if (!datFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QMessageBox::critical(this, "failed to write data file", "failed to open " + datFile.fileName());
      return;
    }

    QTextStream stream(&datFile);
    for (const auto &[_, key, value] : *subSeries.data.data()) {
      stream << key << '\t' << value << '\n';
    }

    script << "set style line " << seriesId << "\\\n";
    script << "    linecolor rgb '" << subSeries.pen.color().name() << "'\\\n";
    script << "    linetype 1 linewidth 1\\\n";
    script << "    pointtype 7\n"; // See: 'test' command in Gnuplot for reference
    script << "\n";

    QString scatterStyle;
    if (subSeries.model.connection == parser::XYSeries::Connection::None)
      scatterStyle = "points";
    else if (subSeries.model.pointMode != parser::XYSeries::PointMode::None)
      scatterStyle = "linespoints";
    else
      scatterStyle = "lines";

    plotCommands.emplace_back(
        QString{"'%1.dat' with %2 linestyle %3"}.arg(subSeriesTitle).arg(scatterStyle).arg(seriesId));
  }

  if (!tie.model.xAxis.name.empty())
    script << "set xlabel \"" << QString::fromStdString(tie.model.xAxis.name) << "\"\n";

  if (!tie.model.yAxis.name.empty())
    script << "set ylabel \"" << QString::fromStdString(tie.model.yAxis.name) << "\"\n";

  const auto range = chartWidget->getTieRange();
  script << "set xrange [" << range.x.lower << ':' << range.x.upper << "]\n";
  script << "set yrange [" << range.y.lower << ':' << range.y.upper << "]\n";

  script << "plot ";
  for (const auto &plotCommand : plotCommands) {
    script << plotCommand << ",\\\n";
  }
}
