#include "editor.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/*
 * Constructor
 */
Editor::Editor() : img_data(nullptr) {}

/*
 * Loads given image path into current editor context.
 * @param path - path to the image to load
 * @returns true if succeeded, false if failed
 */
bool Editor::load_image(const char *const path) {
  this->unload_image();

  int width, height, channels;
  const int components_per_pixel = 4;
  this->img_data =
      stbi_load(path, &width, &height, &channels, components_per_pixel);

  if (nullptr == this->img_data) {
    return false;
  }

  return true;
}

/*
 * Unloads the current loaded image.
 */
void Editor::unload_image() {
  if (nullptr != this->img_data) {
    stbi_image_free(this->img_data);
    this->img_data = nullptr;
  }
}
