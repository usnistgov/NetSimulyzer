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
#include <QColor>

namespace netsimulyzer::palette {

[[maybe_unused]] const QColor Red{245, 61, 0};
[[maybe_unused]] const QColor DarkRed{204, 51, 0};

[[maybe_unused]] const QColor Green{9, 232, 94};
[[maybe_unused]] const QColor DarkGreen{6, 177, 72};

[[maybe_unused]] const QColor Blue{37, 137, 189};
[[maybe_unused]] const QColor DarkBlue{27, 98, 136};

[[maybe_unused]] const QColor Orange{255, 167, 51};
[[maybe_unused]] const QColor DarkOrange{245, 139, 0};

[[maybe_unused]] const QColor Yellow{255, 227, 71};
[[maybe_unused]] const QColor DarkYellow{255, 218, 10};

[[maybe_unused]] const QColor Purple{120, 41, 163};
[[maybe_unused]] const QColor DarkPurple{84, 29, 114};

[[maybe_unused]] const QColor Pink{255, 92, 176};
[[maybe_unused]] const QColor DarkPink{255, 51, 156};

[[maybe_unused]] const QColor Black{7, 6, 0};
[[maybe_unused]] const QColor White{255, 255, 255};

[[maybe_unused]] const QColor Gray90{26, 26, 26};
[[maybe_unused]] const QColor Gray80{51, 51, 51};
[[maybe_unused]] const QColor Gray70{77, 77, 77};
[[maybe_unused]] const QColor Gray60{102, 102, 102};
[[maybe_unused]] const QColor Gray50{128, 128, 128};
[[maybe_unused]] const QColor Gray40{153, 153, 153};
[[maybe_unused]] const QColor Gray30{179, 179, 179};
[[maybe_unused]] const QColor Gray20{204, 204, 204};
[[maybe_unused]] const QColor Gray10{230, 230, 230};

} // namespace netsimulyzer::palette
