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

/*
 * This is character data produced by Hiero (https://libgdx.com/wiki/tools/hiero)
 * and heavily modified for the NetSimulyzer.
 *
 * The font is 'undefined medium' (https://github.com/andirueckel/undefined-medium)
 * licenced under the SIL OPEN FONT LICENSE Version 1.1. The font itself is
 * unmodified.
 */

#pragma once

#include "src/render/font/character.h"
#include <unordered_map>

namespace netsimulyzer::undefined_medium {

// clang-format off
const std::unordered_map<char, Character> fontGlyphs {
     {0, Character{0, 322, 54, {30, 46}, {-5, -5}, 32}},
     {10, Character{10, 322, 54, {30, 46}, {-5, -5}, 32}},
     {32, Character{32, 0, 0, {0, 0}, {0, 0}, 32}},
     {33, Character{33, 210, 268, {14, 38}, {3, 0}, 32}},
     {34, Character{34, 418, 382, {22, 22}, {-1, 16}, 32}},
     {35, Character{35, 52, 382, {30, 30}, {-5, 3}, 32}},
     {36, Character{36, 14, 104, {30, 46}, {-5, -5}, 32}},
     {37, Character{37, 302, 268, {30, 38}, {-5, -1}, 32}},
     {38, Character{38, 332, 268, {30, 38}, {-5, -1}, 32}},
     {39, Character{39, 496, 268, {14, 22}, {3, 16}, 32}},
     {40, Character{40, 374, 54, {22, 46}, {-1, -5}, 32}},
     {41, Character{41, 396, 54, {22, 46}, {-1, -5}, 32}},
     {42, Character{42, 82, 382, {30, 30}, {-5, 3}, 32}},
     {43, Character{43, 22, 382, {30, 30}, {-5, 3}, 32}},
     {44, Character{44, 490, 344, {18, 26}, {-1, -12}, 32}},
     {45, Character{45, 134, 412, {22, 14}, {-1, 11}, 32}},
     {46, Character{46, 492, 382, {18, 18}, {-1, -3}, 32}},
     {47, Character{47, 30, 0, {30, 50}, {-5, -5}, 32}},
     {48, Character{48, 180, 268, {30, 38}, {-5, -2}, 32}},
     {49, Character{49, 422, 230, {22, 38}, {-1, -2}, 32}},
     {50, Character{50, 444, 230, {30, 38}, {-5, -2}, 32}},
     {51, Character{51, 474, 230, {30, 38}, {-5, -2}, 32}},
     {52, Character{52, 0, 268, {30, 38}, {-5, -2}, 32}},
     {53, Character{53, 30, 268, {30, 38}, {-5, -2}, 32}},
     {54, Character{54, 60, 268, {30, 38}, {-5, -2}, 32}},
     {55, Character{55, 90, 268, {30, 38}, {-5, -2}, 32}},
     {56, Character{56, 120, 268, {30, 38}, {-5, -2}, 32}},
     {57, Character{57, 150, 268, {30, 38}, {-5, -2}, 32}},
     {58, Character{58, 450, 344, {18, 30}, {-1, 7}, 32}},
     {59, Character{59, 254, 268, {18, 30}, {-1, 7}, 32}},
     {60, Character{60, 468, 344, {22, 30}, {-1, 3}, 32}},
     {61, Character{61, 0, 412, {30, 22}, {-5, 7}, 32}},
     {62, Character{62, 0, 382, {22, 30}, {-1, 3}, 32}},
     {63, Character{63, 224, 268, {30, 38}, {-5, -2}, 32}},
     {64, Character{64, 272, 268, {30, 38}, {-5, -2}, 32}},
     {65, Character{65, 366, 150, {30, 38}, {-5, -2}, 32}},
     {66, Character{66, 396, 150, {30, 38}, {-5, -2}, 32}},
     {67, Character{67, 426, 150, {30, 38}, {-5, -2}, 32}},
     {68, Character{68, 456, 150, {30, 38}, {-5, -2}, 32}},
     {69, Character{69, 0, 192, {30, 38}, {-5, -2}, 32}},
     {70, Character{70, 30, 192, {30, 38}, {-5, -2}, 32}},
     {71, Character{71, 60, 192, {30, 38}, {-5, -2}, 32}},
     {72, Character{72, 90, 192, {30, 38}, {-5, -2}, 32}},
     {73, Character{73, 486, 150, {22, 38}, {-1, -2}, 32}},
     {74, Character{74, 120, 192, {30, 38}, {-5, -2}, 32}},
     {75, Character{75, 150, 192, {30, 38}, {-5, -2}, 32}},
     {76, Character{76, 180, 192, {30, 38}, {-5, -2}, 32}},
     {77, Character{77, 210, 192, {30, 38}, {-5, -2}, 32}},
     {78, Character{78, 240, 192, {30, 38}, {-5, -2}, 32}},
     {79, Character{79, 270, 192, {30, 38}, {-5, -2}, 32}},
     {80, Character{80, 300, 192, {30, 38}, {-5, -2}, 32}},
     {81, Character{81, 320, 104, {30, 42}, {-5, -5}, 32}},
     {82, Character{82, 330, 192, {30, 38}, {-5, -2}, 32}},
     {83, Character{83, 360, 192, {30, 38}, {-5, -2}, 32}},
     {84, Character{84, 390, 192, {30, 38}, {-5, -2}, 32}},
     {85, Character{85, 420, 192, {30, 38}, {-5, -2}, 32}},
     {86, Character{86, 450, 192, {30, 38}, {-5, -2}, 32}},
     {87, Character{87, 480, 192, {30, 38}, {-5, -2}, 32}},
     {88, Character{88, 0, 230, {30, 38}, {-5, -2}, 32}},
     {89, Character{89, 30, 230, {30, 38}, {-5, -2}, 32}},
     {90, Character{90, 60, 230, {30, 38}, {-5, -2}, 32}},
     {91, Character{91, 418, 54, {18, 46}, {3, -5}, 32}},
     {92, Character{92, 60, 0, {30, 50}, {-5, -5}, 32}},
     {93, Character{93, 436, 54, {18, 46}, {-1, -5}, 32}},
     {94, Character{94, 462, 382, {30, 22}, {-5, 16}, 32}},
     {95, Character{95, 156, 412, {30, 14}, {-5, -1}, 32}},
     {96, Character{96, 440, 382, {22, 22}, {-1, 16}, 32}},
     {97, Character{97, 60, 344, {30, 30}, {-5, -2}, 32}},
     {98, Character{98, 90, 230, {30, 38}, {-5, -2}, 32}},
     {99, Character{99, 90, 344, {30, 30}, {-5, -2}, 32}},
     {100, Character{100, 120, 230, {30, 38}, {-5, -2}, 32}},
     {101, Character{101, 120, 344, {30, 30}, {-5, -2}, 32}},
     {102, Character{102, 150, 230, {22, 38}, {-1, -2}, 32}},
     {103, Character{103, 172, 230, {30, 38}, {-5, -10}, 32}},
     {104, Character{104, 202, 230, {30, 38}, {-5, -2}, 32}},
     {105, Character{105, 232, 230, {22, 38}, {-1, -2}, 32}},
     {106, Character{106, 352, 54, {22, 46}, {-5, -10}, 32}},
     {107, Character{107, 254, 230, {30, 38}, {-5, -2}, 32}},
     {108, Character{108, 284, 230, {22, 38}, {-1, -2}, 32}},
     {109, Character{109, 150, 344, {30, 30}, {-5, -2}, 32}},
     {110, Character{110, 180, 344, {30, 30}, {-5, -2}, 32}},
     {111, Character{111, 210, 344, {30, 30}, {-5, -2}, 32}},
     {112, Character{112, 306, 230, {30, 38}, {-5, -10}, 32}},
     {113, Character{113, 336, 230, {30, 38}, {-5, -10}, 32}},
     {114, Character{114, 240, 344, {30, 30}, {-5, -2}, 32}},
     {115, Character{115, 270, 344, {30, 30}, {-5, -2}, 32}},
     {116, Character{116, 366, 230, {26, 38}, {-5, -2}, 32}},
     {117, Character{117, 300, 344, {30, 30}, {-5, -2}, 32}},
     {118, Character{118, 330, 344, {30, 30}, {-5, -2}, 32}},
     {119, Character{119, 360, 344, {30, 30}, {-5, -2}, 32}},
     {120, Character{120, 390, 344, {30, 30}, {-5, -2}, 32}},
     {121, Character{121, 392, 230, {30, 38}, {-5, -10}, 32}},
     {122, Character{122, 420, 344, {30, 30}, {-5, -2}, 32}},
     {123, Character{123, 454, 54, {22, 46}, {-1, -5}, 32}},
     {124, Character{124, 0, 104, {14, 46}, {3, -5}, 32}},
     {125, Character{125, 476, 54, {22, 46}, {-1, -5}, 32}},
     {126, Character{126, 74, 412, {30, 18}, {-5, 11}, 32}},
     {160, Character{160, 0, 0, {0, 0}, {-5, 27}, 32}},
     {161, Character{161, 362, 268, {14, 38}, {3, -1}, 32}},
     {162, Character{162, 376, 268, {30, 38}, {-5, -1}, 32}},
     {163, Character{163, 406, 268, {30, 38}, {-5, -1}, 32}},
     {164, Character{164, 112, 382, {30, 30}, {-5, 3}, 32}},
     {165, Character{165, 436, 268, {30, 38}, {-5, -1}, 32}},
     {166, Character{166, 44, 104, {14, 46}, {3, -5}, 32}},
     {167, Character{167, 466, 268, {30, 38}, {-5, -1}, 32}},
     {168, Character{168, 186, 412, {22, 14}, {-1, -1}, 32}},
     {169, Character{169, 0, 306, {34, 38}, {-5, -1}, 32}},
     {170, Character{170, 366, 382, {26, 26}, {-5, -5}, 32}},
     {171, Character{171, 142, 382, {30, 30}, {-5, 3}, 32}},
     {172, Character{172, 104, 412, {30, 18}, {-5, 11}, 32}},
     {173, Character{173, 0, 0, {0, 0}, {-5, 27}, 32}},
     {174, Character{174, 34, 306, {34, 38}, {-5, -1}, 32}},
     {175, Character{175, 156, 412, {30, 14}, {-5, -1}, 32}},
     {176, Character{176, 392, 382, {26, 26}, {-5, -5}, 32}},
     {177, Character{177, 68, 306, {30, 38}, {-5, -1}, 32}},
     {178, Character{178, 172, 382, {26, 30}, {-5, -9}, 32}},
     {179, Character{179, 198, 382, {26, 30}, {-5, -9}, 32}},
     {180, Character{180, 30, 412, {22, 22}, {-1, -5}, 32}},
     {181, Character{181, 98, 306, {30, 38}, {-5, 7}, 32}},
     {182, Character{182, 128, 306, {30, 38}, {-5, -1}, 32}},
     {183, Character{183, 492, 382, {18, 18}, {-1, 7}, 32}},
     {184, Character{184, 52, 412, {22, 22}, {-1, 23}, 32}},
     {185, Character{185, 224, 382, {22, 30}, {-5, -9}, 32}},
     {186, Character{186, 392, 382, {26, 26}, {-5, -5}, 32}},
     {187, Character{187, 246, 382, {30, 30}, {-5, 3}, 32}},
     {188, Character{188, 90, 0, {54, 50}, {-5, -5}, 56}},
     {189, Character{189, 144, 0, {54, 50}, {-5, -5}, 56}},
     {190, Character{190, 198, 0, {54, 50}, {-5, -5}, 56}},
     {191, Character{191, 158, 306, {30, 38}, {-5, -1}, 32}},
     {192, Character{192, 252, 0, {30, 50}, {-5, -13}, 32}},
     {193, Character{193, 282, 0, {30, 50}, {-5, -13}, 32}},
     {194, Character{194, 312, 0, {30, 50}, {-5, -13}, 32}},
     {195, Character{195, 342, 0, {30, 50}, {-5, -13}, 32}},
     {196, Character{196, 58, 104, {30, 46}, {-5, -9}, 32}},
     {197, Character{197, 0, 0, {30, 54}, {-5, -17}, 32}},
     {198, Character{198, 188, 306, {30, 38}, {-5, -1}, 32}},
     {199, Character{199, 88, 104, {30, 46}, {-5, -1}, 32}},
     {200, Character{200, 372, 0, {30, 50}, {-5, -13}, 32}},
     {201, Character{201, 402, 0, {30, 50}, {-5, -13}, 32}},
     {202, Character{202, 432, 0, {30, 50}, {-5, -13}, 32}},
     {203, Character{203, 118, 104, {30, 46}, {-5, -9}, 32}},
     {204, Character{204, 462, 0, {22, 50}, {-1, -13}, 32}},
     {205, Character{205, 484, 0, {22, 50}, {-1, -13}, 32}},
     {206, Character{206, 0, 54, {22, 50}, {-1, -13}, 32}},
     {207, Character{207, 148, 104, {22, 46}, {-1, -9}, 32}},
     {208, Character{208, 218, 306, {34, 38}, {-9, -1}, 32}},
     {209, Character{209, 22, 54, {30, 50}, {-5, -13}, 32}},
     {210, Character{210, 52, 54, {30, 50}, {-5, -13}, 32}},
     {211, Character{211, 82, 54, {30, 50}, {-5, -13}, 32}},
     {212, Character{212, 112, 54, {30, 50}, {-5, -13}, 32}},
     {213, Character{213, 142, 54, {30, 50}, {-5, -13}, 32}},
     {214, Character{214, 170, 104, {30, 46}, {-5, -9}, 32}},
     {215, Character{215, 390, 344, {30, 30}, {-5, 3}, 32}},
     {216, Character{216, 252, 306, {38, 38}, {-9, -1}, 32}},
     {217, Character{217, 172, 54, {30, 50}, {-5, -13}, 32}},
     {218, Character{218, 202, 54, {30, 50}, {-5, -13}, 32}},
     {219, Character{219, 232, 54, {30, 50}, {-5, -13}, 32}},
     {220, Character{220, 200, 104, {30, 46}, {-5, -9}, 32}},
     {221, Character{221, 262, 54, {30, 50}, {-5, -13}, 32}},
     {222, Character{222, 290, 306, {30, 38}, {-5, -1}, 32}},
     {223, Character{223, 320, 306, {30, 38}, {-5, -1}, 32}},
     {224, Character{224, 350, 104, {30, 42}, {-5, -5}, 32}},
     {225, Character{225, 380, 104, {30, 42}, {-5, -5}, 32}},
     {226, Character{226, 410, 104, {30, 42}, {-5, -5}, 32}},
     {227, Character{227, 440, 104, {30, 42}, {-5, -5}, 32}},
     {228, Character{228, 350, 306, {30, 38}, {-5, -1}, 32}},
     {229, Character{229, 230, 104, {30, 46}, {-5, -9}, 32}},
     {230, Character{230, 276, 382, {30, 30}, {-5, 7}, 32}},
     {231, Character{231, 380, 306, {30, 38}, {-5, 7}, 32}},
     {232, Character{232, 470, 104, {30, 42}, {-5, -5}, 32}},
     {233, Character{233, 0, 150, {30, 42}, {-5, -5}, 32}},
     {234, Character{234, 30, 150, {30, 42}, {-5, -5}, 32}},
     {235, Character{235, 410, 306, {30, 38}, {-5, -1}, 32}},
     {236, Character{236, 60, 150, {22, 42}, {-1, -5}, 32}},
     {237, Character{237, 82, 150, {22, 42}, {-1, -5}, 32}},
     {238, Character{238, 104, 150, {22, 42}, {-1, -5}, 32}},
     {239, Character{239, 440, 306, {22, 38}, {-1, -1}, 32}},
     {240, Character{240, 462, 306, {34, 38}, {-5, -1}, 32}},
     {241, Character{241, 126, 150, {30, 42}, {-5, -5}, 32}},
     {242, Character{242, 156, 150, {30, 42}, {-5, -5}, 32}},
     {243, Character{243, 186, 150, {30, 42}, {-5, -5}, 32}},
     {244, Character{244, 216, 150, {30, 42}, {-5, -5}, 32}},
     {245, Character{245, 246, 150, {30, 42}, {-5, -5}, 32}},
     {246, Character{246, 0, 344, {30, 38}, {-5, -1}, 32}},
     {247, Character{247, 306, 382, {30, 30}, {-5, 3}, 32}},
     {248, Character{248, 336, 382, {30, 30}, {-5, 7}, 32}},
     {249, Character{249, 276, 150, {30, 42}, {-5, -5}, 32}},
     {250, Character{250, 306, 150, {30, 42}, {-5, -5}, 32}},
     {251, Character{251, 336, 150, {30, 42}, {-5, -5}, 32}},
     {252, Character{252, 30, 344, {30, 38}, {-5, -1}, 32}},
     {253, Character{253, 292, 54, {30, 50}, {-5, -5}, 32}},
     {254, Character{254, 260, 104, {30, 46}, {-5, -1}, 32}},
     {255, Character{255, 290, 104, {30, 46}, {-5, -1}, 32}},
};
// clang-format on

} // namespace netsimulyzer::undefined_medium