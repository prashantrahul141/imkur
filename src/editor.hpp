#pragma once

class Editor {
public:
  unsigned char *img_data;

public:
  /*
   * Constructor
   */
  Editor();

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
};
