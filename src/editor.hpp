#pragma once
#include <cstdint>

struct Image {
  std::uint8_t *data;
  std::int32_t width, height, channels;
  const std::int32_t components_per_pixel = 4;
};

class Editor {
public:
  Image img;

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
};
