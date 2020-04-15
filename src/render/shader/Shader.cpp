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

#include "Shader.h"

void log_uniform(int location, std::string_view name) {
#ifndef NDEBUG
  if (location == -1)
    std::cerr << "Warning Uniform '" << name << "' unused by shader\n";
#else // Maker sure the variable is 'used' in release builds
  (void)location(void) name;
#endif
}
namespace visualization {

unsigned int Shader::compile_shader(unsigned int type, const char *src) {
  const auto id = glCreateShader(type);
  glShaderSource(id, 1, &src, nullptr);
  glCompileShader(id);

  int result;
  glGetShaderiv(id, GL_COMPILE_STATUS, &result);
  if (!result) {
    int length;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    auto message = reinterpret_cast<char *>(alloca(length));
    glGetShaderInfoLog(id, length, nullptr, message);

    std::cerr << "Error, failed to compile shader type: " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << '\n'
              << "----- MESSAGE -----\n"
              << message << '\n';

    glDeleteShader(id);
    return 0;
  } else {
    std::clog << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader id: " << id
              << " compiled successfully\n";
  }

  return id;
}

unsigned int Shader::create_program(const std::string &vertex, const std::string &fragment) {
  const auto program_id = glCreateProgram();
  const auto vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex.c_str());
  const auto fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment.c_str());

  glAttachShader(program_id, vertex_shader);
  glAttachShader(program_id, fragment_shader);
  glLinkProgram(program_id);
  glValidateProgram(program_id);

  int is_valid;
  glGetProgramiv(program_id, GL_VALIDATE_STATUS, &is_valid);
  if (!is_valid) {
    int length;
    glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &length);
    auto message = reinterpret_cast<char *>(alloca(length));

    glGetProgramInfoLog(program_id, length, nullptr, message);
    std::cerr << "msg:\n" << message;

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
  } else {
    std::clog << "program: " << program_id << " compiled successfully\n";
  }

  return program_id;
}

Shader::~Shader() {
  glDeleteProgram(gl_id);
}

void Shader::init(const std::string &vertex, const std::string &fragment) {
  initializeOpenGLFunctions();
  gl_id = create_program(vertex, fragment);
}

void Shader::set_uniform_vector_3f(const std::string &name, const glm::vec3 &value) {
  bind();
  auto cached_location = uniform_cache.find(name);
  if (cached_location != uniform_cache.end()) {
    log_uniform(cached_location->second, name);
    glUniform3f(cached_location->second, value.x, value.y, value.z);
    return;
  }

  auto location = glGetUniformLocation(gl_id, name.c_str());
  log_uniform(location, name);
  uniform_cache.emplace(name, location);
  glUniform3f(location, value.x, value.y, value.z);
}

void Shader::set_uniform_vector_1f(const std::string &name, float value) {
  bind();
  auto cached_location = uniform_cache.find(name);
  if (cached_location != uniform_cache.end()) {
    log_uniform(cached_location->second, name);
    glUniform1f(cached_location->second, value);
    return;
  }

  auto location = glGetUniformLocation(gl_id, name.c_str());
  log_uniform(location, name);
  uniform_cache.emplace(name, location);
  glUniform1f(location, value);
}

void Shader::set_uniform_matrix_4fv(const std::string &name, const float *value) {
  bind();
  auto cached_location = uniform_cache.find(name);
  if (cached_location != uniform_cache.end()) {
    log_uniform(cached_location->second, name);
    glUniformMatrix4fv(cached_location->second, 1, GL_FALSE, value);
    return;
  }

  auto location = glGetUniformLocation(gl_id, name.c_str());
  log_uniform(location, name);
  uniform_cache.emplace(name, location);
  glUniformMatrix4fv(location, 1, GL_FALSE, value);
}

void Shader::set_uniform_1i(const std::string &name, int value) {
  bind();
  auto cached_location = uniform_cache.find(name);
  if (cached_location != uniform_cache.end()) {
    log_uniform(cached_location->second, name);
    glUniform1i(cached_location->second, value);
    return;
  }

  auto location = glGetUniformLocation(gl_id, name.c_str());
  log_uniform(location, name);
  uniform_cache.emplace(name, location);
  glUniform1i(location, value);
}

void Shader::set_uniform_1ui(const std::string &name, unsigned int value) {
  bind();
  auto cached_location = uniform_cache.find(name);
  if (cached_location != uniform_cache.end()) {
    log_uniform(cached_location->second, name);
    glUniform1ui(cached_location->second, value);
    return;
  }

  auto location = glGetUniformLocation(gl_id, name.c_str());
  log_uniform(location, name);
  uniform_cache.emplace(name, location);
  glUniform1ui(location, value);
}

void Shader::bind() {
  glUseProgram(gl_id);
}

void Shader::unbind() {
  glUseProgram(0u);
}

} // namespace visualization
