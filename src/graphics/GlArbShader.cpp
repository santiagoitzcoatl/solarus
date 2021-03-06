/*
 * Copyright (C) 2006-2018 Christopho, Solarus - http://www.solarus-games.org
 *
 * Solarus is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Solarus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "solarus/core/Logger.h"
#include "solarus/core/QuestFiles.h"
#include "solarus/core/Size.h"
#include "solarus/core/System.h"
#include "solarus/graphics/GlArbShader.h"
#include "solarus/graphics/ShaderData.h"
#include "solarus/graphics/Surface.h"
#include "solarus/graphics/Video.h"
#include <sstream>

#ifdef SOLARUS_HAVE_OPENGL

namespace Solarus {

namespace {

PFNGLATTACHOBJECTARBPROC glAttachObjectARB;
PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
PFNGLCOMPILESHADERARBPROC glCompileShaderARB;
PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB;
PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB;
PFNGLDELETEOBJECTARBPROC glDeleteObjectARB;
PFNGLGETINFOLOGARBPROC glGetInfoLogARB;
PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB;
PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB;
PFNGLLINKPROGRAMARBPROC glLinkProgramARB;
PFNGLSHADERSOURCEARBPROC glShaderSourceARB;
PFNGLUNIFORM1IARBPROC glUniform1iARB;
PFNGLUNIFORM1FARBPROC glUniform1fARB;
PFNGLUNIFORM2FARBPROC glUniform2fARB;
PFNGLUNIFORM3FARBPROC glUniform3fARB;
PFNGLUNIFORM4FARBPROC glUniform4fARB;
PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB;
PFNGLGETHANDLEARBPROC glGetHandleARB;

/**
 * \brief Casts a pointer-to-object (void*) to a pointer-to-function.
 *
 * This function avoids compilation warnings when using SDL_GL_GetProcAddress(),
 * because directly casting a pointer-to-object to a pointer-to-function
 * is not allowed.
 *
 * \param object_ptr The pointer to cast.
 * \return The pointer to a function.
 */
template<typename FunctionPointerType>
FunctionPointerType get_proc_address_cast(void* object_ptr) {
  return *reinterpret_cast<FunctionPointerType*>(&object_ptr);
}

}  // Anonymous namespace

/**
 * \brief Initializes the GL ARB shader system.
 * \return \c true if GL ARB shaders are supported.
 */
bool GlArbShader::initialize() {

  // TODO Make sure that SDL doesn't disable GL ARB for render targets even if available.

  // Setting some parameters.
  glClearDepth(1.0); // Enables clearing of the depth buffer.
  glShadeModel(GL_SMOOTH); // Enables smooth color shading.

  // Initialize GL ARB functions.
  if (SDL_GL_ExtensionSupported("GL_ARB_shader_objects") &&
      SDL_GL_ExtensionSupported("GL_ARB_shading_language_100") &&
      SDL_GL_ExtensionSupported("GL_ARB_vertex_shader") &&
      SDL_GL_ExtensionSupported("GL_ARB_fragment_shader") &&
      SDL_GL_ExtensionSupported("GL_ARB_multitexture")) {
    glAttachObjectARB = get_proc_address_cast<PFNGLATTACHOBJECTARBPROC>(SDL_GL_GetProcAddress("glAttachObjectARB"));
    glActiveTextureARB = get_proc_address_cast<PFNGLACTIVETEXTUREARBPROC>(SDL_GL_GetProcAddress("glActiveTextureARB"));
    glCompileShaderARB = get_proc_address_cast<PFNGLCOMPILESHADERARBPROC>(SDL_GL_GetProcAddress("glCompileShaderARB"));
    glCreateProgramObjectARB = get_proc_address_cast<PFNGLCREATEPROGRAMOBJECTARBPROC>(SDL_GL_GetProcAddress("glCreateProgramObjectARB"));
    glCreateShaderObjectARB = get_proc_address_cast<PFNGLCREATESHADEROBJECTARBPROC>(SDL_GL_GetProcAddress("glCreateShaderObjectARB"));
    glDeleteObjectARB = get_proc_address_cast<PFNGLDELETEOBJECTARBPROC>(SDL_GL_GetProcAddress("glDeleteObjectARB"));
    glGetInfoLogARB = get_proc_address_cast<PFNGLGETINFOLOGARBPROC>(SDL_GL_GetProcAddress("glGetInfoLogARB"));
    glGetObjectParameterivARB = get_proc_address_cast<PFNGLGETOBJECTPARAMETERIVARBPROC>(SDL_GL_GetProcAddress("glGetObjectParameterivARB"));
    glGetUniformLocationARB = get_proc_address_cast<PFNGLGETUNIFORMLOCATIONARBPROC>(SDL_GL_GetProcAddress("glGetUniformLocationARB"));
    glLinkProgramARB = get_proc_address_cast<PFNGLLINKPROGRAMARBPROC>(SDL_GL_GetProcAddress("glLinkProgramARB"));
    glShaderSourceARB = get_proc_address_cast<PFNGLSHADERSOURCEARBPROC>(SDL_GL_GetProcAddress("glShaderSourceARB"));
    glUniform1iARB = get_proc_address_cast<PFNGLUNIFORM1IARBPROC>(SDL_GL_GetProcAddress("glUniform1iARB"));
    glUniform1fARB = get_proc_address_cast<PFNGLUNIFORM1FARBPROC>(SDL_GL_GetProcAddress("glUniform1fARB"));
    glUniform2fARB = get_proc_address_cast<PFNGLUNIFORM2FARBPROC>(SDL_GL_GetProcAddress("glUniform2fARB"));
    glUniform3fARB = get_proc_address_cast<PFNGLUNIFORM3FARBPROC>(SDL_GL_GetProcAddress("glUniform3fARB"));
    glUniform4fARB = get_proc_address_cast<PFNGLUNIFORM4FARBPROC>(SDL_GL_GetProcAddress("glUniform4fARB"));
    glUseProgramObjectARB = get_proc_address_cast<PFNGLUSEPROGRAMOBJECTARBPROC>(SDL_GL_GetProcAddress("glUseProgramObjectARB"));
    glGetHandleARB = get_proc_address_cast<PFNGLGETHANDLEARBPROC>(SDL_GL_GetProcAddress("glGetHandleARB"));
    if (glAttachObjectARB &&
        glActiveTextureARB &&
        glCompileShaderARB &&
        glCreateProgramObjectARB &&
        glCreateShaderObjectARB &&
        glDeleteObjectARB &&
        glGetInfoLogARB &&
        glGetObjectParameterivARB &&
        glGetUniformLocationARB &&
        glLinkProgramARB &&
        glShaderSourceARB &&
        glUniform1iARB &&
        glUniform1fARB &&
        glUniform2fARB &&
        glUniform3fARB &&
        glUniform4fARB &&
        glUseProgramObjectARB &&
        glGetHandleARB) {

      return true;
    }
  }

  return false;
}

/**
 * \brief Constructor.
 * \param shader_id Id of the shader to load.
 */
GlArbShader::GlArbShader(const std::string& shader_id):
    Shader(shader_id),
    program(0),
    vertex_shader(0),
    fragment_shader(0) {

  glGetError();

  // Load the shader.
  load();

  // Set up constant uniform variables.
  GLenum gl_error = glGetError();
  GLhandleARB previous_program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
  glUseProgramObjectARB(program);

  // TODO use our uniform functions
  GLint location = get_uniform_location("sol_texture");
  if (location >= 0) {
    glUniform1iARB(location, 0);  // Use texture unit 0 for the quest surface.
  }

  const Size& quest_size = Video::get_quest_size();
  set_uniform_2f("sol_input_size", quest_size.width, quest_size.height);

  glUseProgramObjectARB(previous_program);

  if (gl_error != GL_NO_ERROR) {
    std::ostringstream oss;
    oss << "OpenGL error: " << gl_error;
    set_error(oss.str());
    set_valid(false);
  }
}

/**
 * \brief Destructor.
 */
GlArbShader::~GlArbShader() {

  // TODO use glDeleteProgramARB?
  glDeleteObjectARB(vertex_shader);
  glDeleteObjectARB(fragment_shader);
  glDeleteObjectARB(program);
}

/**
 * \copydoc Shader::load
 */
void GlArbShader::load() {

  set_valid(true);

  // Load the shader data file.
  const std::string shader_file_name =
      "shaders/" + get_id() + ".dat";

  ShaderData data;
  bool success = data.import_from_quest_file(shader_file_name);
  if (!success) {
    set_valid(false);
    return;
  }
  set_data(data);
  uniform_locations.clear();

  // Create the vertex and fragment shaders.
  vertex_shader = create_shader(GL_VERTEX_SHADER_ARB, data.get_vertex_source().c_str());
  if (!is_valid()) {
    return;
  }
  fragment_shader = create_shader(GL_FRAGMENT_SHADER_ARB, data.get_fragment_source().c_str());
  if (!is_valid()) {
    return;
  }

  // Create a program object with both shaders.
  program = glCreateProgramObjectARB();
  glAttachObjectARB(program, vertex_shader);
  glAttachObjectARB(program, fragment_shader);

  glLinkProgramARB(program);

  // Check the link status.
  GLint status;
  glGetObjectParameterivARB(program, GL_OBJECT_LINK_STATUS_ARB, &status);

  if (status == 0) {
    GLint length = 0;
    char* info = nullptr;

    glGetObjectParameterivARB(program, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);
    info = SDL_stack_alloc(char, length + 1);
    glGetInfoLogARB(program, length, nullptr, info);
    set_error(std::string("Failed to link shader ") + get_id() + std::string(" :\n") + info);
    set_valid(false);
    SDL_stack_free(info);
  }
}

/**
 * \brief Compile a shader from source.
 * \param type The type of the shader to fill and compile.
 * \param source Sources to compile.
 * \return The shader created and compiled.
 */
GLhandleARB GlArbShader::create_shader(unsigned int type, const char* source) {

  GLint status;

  // Create the shader object.
  GLhandleARB shader = glCreateShaderObjectARB(type);

  glShaderSourceARB(shader, 1, &source, nullptr);
  glCompileShaderARB(shader);

  // Check the compile status.
  glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &status);
  if (status == 0) {
    GLint length;
    char* info;

    glGetObjectParameterivARB(shader, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);
    info = SDL_stack_alloc(char, length + 1);
    glGetInfoLogARB(shader, length, nullptr, info);
    set_error(std::string("Failed to compile shader ") + get_id() + std::string(" :\n") + info);
    set_valid(false);
    SDL_stack_free(info);
  }

  return shader;
}

/**
 * \brief Set up OpenGL rendering parameter.
 * This basically resets the projection matrix.
 */
void GlArbShader::set_rendering_settings() {

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(-1.0, 1.0, -1.0, 1.0, 0.0, 1.0);

  glMatrixMode(GL_MODELVIEW);
}

/**
 * \copydoc Shader::render
 */
void GlArbShader::render(const SurfacePtr& quest_surface) {

  Shader::render(quest_surface);

  SDL_Renderer* renderer = Video::get_renderer();
  SDL_Window* window = Video::get_window();
  SDL_Texture* render_target = Video::get_render_target();

  // Clear the window.
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Clear the render target.
  SDL_SetRenderTarget(renderer, render_target);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderSetClipRect(renderer, nullptr);
  SDL_RenderClear(renderer);

  // Draw on the render target.
  quest_surface->render(*render_target);

  SDL_SetRenderTarget(renderer, nullptr);
  set_rendering_settings();

  // Update uniform variables.
  GLhandleARB previous_program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
  glUseProgramObjectARB(program);

  const Size& output_size = Video::get_output_size();
  set_uniform_1i("sol_time", System::now());
  set_uniform_2f("sol_output_size", output_size.width, output_size.height);

  glActiveTextureARB(GL_TEXTURE0_ARB + 0);  // Texture unit 0.
  SDL_GL_BindTexture(render_target, nullptr, nullptr);

  // Draw.
  const GLfloat square_texcoord[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
  const GLfloat* texcoord = square_texcoord;

  glBegin(GL_QUADS);
  glTexCoord2f(texcoord[0], texcoord[1]);
  glVertex3f(-1.0f, 1.0f, 0.0f);  // Top left.
  glTexCoord2f(texcoord[2], texcoord[1]);
  glVertex3f(1.0f, 1.0f, 0.0f);  // Top right.
  glTexCoord2f(texcoord[2], texcoord[3]);
  glVertex3f(1.0f, -1.0f, 0.0f);  // Bottom right.
  glTexCoord2f(texcoord[0], texcoord[3]);
  glVertex3f(-1.0f, -1.0f, 0.0f);  // Bottom left.
  glEnd();

  SDL_GL_UnbindTexture(render_target);

  for (const auto& kvp : uniform_texture_units) {
    const GLuint texture_unit = kvp.second;
    glActiveTextureARB(GL_TEXTURE0_ARB + texture_unit);
    const GlTextureHandle& texture = uniform_textures.find(kvp.first)->second;
    glBindTexture(GL_TEXTURE_2D, texture.get());
  }

  glActiveTextureARB(GL_TEXTURE0_ARB + 0);  // Necessary to reset the active texture.

  // And swap the window.
  SDL_GL_SwapWindow(window);

  glUseProgramObjectARB(previous_program);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
}

/**
 * \brief Returns the location of a uniform value in the shader program.
 * \param uniform_name Name of the uniform to find.
 * \return The uniform location or \c -1.
 */
GLint GlArbShader::get_uniform_location(const std::string& uniform_name) const {

  const auto& it = uniform_locations.find(uniform_name);
  if (it != uniform_locations.end()) {
    return it->second;
  }

  GLhandleARB previous_program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
  glUseProgramObjectARB(program);
  const GLint location = glGetUniformLocationARB(program, uniform_name.c_str());
  glUseProgramObjectARB(previous_program);
  uniform_locations.insert(std::make_pair(uniform_name, location));
  return location;
}

/**
 * \copydoc Shader::set_uniform1f
 */
void GlArbShader::set_uniform_1b(const std::string& uniform_name, bool value) {

  const GLint location = get_uniform_location(uniform_name);
  if (location == -1) {
    return;
  }

  GLhandleARB previous_program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
  glUseProgramObjectARB(program);
  glUniform1iARB(location, (value ? 1 : 0));
  glUseProgramObjectARB(previous_program);
}

/**
 * \copydoc Shader::set_uniform_1i
 */
void GlArbShader::set_uniform_1i(const std::string& uniform_name, int value) {

  const GLint location = get_uniform_location(uniform_name);
  if (location == -1) {
    return;
  }

  GLhandleARB previous_program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
  glUseProgramObjectARB(program);
  glUniform1iARB(location, value);
  glUseProgramObjectARB(previous_program);
}

/**
 * \copydoc Shader::set_uniform_1f
 */
void GlArbShader::set_uniform_1f(const std::string& uniform_name, float value) {

  const GLint location = get_uniform_location(uniform_name);
  if (location == -1) {
    return;
  }

  GLhandleARB previous_program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
  glUseProgramObjectARB(program);
  glUniform1fARB(location, value);
  glUseProgramObjectARB(previous_program);
}

/**
 * \copydoc Shader::set_uniform_2f
 */
void GlArbShader::set_uniform_2f(const std::string& uniform_name, float value_1, float value_2) {

  const GLint location = get_uniform_location(uniform_name);
  if (location == -1) {
    return;
  }

  GLhandleARB previous_program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
  glUseProgramObjectARB(program);
  glUniform2fARB(location, value_1, value_2);
  glUseProgramObjectARB(previous_program);
}

/**
 * \copydoc Shader::set_uniform_3f
 */
void GlArbShader::set_uniform_3f(
    const std::string& uniform_name, float value_1, float value_2, float value_3) {

  const GLint location = get_uniform_location(uniform_name);
  if (location == -1) {
    return;
  }

  GLhandleARB previous_program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
  glUseProgramObjectARB(program);
  glUniform3fARB(location, value_1, value_2, value_3);
  glUseProgramObjectARB(previous_program);
}

/**
 * \copydoc Shader::set_uniform_4f
 */
void GlArbShader::set_uniform_4f(
    const std::string& uniform_name, float value_1, float value_2, float value_3, float value_4) {

  const GLint location = get_uniform_location(uniform_name);
  if (location == -1) {
    return;
  }

  GLhandleARB previous_program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
  glUseProgramObjectARB(program);
  glUniform4fARB(location, value_1, value_2, value_3, value_4);
  glUseProgramObjectARB(previous_program);
}

/**
 * \copydoc Shader::set_uniform_texture
 */
bool GlArbShader::set_uniform_texture(const std::string& uniform_name, const SurfacePtr& value) {

  const GLint location = get_uniform_location(uniform_name);
  if (location == -1) {
    // Not an error.
    return true;
  }

  GLhandleARB previous_program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
  glUseProgramObjectARB(program);

  int texture_unit = 0;
  const auto& it = uniform_texture_units.find(value);
  if (it != uniform_texture_units.end()) {
    // Texture already created.
    texture_unit = it->second;
    const GlTextureHandle& texture = uniform_textures.at(value);
    update_gl_texture(value, texture);
  }
  else {
    texture_unit = uniform_texture_units.size() + 1;
    uniform_texture_units[value] = texture_unit;
    uniform_textures.emplace(value, create_gl_texture(value));
  }
  glUniform1iARB(location, texture_unit);

  glUseProgramObjectARB(previous_program);

  return true;
}

/**
 * \brief Creates an OpenGL texture from the given surface.
 * \param surface The source surface.
 * \return The OpenGL texture or an empty handle in case of failure.
 */
GlTextureHandle GlArbShader::create_gl_texture(const SurfacePtr& surface) {

  // Create the OpenGL texture.
  GLuint handle = 0;
  glGenTextures(1, &handle);
  GlTextureHandle texture(handle);

  glBindTexture(GL_TEXTURE_2D, texture.get());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glBindTexture(GL_TEXTURE_2D, 0);

  update_gl_texture(surface, texture);
  return texture;
}

/**
 * \brief Copies a surface to an OpenGL texture.
 * \param surface The source surface.
 * \param texture The destination OpenGL texture.
 */
void GlArbShader::update_gl_texture(const SurfacePtr& surface, const GlTextureHandle& texture) {

  // Use the surface width and height.
  const Size& size = surface->get_size();

  // TODO Don't copy to an intermediate surface if the format is already RGBA or ABGR.
  Surface::SDL_Surface_UniquePtr rgba_surface;
#if SDL_BYTEORDER == SDL_LIL_ENDIAN     // OpenGL RGBA masks.
  rgba_surface = Surface::SDL_Surface_UniquePtr(SDL_CreateRGBSurface(
      0, size.width, size.height, 32,
      0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
  ));
#else
  rgba_surface = Surface::SDL_Surface_UniquePtr(SDL_CreateRGBSurface(
      0, size.width, size.height, 32,
      0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF
  ));
#endif

  Debug::check_assertion(rgba_surface != nullptr, "Failed to create surface");

  // Save the alpha blending attributes.
  BlendMode saved_mode = surface->get_blend_mode();
  surface->set_blend_mode(BlendMode::NONE);

  // Copy the original surface into the power-of-two surface.
  SDL_Rect area;
  area.x = 0;
  area.y = 0;
  area.w = size.width;
  area.h = size.height;
  SDL_BlitSurface(surface->get_internal_surface(), &area, rgba_surface.get(), &area);

  // Restore the alpha blending attributes.
  surface->set_blend_mode(saved_mode);

  // Copy the image to the OpenGL texture.
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // Restore default pixel alignment settings.
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
  glBindTexture(GL_TEXTURE_2D, texture.get());
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RGBA8, size.width, size.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_surface->pixels);

  glBindTexture(GL_TEXTURE_2D, 0);
}

}

#endif  // SOLARUS_HAVE_OPENGL
