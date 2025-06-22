#pragma once
#include "common.hpp"
#include "glad/glad.h"
#include <cstdint>

struct Image {
  std::uint8_t *data;
  std::int32_t width, height, channels;
  const std::int32_t components_per_pixel = 4;
};

struct EditorState {
  Color primary_selected_color;
  uint8_t opacity;
};

class Editor {
public:
  Image img;
  Texture texture;
  EditorState editor_state;

public:
  /*
   * Constructor
   */
  Editor();

  /*
   * Cleans up resources
   */
  ~Editor();

  /*
   * Loads given image path into current editor context.
   * @param path - path to the image to load
   * @returns true if succeeded, false if failed
   */
  bool load_image(const char *const path);

  /*
   * Unloads the current loaded image.
   */
  void unload_image();

  /*
   * Saves images to the given path.
   */
  void save_image(const char *const path);

  /*
   * Get color at a specific location.
   */
  Color get_pixel(std::int32_t x, std::int32_t y);

  /*
   * Puts color at a specific location.
   */
  void put_pixel(Color color, ImVec2 pos);

  /*
   * Regen texture from image data.
   * @returns true if succeeded, false if failed.
   */
  void regen_texture();
};
