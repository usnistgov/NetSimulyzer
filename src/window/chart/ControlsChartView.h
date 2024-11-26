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

#pragma once

#include "ChartManager.h"
#include <lib/QCustomPlot/qcustomplot.h>

// Pre-declared so we don't have a circular include
namespace netsimulyzer {
class ChartWidget;
}

// Unfortunately I can't have this in the
// visualizer namespace and use it with
// the Qt Creator designer

/**
 * ChartView with mouse/keyboard controls
 * to zoom & move about the graph
 */
class ControlsChartView : public QCustomPlot {
  /**
   * The amount to scale the graph when zooming
   * \see QCPAxisRect::mRangeZoomFactorHorz
   */
  const double zoomFactor = 0.85;

  /**
   * The amount to move the chart scrolling
   */
  const double scrollMagnitude = 1.0;

  netsimulyzer::ChartWidget *chartWidget{nullptr};

protected:
  void keyPressEvent(QKeyEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;
  void contextMenuEvent(QContextMenuEvent *event) override;

public:
  /**
   * Default Qt Widget constructor
   *
   * @param parent
   * The widget that contains this one
   */
  ControlsChartView(QWidget *parent);

  enum class PlotVisibility {
    Shown,
    Hidden
  };

  void setPlotPlotVisibility(PlotVisibility mode);

  void setChartWidget(netsimulyzer::ChartWidget *value);

  std::unique_ptr<QCPTextElement> title;

private:
  void exportToGnuplot();
  void exportToGnuplot(const netsimulyzer::ChartManager::XYSeriesTie &tie);
};
