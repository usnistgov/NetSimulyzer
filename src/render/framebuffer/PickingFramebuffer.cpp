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

#include "PickingFramebuffer.h"
#include <QOpenGLFunctions_3_3_Core>
namespace netsimulyzer {

void PickingFramebuffer::generate(int width, int height) {
  openGl.glGenFramebuffers(1, &fbo);
  bind(GL_FRAMEBUFFER);

  openGl.glGenTextures(1, &idTexture);
  openGl.glBindTexture(GL_TEXTURE_2D, idTexture);
  openGl.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32UI, width, height, 0, GL_RGB_INTEGER, GL_UNSIGNED_INT, nullptr);
  openGl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  openGl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  openGl.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, idTexture, 0);

  openGl.glGenTextures(1, &depthTexture);
  openGl.glBindTexture(GL_TEXTURE_2D, depthTexture);
  openGl.glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  openGl.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
}

// NOLINT(cppcoreguidelines-pro-type-member-init)
PickingFramebuffer::PickingFramebuffer(QOpenGLFunctions_3_3_Core &openGl, int width, int height)
    : openGl(openGl) { // NOLINT(cppcoreguidelines-pro-type-member-init)
  generate(width, height);
}

PickingFramebuffer::~PickingFramebuffer() {
  openGl.glDeleteTextures(1, &idTexture);
  openGl.glDeleteTextures(1, &depthTexture);
  openGl.glDeleteFramebuffers(1, &fbo);
}

void PickingFramebuffer::bind(GLenum mode) const {
  openGl.glBindFramebuffer(mode, fbo);
}

void PickingFramebuffer::unbind(GLenum mode, unsigned int defaultFbo) const {
  openGl.glBindFramebuffer(mode, defaultFbo);
}

PickingFramebuffer::PixelInfo PickingFramebuffer::read(int x, int y) const {
  bind(GL_READ_FRAMEBUFFER);
  openGl.glReadBuffer(GL_COLOR_ATTACHMENT0);

  PixelInfo pixelInfo; // NOLINT(cppcoreguidelines-pro-type-member-init)
  openGl.glReadPixels(x, y, 1, 1, GL_RGB_INTEGER, GL_UNSIGNED_INT, &pixelInfo);

  return pixelInfo;
}

void PickingFramebuffer::resize(int width, int height) {
  openGl.glDeleteTextures(1, &idTexture);
  openGl.glDeleteTextures(1, &depthTexture);
  openGl.glDeleteFramebuffers(1, &fbo);
  generate(width, height);
}

} // namespace netsimulyzer
