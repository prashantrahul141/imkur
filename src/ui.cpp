#include "ui.hpp"
#include "config.hpp"
#include "glad/glad.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "nfd.h"
#include "plugin_base.hpp"
#include "src/app.hpp"
#include "src/common.hpp"
#include "src/editor.hpp"
#include "src/plugins_manager.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cfloat>
#include <chrono>
#include <cstdint>
#include <cstdio>
#define NFD_NATIVE
#include "nhlog.h"
#include <cassert>
#include <cstdlib>

#ifdef _WIN32
const wchar_t *default_path = L"default.png";
static nfdfilteritem_t open_dialog_filter_list[1] = {
    {L"Image", L"png,jpg,jpeg"}};
#else
const char *default_path = "default.png";
static nfdfilteritem_t open_dialog_filter_list[1] = {{"Image", "png,jpg,jpeg"}};
#endif

static void glfw_error_callback(int error, const char *description) {
  nhlog_error("GLFW ERROR :%d: %s", error, description);
}

/*
 * constructor
 */
UI::UI() noexcept
    : scale(1.0f), pan(ImVec2(0.0f, 0.0f)), active_plugin_index(-1),
      last_pos_put_pixel(Vec2(-1, -1)), last_put_pixel_time(0) {
  nhlog_info("UI: ui init");
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit()) {
    nhlog_fatal("UI: glfw init failed");
    std::abort();
  }

  nhlog_info("UI: init native file dialog");
  NFD_Init();

  // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100 (WebGL 1.0)
  const char *glsl_version = "#version 100";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
  // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
  const char *glsl_version = "#version 300 es";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
  // GL 3.2 + GLSL 150
  const char *glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#else
  // GL 3.0 + GLSL 130
  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

  nhlog_info("UI: glsl_version + %s", glsl_version);

  float main_scale =
      ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
  GLFWwindow *window =
      glfwCreateWindow((int)(UI_INIT_WINDOW_WIDTH * main_scale),
                       (int)(UI_INIT_WINDOW_HEIGHT * main_scale),
                       UI_WINDOW_TITLE, nullptr, nullptr);

  if (!window) {
    nhlog_fatal("UI: window creation failed");
    std::abort();
  }

  nhlog_debug("UI: window creation success");

  glfwMakeContextCurrent(window);
  if (UI_ENABLE_VSYNC) {
    nhlog_debug("UI: ui enable vsync enabled");
    glfwSwapInterval(1);
  }

  if (UI_ENABLE_ANTIALIASING) {
    glfwWindowHint(GLFW_SAMPLES, UI_ANTIALIASING_MUT);
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigDpiScaleFonts = true;
  io.ConfigDpiScaleViewports = true;

  nhlog_debug("UI: loading fonts.");
  nhlog_info("UI: loading font file = %s", PATH_FONT_FILE);
  ImFont *font = io.Fonts->AddFontFromFileTTF(PATH_FONT_FILE);
  assert(nullptr != font);

  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    nhlog_fatal("UI: failed gladLoadGLLoader");
    std::abort();
  }
  ImGui_ImplOpenGL3_Init(glsl_version);

  this->window = window;
  this->io = &io;
  nhlog_info("UI: init finished.");
}

/*
 * Window should close
 */
[[nodiscard]] bool UI::should_close() noexcept {
  return glfwWindowShouldClose(this->window);
}

/*
 * Updates all ui related stuff
 */
void UI::update() {
  ImVec4 clear_color = ImVec4(COLOR_PRIMARY_BACKGROUND);
  glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
  glClear(GL_COLOR_BUFFER_BIT);
  nhlog_trace("UI: updating");
  if (this->update_state()) {
    // only rerender if needed
    this->update_layout();
    this->update_draw();
  }
}

/*
 * Updates ui state
 * @returns Whether to rerender
 */
bool UI::update_state() {
  nhlog_trace("UI: update_state");
  glfwPollEvents();
  if (glfwGetWindowAttrib(this->window, GLFW_ICONIFIED) != 0) {
    ImGui_ImplGlfw_Sleep(10);
    return false;
  }
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  return true;
}

/*
 * Updates the layout
 */
void UI::update_layout() {
  nhlog_trace("UI: update layout");
  this->update_layout_menubar();
  this->update_layout_sidebar();
  this->update_layout_rightbar();
  this->update_layout_bottombar();
  this->update_layout_image_window();
}

/*
 * *****************************
 * Layout functions
 * *****************************
 */
void UI::update_layout_menubar() {
  ImGui::BeginMainMenuBar();
  if (ImGui::BeginMenu("File")) {
    if (ImGui::MenuItem("open")) {
      nhlog_debug("UI: clicked open file dialog");
      this->menu_callback_file_open();
    }

    if (ImGui::MenuItem("save")) {
      nhlog_debug("UI: clicked save file dialog");
      this->menu_callback_save_open();
    }

    ImGui::EndMenu();
  }

  ImGuiStyle &style = ImGui::GetStyle();
  float size =
      ImGui::CalcTextSize(UI_WINDOW_TITLE).x + style.FramePadding.x * 2.0f;
  float avail = ImGui::GetContentRegionAvail().x;
  float off = (avail - size) * 0.5f;
  if (off > 0.0f)
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
  ImGui::Text(UI_WINDOW_TITLE);

  ImGui::EndMainMenuBar();
}

void UI::update_layout_sidebar() {
  ImGuiViewportP *viewport = (ImGuiViewportP *)(void *)ImGui::GetMainViewport();
  PluginManager *plugins_manager = &App::global_app_context->plugins_manager;
  ImGui::BeginViewportSideBar(
      "##SIDEBAR", viewport, ImGuiDir_Left, UI_SIDEBAR_WIDTH,
      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
          ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNavFocus);

  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.f);
  for (int32_t i = 0; i < plugins_manager->plugins.size(); i++) {
    Plugin plugin = plugins_manager->plugins[static_cast<size_t>(i)];
    if (PLUGIN_TYPE_PUT_PIXEL != plugin.info_function()->plugin_type) {
      continue;
    }

    if (i == this->active_plugin_index) {
      ImGui::PushStyleColor(ImGuiCol_Button,
                            ImVec4(COLOR_SECONDARY_BACKGROUND));

      if (ImGui::ImageButton(plugin.info_function()->name,
                             plugin.icon.texture_id,
                             ImVec2(PLUGIN_ICON_SIZE, PLUGIN_ICON_SIZE))) {
        this->active_plugin_index = i;
      }

      ImGui::PopStyleColor();
    } else {
      if (ImGui::ImageButton(plugin.info_function()->name,
                             plugin.icon.texture_id,
                             ImVec2(PLUGIN_ICON_SIZE, PLUGIN_ICON_SIZE))) {
        this->active_plugin_index = i;
      }
    }

    if (ImGui::BeginItemTooltip()) {
      ImGui::Text("%s", plugin.info_function()->name);
      ImGui::PushFontSize(ImGui::GetFontSize() * 0.8f);
      ImGui::Text("%s", plugin.info_function()->description);
      ImGui::PopFontSize();
      ImGui::EndTooltip();
    }
  }
  ImGui::PopStyleVar();
  ImGui::End();
}

void UI::update_layout_bottombar() {
  ImGuiViewportP *viewport = (ImGuiViewportP *)(void *)ImGui::GetMainViewport();

  ImGui::BeginViewportSideBar(
      "##BOTTOM_BAR", viewport, ImGuiDir_Down, UI_BOTTOMBAR_HEIGHT,
      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
          ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNavFocus);

  ImGui::ColorButton(
      "Active Color",
      ColorToImVec4(
          App::global_app_context->editor.editor_state.primary_selected_color));
  ImGui::SameLine(60.0f);
  if (ImGui::ColorButton("White", ImVec4(UI_SWATCH_1))) {
    App::global_app_context->editor.editor_state.primary_selected_color =
        ImVec4ToColor(ImVec4(UI_SWATCH_1));
  }

  ImGui::SameLine();
  if (ImGui::ColorButton("Black", ImVec4(UI_SWATCH_2))) {
    App::global_app_context->editor.editor_state.primary_selected_color =
        ImVec4ToColor(ImVec4(UI_SWATCH_2));
  }

  ImGui::SameLine();
  if (ImGui::ColorButton("Red", ImVec4(UI_SWATCH_3))) {
    App::global_app_context->editor.editor_state.primary_selected_color =
        ImVec4ToColor(ImVec4(UI_SWATCH_3));
  }

  ImGui::SameLine();
  if (ImGui::ColorButton("Green", ImVec4(UI_SWATCH_4))) {
    App::global_app_context->editor.editor_state.primary_selected_color =
        ImVec4ToColor(ImVec4(UI_SWATCH_4));
  }

  ImGui::SameLine();
  if (ImGui::ColorButton("Blue", ImVec4(UI_SWATCH_5))) {
    App::global_app_context->editor.editor_state.primary_selected_color =
        ImVec4ToColor(ImVec4(UI_SWATCH_5));
  }

  ImGui::SameLine();
  if (ImGui::ColorButton("Yellow", ImVec4(UI_SWATCH_6))) {
    App::global_app_context->editor.editor_state.primary_selected_color =
        ImVec4ToColor(ImVec4(UI_SWATCH_6));
  }
  ImGui::SameLine();
  if (ImGui::ColorButton("Cyan", ImVec4(UI_SWATCH_7))) {
    App::global_app_context->editor.editor_state.primary_selected_color =
        ImVec4ToColor(ImVec4(UI_SWATCH_7));
  }
  ImGui::SameLine();
  if (ImGui::ColorButton("Magenta", ImVec4(UI_SWATCH_8))) {
    App::global_app_context->editor.editor_state.primary_selected_color =
        ImVec4ToColor(ImVec4(UI_SWATCH_8));
  }

  ImGui::SameLine(350.0f);
  ImGui::SetNextItemWidth(60.0f);
  ImGui::InputInt("size",
                  &App::global_app_context->editor.editor_state.put_pixel_size,
                  0, 0, 0);

  // clamp minimum to 1.
  App::global_app_context->editor.editor_state.put_pixel_size =
      std::max(1, App::global_app_context->editor.editor_state.put_pixel_size);

  ImGui::End();
}

void UI::update_layout_rightbar() {
  // Editor *editor = &App::global_app_context->editor;

  ImGuiViewportP *viewport = (ImGuiViewportP *)(void *)ImGui::GetMainViewport();
  PluginManager *plugins_manager = &App::global_app_context->plugins_manager;

  ImGui::BeginViewportSideBar(
      "##RIGHT_BAR", viewport, ImGuiDir_Right, UI_RIGHTBAR_WIDTH,
      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
          ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNavFocus);

  for (size_t i = 0; i < plugins_manager->plugins.size(); i++) {
    if (PLUGIN_TYPE_REPLACE_IMAGE !=
        plugins_manager->plugins[i].info_function()->plugin_type) {
      continue;
    }
  }

  ImGui::End();
}

void UI::update_layout_image_window() {
  Editor *editor = &App::global_app_context->editor;
  ImGuiIO &io = ImGui::GetIO();

  ImGui::SetNextWindowPos(
      ImVec2(io.DisplaySize.x * 0.502f - (UI_RIGHTBAR_WIDTH / 3),
             io.DisplaySize.y * 0.483f),
      ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(
      ImVec2(io.DisplaySize.x - (UI_SIDEBAR_WIDTH + UI_RIGHTBAR_WIDTH) - 20.0f,
             io.DisplaySize.y - (UI_BOTTOMBAR_HEIGHT)-50.0f));

  ImGui::Begin("##IMAGE_WINDOW", NULL,
               ImGuiWindowFlags_NoNav | (ImGuiWindowFlags_NoDecoration |
                                         ImGuiWindowFlags_NoScrollWithMouse |
                                         ImGuiWindowFlags_NoSavedSettings));

  Vec2<std::int32_t> image_scaled_size =
      Vec2(static_cast<std::int32_t>((float)editor->img.width * this->scale),
           static_cast<std::int32_t>((float)editor->img.height * this->scale));

  Vec2<std::int32_t> top_left_of_image_relative_to_image_window = Vec2(
      static_cast<std::int32_t>((ImGui::GetWindowSize().x -
                                 (float)image_scaled_size.x + this->pan.x) *
                                0.5f),
      static_cast<std::int32_t>((ImGui::GetWindowSize().y -
                                 (float)image_scaled_size.y + this->pan.y) *
                                0.5f));

  std::chrono::milliseconds now = duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch());

  // mouse above the image window.
  if (ImGui::IsWindowHovered()) {
    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
      this->last_pos_put_pixel.x = this->last_pos_put_pixel.y = -1;
    }

    // clicked over the image.
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
        -1 != this->active_plugin_index &&
        // we only draw if it have been more than EDITOR_PUT_PIXEL_DELAY_MS
        (now - this->last_put_pixel_time).count() > EDITOR_PUT_PIXEL_DELAY_MS) {

      nhlog_debug("drawing over image");
      this->last_put_pixel_time = now;

      EditorState es = App::global_app_context->editor.editor_state;
      // calc mouse position relative to image.
      Vec2 absolute_mouse_pos = Vec2<std::int32_t>(ImGui::GetMousePos());
      Vec2 window_pos = Vec2<std::int32_t>(ImGui::GetWindowPos());
      Vec2 image_window_size = Vec2<std::int32_t>(ImGui::GetWindowSize());
      Vec2 relative_mouse_pos = Vec2(absolute_mouse_pos.x - window_pos.x,
                                     absolute_mouse_pos.y - window_pos.y);

      // make sure mouse position is inside the image window.
      if (relative_mouse_pos.x > 0 &&
          relative_mouse_pos.x < image_window_size.x &&
          relative_mouse_pos.y > 0 &&
          relative_mouse_pos.y < image_window_size.y) {

        // inside actual image
        if (
            // within left bound
            relative_mouse_pos.x >
                top_left_of_image_relative_to_image_window.x &&
            // within right bound
            relative_mouse_pos.x <
                (top_left_of_image_relative_to_image_window.x +
                 image_scaled_size.x) &&
            // within top bound
            relative_mouse_pos.y >
                top_left_of_image_relative_to_image_window.y &&
            // within bottom bound
            relative_mouse_pos.y <
                (top_left_of_image_relative_to_image_window.y +
                 image_scaled_size.y)) {

          // finally calculate mouse position relative to image.
          Vec2<std::int32_t> mouse_relative_to_image =
              Vec2(static_cast<std::int32_t>(
                       ((float)relative_mouse_pos.x -
                        (float)top_left_of_image_relative_to_image_window.x) /
                       this->scale),
                   static_cast<std::int32_t>(
                       ((float)relative_mouse_pos.y -
                        (float)top_left_of_image_relative_to_image_window.y) /
                       this->scale));
          nhlog_trace("UI: clicking inside image at x = %d, y = %d",
                      mouse_relative_to_image.x, mouse_relative_to_image.y);

          Color color =
              App::global_app_context->plugins_manager
                  .plugins[static_cast<size_t>(this->active_plugin_index)]
                  .callback.put_pixel(es, mouse_relative_to_image.to_imvec2());

          // if last frame mouse wasnt clicking on the image.
          if (this->last_pos_put_pixel.x < 0 ||
              this->last_pos_put_pixel.y < 0) {
            this->last_pos_put_pixel = mouse_relative_to_image;
            App::global_app_context->editor.draw_cirlce(mouse_relative_to_image,
                                                        color);
          }
          // but if last frame mouse was clicking on the image we lerp through
          // these two mouse positions
          else {
            Vec2<std::int32_t> diff =
                mouse_relative_to_image - this->last_pos_put_pixel;
            auto step_spacing =
                (EDITOR_LERP_STEP_SPACING_PERCENT / 100) *
                App::global_app_context->editor.editor_state.put_pixel_size;
            std::int32_t steps = static_cast<int>(diff.dist() / step_spacing);

            // lerp through each step
            for (std::int32_t i = 0; i <= steps; i++) {
              float t = static_cast<float>(i) / static_cast<float>(steps);
              Vec2<std::int32_t> pos = Vec2<std::int32_t>::lerp(
                  this->last_pos_put_pixel, mouse_relative_to_image, t);

              App::global_app_context->editor.draw_cirlce(pos, color);
            }
          }

          this->last_pos_put_pixel = mouse_relative_to_image;
        }
      }
    }

    // hovered and scrolling
    else if (0.0f != io.MouseWheel) {
      // with left ctrl key pressed: ZOOM
      if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
        float prev_scale = this->scale;
        this->scale = 0 < io.MouseWheel ? this->scale * UI_IMAGE_ZOOM_RATE
                                        : this->scale / UI_IMAGE_ZOOM_RATE;
        nhlog_debug("UI: scale = %f", this->scale);
        // stop zooming in or out when we reach 1.
        if ((prev_scale < 1 && this->scale > 1) ||
            (prev_scale > 1 && this->scale < 1)) {
          this->scale = 1;
        }

        // clamp scale
        this->scale =
            std::clamp(this->scale, UI_IMAGE_MIN_SCALE, UI_IMAGE_MAX_SCALE);

        // ALSO clamp x and y pan when zooming in or out, makes zooming in and
        // out much smoother.
        float limit_pan_x = ImGui::GetWindowSize().x * this->scale;
        this->pan.x = std::clamp(this->pan.x, -limit_pan_x, limit_pan_x);

        float limit_pan_y = ImGui::GetWindowSize().y * this->scale;
        this->pan.y = std::clamp(this->pan.y, -limit_pan_y, limit_pan_y);

        // with left shift key pressed : HORIZONTAL SCROLL
      } else if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
        float move = (float)editor->img.width * UI_IMAGE_SCROLL_RATE;
        this->pan.x -= 0 < io.MouseWheel ? move : -move;
        this->pan.x =
            std::clamp(this->pan.x, -ImGui::GetWindowSize().x * this->scale,
                       ImGui::GetWindowSize().x * this->scale);
        nhlog_debug("UI: pan = %f, %f", this->pan.x, this->pan.y);

        // no keys are pressed: VERTICAL SCROLL
      } else {
        float move = (float)editor->img.height * UI_IMAGE_SCROLL_RATE;
        this->pan.y -= 0 < io.MouseWheel ? -move : move;
        this->pan.y =
            std::clamp(this->pan.y, -ImGui::GetWindowSize().y * this->scale,
                       ImGui::GetWindowSize().y * this->scale);
        nhlog_debug("UI: pan = %f, %f", this->pan.x, this->pan.y);
      }
    }
  } else {
    this->last_pos_put_pixel.x = this->last_pos_put_pixel.y = -1;
  }
  ImGui::SetCursorPos(top_left_of_image_relative_to_image_window.to_imvec2());
  ImGui::Image((ImTextureID)(intptr_t)editor->texture.texture_id,
               ImVec2((float)editor->img.width * this->scale,
                      (float)editor->img.height * this->scale));

  ImGui::End();
}

/*
 * Renderes on screen
 */

void UI::update_draw() {
  nhlog_trace("UI: draw");
  ImGui::Render();
  int display_w, display_h;
  glfwGetFramebufferSize(window, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glfwSwapBuffers(window);
}

/*
 * Destructors
 */
UI::~UI() {
  nhlog_info("UI: destroying");
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  NFD_Quit();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();
}

/*
 * Called when menu item file -> open is called
 */
void UI::menu_callback_file_open() {
  nfdchar_t *out_path;
  nfdresult_t result =
      NFD_OpenDialog(&out_path, open_dialog_filter_list, 1, NULL);
  if (NFD_OKAY == result) {
    nhlog_info("UI: selected file = %s", out_path);
    App::global_app_context->editor.load_image(out_path);
    NFD_FreePath(out_path);
  } else if (NFD_ERROR == result) {
    nhlog_error("UI: failed to open file dialog.");
  }
}

/*
 * Called when menu item file -> save is called
 */
void UI::menu_callback_save_open() {
  if (nullptr == App::global_app_context->editor.img.data) {
    nhlog_warn("UI: no image to save, skipping.");
    return;
  }
  nfdchar_t *out_path;
  nfdresult_t result =
      NFD_SaveDialog(&out_path, open_dialog_filter_list, 1, NULL, default_path);
  if (NFD_OKAY == result) {
    nhlog_info("UI: selected file = %s", out_path);
    App::global_app_context->editor.save_image(out_path);
    NFD_FreePath(out_path);
  } else if (NFD_ERROR == result) {
    nhlog_error("UI: failed to open file dialog.");
  }
}
