#include "editor.hpp"
#include "imgui.h"
#include "internal.h"
#include "nhlog.h"
#include <cstddef>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

/*
 * Constructor
 */
Editor::Editor() {
  nhlog_debug("Editor: init");
  this->img.data = nullptr;
  this->img.width = this->img.height = this->img.channels = 0;
  this->editor_state.opacity = 100;
  this->editor_state.put_pixel_size = 1;
  this->editor_state.primary_selected_color =
      Color{.r = 255, .g = 255, .b = 255, .a = 255};
}

/*
 * Cleans up resources
 */
Editor::~Editor() {
  nhlog_info("Editor: destroying");
  this->unload_image();
}

/*
 * Loads given image path into current editor context.
 * @param path - path to the image to load
 * @returns true if succeeded, false if failed
 */
bool Editor::load_image(const char *const path) {
  nhlog_debug("Editor: load_image(path = %s)", path);
  this->unload_image();
  this->img.data = stbi_load(path, &this->img.width, &this->img.height,
                             &this->img.channels, 0);

  if (nullptr == this->img.data) {
    return false;
  }

  nhlog_debug("Editor: loaded image = %s, width = %d, height = %d, channels = "
              "%d, components_per_pixel = %d",
              path, this->img.width, this->img.height, this->img.channels,
              this->img.components_per_pixel);

  this->regen_texture();
  return true;
}

/*
 * Unloads the current loaded image.
 */
void Editor::unload_image() {
  if (nullptr != this->img.data) {
    nhlog_debug("Editor: unloading existing image data.");
    stbi_image_free(this->img.data);
    this->img.data = nullptr;
    glDeleteTextures(1, &this->texture.texture_id);
  }
}

/*
 * Saves images to the given path.
 */
void Editor::save_image(const char *const path) {
  nhlog_debug("Editor: saving image");

  if (!stbi_write_png(path, static_cast<int>(this->img.width),
                      static_cast<int>(this->img.height),
                      static_cast<int>(this->img.channels), this->img.data,
                      0)) {
    // app_notify(NOTIF_ERROR, "Failed to save image.");
  } else {
    // app_notify(NOTIF_SUCCESS, "Save image");
  }
}

/*
 * Regen texture from image data.
 * @returns true if succeeded, false if failed.
 */
void Editor::regen_texture() {
  glDeleteTextures(1, &this->texture.texture_id);
  glGenTextures(1, &this->texture.texture_id);
  glBindTexture(GL_TEXTURE_2D, this->texture.texture_id);

  // setup filtering parameters for display
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // upload pixels into texture
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  GLint format = this->img.channels == 4 ? GL_RGBA : GL_RGB;
  glTexImage2D(GL_TEXTURE_2D, 0, format, (int)this->img.width,
               (int)this->img.height, 0, (GLuint)format, GL_UNSIGNED_BYTE,
               this->img.data);
}

/*
 * Get color at a specific location.
 */
Color Editor::get_pixel(std::int32_t x, std::int32_t y) {
  const stbi_uc *p = this->img.data + (4 * (y * this->img.width + x));
  return {.r = p[0], .g = p[1], .b = p[2], .a = p[3]};
}

/*
 * Draws a cirlce around the given point according to internal size paramter.
 */
void Editor::draw_cirlce(Vec2<std::int32_t> center, Color color) {
  nhlog_debug("called at center = %d, %d", center.x, center.y);
  auto pixels_to_update = get_surrounding_pixels(
      center, this->editor_state.put_pixel_size, this->img);

  for (auto pixel : pixels_to_update) {
    this->put_pixel(color, pixel.to_imvec2());
  }
}

/*
 * Puts color at a specific location.
 */
void Editor::put_pixel(Color color, ImVec2 pos) {
  nhlog_debug("Editor: put_pixel color = (%d, %d, %d, %d), pos = (%f, %f)",
              color.r, color.g, color.b, color.a, pos.x, pos.y);

  size_t index =
      (static_cast<size_t>(pos.y) * static_cast<size_t>(this->img.width) +
       static_cast<size_t>(pos.x)) *
      static_cast<size_t>(this->img.channels);

  this->img.data[index] = color.r;
  this->img.data[index + 1] = color.g;
  this->img.data[index + 2] = color.b;
  //  this->img.data[index + 3] = color.a;

  this->regen_texture();
}
