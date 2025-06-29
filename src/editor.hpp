#pragma once
#include "common.hpp"
#include "glad/glad.h"
#include <cstdint>

struct EditorState {
  Color primary_selected_color;
  uint8_t opacity;
  // radius of put pixel tool pointer.
  int32_t put_pixel_size;
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
   * Draws a cirlce around the given point according to internal size paramter.
   */
  void draw_cirlce(Vec2<std::int32_t> center, Color color);

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
