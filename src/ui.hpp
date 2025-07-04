#pragma once

#include "glad/glad.h"
#include "imgui.h"
#include "src/common.hpp"
#include <GLFW/glfw3.h>
#include <chrono>
#include <cstdint>
#include <expected>
#include <memory>
#include <queue>

#define UI_FONT_ID_REGULAR 0
#define UI_FONT_ID_BOLD 1

/*
 * Variants of notifications
 */
typedef enum {
  NOTIF_ERROR,
  NOTIF_WARN,
  NOTIF_SUCCESS,
} NotifType;

/*
 * A single notification
 */
struct Notification {
  const NotifType ntype;
  const char *msg;
};

class UI {
public:
  std::queue<Notification> notification_queue;
  std::optional<std::shared_ptr<Notification>> current_notification;
  GLFWwindow *window;
  ImGuiIO *io;

private:
  float scale;
  Vec2<float> pan;
  int32_t active_plugin_index;
  Vec2<std::int32_t> last_pos_put_pixel;
  std::chrono::milliseconds last_put_pixel_time;

public:
  /*
   * Constructor
   */
  UI() noexcept;

  /*
   * Check whether the ui should close.
   *
   * @returns whether the window should close
   */
  [[nodiscard]] bool should_close() noexcept;

  /*
   * Updates all ui related stuff
   */
  void update();

  /*
   * Adds a notification to queue
   * @param type - NotifType variant
   * @param msg - message
   */
  void notify(const NotifType type, const char *msg);

  /*
   * Destructor
   */
  ~UI();

private:
  /*
   * Updates ui state
   * @returns Whether to rerender
   */
  bool update_state();

  /*
   * Updates the layout
   */
  void update_layout();

  /*
   * Layout functions
   */
  void update_layout_menubar();
  void update_layout_sidebar();
  void update_layout_bottombar();
  void update_layout_rightbar();
  void update_layout_image_window();

  /*
   * Renderes on screen
   */
  void update_draw();

  /*
   * Called when menu item file -> open is called
   */
  void menu_callback_file_open();

  /*
   * Called when menu item file -> save is called
   */
  void menu_callback_save_open();
};
