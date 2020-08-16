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

#include "ScenarioLogWidget.h"
#include "../../conversion.h"
#include "ui_ScenarioLogWidget.h"
#include <QColor>
#include <QString>
#include <variant>

namespace visualization {

ScenarioLogWidget::LogStreamPair::LogStreamPair(parser::LogStream model) : model(std::move(model)) {
  data->setDocumentLayout(new QPlainTextDocumentLayout(data.get()));
  if (this->model.color) {
    const auto &color = *this->model.color;
    textFormat.setForeground(QColor{color.red, color.green, color.blue, 255});
  }

  name = QString::fromStdString(this->model.name);
}

void ScenarioLogWidget::handleEvent(const parser::StreamAppendEvent &e) {
  const auto &iter = streams.find(e.streamId);
  if (iter == streams.end())
    return;

  undo::StreamAppendEvent undo;
  undo.event = e;
  undo.lastUnifiedWriter = lastUnifiedWriter;

  auto &pair = iter->second;
  auto value = QString::fromStdString(e.value);
  pair.print(value);

  undo.unifiedLogEraseCount = printToUnifiedLog(pair, value);

  undoEvents.emplace_back(undo);

  // Scroll the document to the bottom (where the cursor is now)
  // after every append, keeping the newest info visible
  // TODO: Should be a setting "autoscroll logs" maybe?
  ui.plainTextLog->ensureCursorVisible();
}

void ScenarioLogWidget::handleEvent(const undo::StreamAppendEvent &e) {
  const auto &iter = streams.find(e.event.streamId);
  if (iter == streams.end())
    return;

  auto &pair = iter->second;
  // TODO: Maybe check this cast?
  pair.erase(static_cast<int>(e.event.value.size()));

  unifiedStreamCursor.movePosition(QTextCursor::MoveOperation::Left, QTextCursor::MoveMode::KeepAnchor,
                                   e.unifiedLogEraseCount);
  unifiedStreamCursor.removeSelectedText();
  lastUnifiedWriter = e.lastUnifiedWriter;

  events.emplace_front(e.event);
}

int ScenarioLogWidget::printToUnifiedLog(LogStreamPair &pair, const QString &value) {
  // File us down to single line prints
  if (value.count('\n') > 1) {
    auto lines = value.split('\n');
    int accumulator = 0;
    for (const auto &line : lines)
      accumulator += printToUnifiedLog(pair, line + '\n');

    return accumulator;
  }

  int characterTotal = 0;

  const auto id = pair.getModel().id;
  const auto &format = pair.getFormat();

  if (id != lastUnifiedWriter && !unifiedStreamCursor.atBlockStart()) {
    unifiedStreamCursor.insertText("\n", format);
    characterTotal++;
  }

  if (unifiedStreamCursor.atBlockStart()) {
    const QString prompt = '[' + pair.getName() + "]: ";
    unifiedStreamCursor.insertText(prompt, format);
    characterTotal += prompt.size();
  }

  unifiedStreamCursor.insertText(value);
  characterTotal += value.size();

  lastUnifiedWriter = id;

  return characterTotal;
}

void ScenarioLogWidget::streamSelected(unsigned int id) {
  if (id == unifiedStreamId) {
    ui.plainTextLog->setDocument(&unifiedStreamDocument);

    // When changing the document, the cursor seems to get stuck
    // at the top, so move it back to the end
    ui.plainTextLog->moveCursor(QTextCursor::End);

    // Scroll the document to the bottom (where the cursor is now)
    ui.plainTextLog->ensureCursorVisible();
    return;
  }

  const auto &iter = streams.find(id);
  if (iter == streams.end())
    return;

  ui.plainTextLog->setDocument(&iter->second.getData());

  // When changing the document, the cursor seems to get stuck
  // at the top, so move it back to the end
  ui.plainTextLog->moveCursor(QTextCursor::End);

  // Scroll the document to the bottom (where the cursor is now)
  ui.plainTextLog->ensureCursorVisible();
}

ScenarioLogWidget::ScenarioLogWidget(QWidget *parent) : QWidget(parent) {
  ui.setupUi(this);
  unifiedStreamDocument.setDocumentLayout(new QPlainTextDocumentLayout(&unifiedStreamDocument));

  reset();

  QObject::connect(ui.comboBoxLogName, qOverload<int>(&QComboBox::currentIndexChanged),
                   [this](int index) { streamSelected(ui.comboBoxLogName->itemData(index).toUInt()); });
}

void ScenarioLogWidget::addStream(const parser::LogStream &stream) {
  streams.try_emplace(stream.id, stream);

  if (stream.visible)
    ui.comboBoxLogName->addItem(QString::fromStdString(stream.name), stream.id);
}

void ScenarioLogWidget::enqueueEvents(const std::vector<parser::LogEvent> &e) {
  events.insert(events.end(), e.begin(), e.end());
}
void ScenarioLogWidget::timeAdvanced(double time) {
  auto handle = [this, time](auto &&e) -> bool {
    if (time < e.time)
      return false;

    handleEvent(e);
    events.pop_front();
    return true;
  };

  while (!events.empty() && std::visit(handle, events.front())) {
    // Intentionally Blank
  }
}

void ScenarioLogWidget::timeRewound(double time) {
  auto handleUndoEvent = [time, this](auto &&e) -> bool {
    // All events have a time
    // Make sure we don't handle one
    // Before it was originally applied
    if (time > e.event.time)
      return false;

    handleEvent(e);
    return true;
  };

  while (!undoEvents.empty() && std::visit(handleUndoEvent, undoEvents.back())) {
    undoEvents.pop_back();
  }
}

void ScenarioLogWidget::reset() {
  unifiedStreamDocument.clear();
  ui.plainTextLog->setDocument(&unifiedStreamDocument);
  ui.comboBoxLogName->clear();
  streams.clear();
  ui.comboBoxLogName->addItem("Unified Log", unifiedStreamId);
}

} // namespace visualization
