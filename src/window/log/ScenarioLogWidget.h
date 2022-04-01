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
#include "../../util/undo-events.h"
#include "ui_ScenarioLogWidget.h"
#include <QColor>
#include <QString>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QWidget>
#include <deque>
#include <memory>
#include <model.h>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

namespace netsimulyzer {

class ScenarioLogWidget : public QWidget {
  Q_OBJECT
  Ui::ScenarioLogWidget ui{};
  const unsigned int unifiedStreamId = 0u;
  QTextDocument unifiedStreamDocument{this};
  QTextCursor unifiedStreamCursor{&unifiedStreamDocument};

  class LogStreamPair {
    parser::LogStream model;
    std::unique_ptr<QTextDocument> data = std::make_unique<QTextDocument>();
    std::unique_ptr<QTextCursor> cursor = std::make_unique<QTextCursor>(data.get());
    QTextCharFormat textFormat;
    QString name;

  public:
    explicit LogStreamPair(parser::LogStream model);

    // No Copies
    LogStreamPair(const LogStreamPair &other) = delete;
    LogStreamPair &operator=(const LogStreamPair &other) = delete;

    // Allow Moves
    LogStreamPair(LogStreamPair &&other) noexcept = default;
    LogStreamPair &operator=(LogStreamPair &&other) noexcept = default;

    ~LogStreamPair() = default;

    void print(const QString &value) const {
      cursor->insertText(value, textFormat);
    }

    void erase(int characters) {
      cursor->movePosition(QTextCursor::MoveOperation::Left, QTextCursor::MoveMode::KeepAnchor, characters);
      cursor->removeSelectedText();
    }

    [[nodiscard]] QTextDocument &getData() {
      return *data;
    };

    [[nodiscard]] const parser::LogStream &getModel() {
      return model;
    };

    [[nodiscard]] const QString &getName() const {
      return name;
    }

    [[nodiscard]] const QTextCharFormat &getFormat() const {
      return textFormat;
    }
  };

  unsigned int lastUnifiedWriter = 0u;
  std::unordered_map<unsigned int, LogStreamPair> streams;
  std::deque<parser::LogEvent> events;
  std::deque<undo::LogUndoEvent> undoEvents;

  void handleEvent(const parser::StreamAppendEvent &e);
  void handleEvent(const undo::StreamAppendEvent &e);
  void streamSelected(unsigned int id);
  int printToUnifiedLog(LogStreamPair &pair, const QString &value);

  void timeAdvanced(parser::nanoseconds time);
  void timeRewound(parser::nanoseconds time);

public:
  explicit ScenarioLogWidget(QWidget *parent = nullptr);

  void addStream(const parser::LogStream &stream);
  void enqueueEvents(const std::vector<parser::LogEvent> &e);
  void timeChanged(parser::nanoseconds time, parser::nanoseconds increment);
  void reset();
};

} // namespace netsimulyzer
