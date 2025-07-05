#include "plugin_base.hpp"
#include "thirdparty/nhlog.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

static const uint8_t vars_len = 1;

static VariableMeta vars[vars_len] = {
    {.name = "Box size",
     .description = "Size of box blur kernel",
     .type = VariableMetaType::TYPE_INT,
     .default_value = {.default_int = true},
     .range = {.min = 1.0f, .max = 99999999.0f, .step = 1.0f}}};

static PluginInfo plugin_info = {
    .name = "Blur",
    .description = "Blur filter",
    .plugin_type = PluginType::PLUGIN_TYPE_REPLACE_IMAGE,
    .vars = vars,
    .vars_len = vars_len,
    .icon = {{0}},
};

extern "C" EXPORT PluginInfo *const GET_PLUGIN_INFO() { return &plugin_info; }

extern "C" EXPORT void PLUGIN_REPLACE_IMAGE(EditorState es, Image img,
                                            void *data) {
  size_t img_buffer_size =
      sizeof(uint8_t) * img.width * img.height * img.channels;
  uint8_t *copy_img_buffer = (uint8_t *)malloc(img_buffer_size);
  memcpy(copy_img_buffer, img.data, img_buffer_size);

  int32_t box_size = *(int *)data;
  for (size_t x = 0; x < img.width; x++) {
    for (size_t y = 0; y < img.height; y++) {

      size_t current_index =
          (static_cast<size_t>(y) * static_cast<size_t>(img.width) +
           static_cast<size_t>(x)) *
          static_cast<size_t>(img.channels);

      // get approximation of neighboring pixels
      LColor color = {.r = 0, .g = 0, .b = 0, .a = 0};
      uint8_t count = 0; // number of pixels present in THIS box.
      int32_t min_x = std::max(0, (int32_t)(x - box_size));
      int32_t max_x = std::min(img.width, (int32_t)(x + box_size + 1));
      int32_t min_y = std::max(0, (int32_t)(y - box_size));
      int32_t max_y = std::min(img.height, (int32_t)(y + box_size + 1));
      for (int32_t nx = min_x; nx < max_x; nx++) {
        for (int32_t ny = min_y; ny < max_y; ny++) {
          count++;
          size_t neighboring_index =
              (static_cast<size_t>(ny) * static_cast<size_t>(img.width) +
               static_cast<size_t>(nx)) *
              static_cast<size_t>(img.channels);

          color.r += copy_img_buffer[neighboring_index];
          color.g += copy_img_buffer[neighboring_index + 1];
          color.b += copy_img_buffer[neighboring_index + 2];
          color.a += copy_img_buffer[neighboring_index + 3];
        }
      }

      // minimum value of  count should be atleast 1 for division.
      count = std::max((uint8_t)1, count);

      Color final_color = {
          .r = static_cast<uint8_t>(color.r / count),
          .g = static_cast<uint8_t>(color.g / count),
          .b = static_cast<uint8_t>(color.b / count),
          .a = static_cast<uint8_t>(color.a / count),
      };
      img.data[current_index] = final_color.r;
      img.data[current_index + 1] = final_color.g;
      img.data[current_index + 2] = final_color.b;
      img.data[current_index + 3] = final_color.a;
    }
  }

  free(copy_img_buffer);
}
