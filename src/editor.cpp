#include "editor.hpp"
#include "nhlog.h"
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

  this->img.data =
      stbi_load(path, &this->img.width, &this->img.height, &this->img.channels,
                this->img.components_per_pixel);

  if (nullptr == this->img.data) {
    return false;
  }

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
  }
}

/*
 * Saves images to the given path.
 */
void Editor::save_image(const char *const path) {
  nhlog_debug("Editor: saving image");
  __builtin_dump_struct(&this->img, printf);
  if (nullptr == this->img.data) {
    // app_notify(NOTIF_ERROR, "no image.");
    return;
  }

  if (!stbi_write_png(path, this->img.width, this->img.height,
                      this->img.components_per_pixel, this->img.data, 0)) {
    // app_notify(NOTIF_ERROR, "Failed to save image.");
  } else {
    // app_notify(NOTIF_SUCCESS, "Save image");
  }
}
