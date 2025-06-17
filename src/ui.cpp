#include "ui.hpp"
#include "config.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "nfd.h"
#include "src/app.hpp"
#define NFD_NATIVE
#include "nhlog.h"
#include <GLFW/glfw3.h>
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
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

/*
 * constructor
 */
UI::UI() noexcept {
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
  update_layout_menubar();
  update_layout_sidebar();
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
  ImGui::EndMainMenuBar();
}

void UI::update_layout_sidebar() {
  ImGuiViewportP *viewport = (ImGuiViewportP *)(void *)ImGui::GetMainViewport();
  ImGui::BeginViewportSideBar(
      "##TOOL_BAR", viewport, ImGuiDir_Left, UI_SIDEBAR_WIDTH,
      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
          ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNavFocus);
  // render tool icons here
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
  ImVec4 clear_color = ImVec4(COLOR_PRIMARY_BACKGROUND);
  glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
  glClear(GL_COLOR_BUFFER_BIT);
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
    App::get_global_context()->editor.load_image(out_path);
    NFD_FreePath(out_path);
  } else if (NFD_ERROR == result) {
    nhlog_error("UI: failed to open file dialog.");
  }
}

/*
 * Called when menu item file -> save is called
 */
void UI::menu_callback_save_open() {
  nfdchar_t *out_path;
  nfdresult_t result =
      NFD_SaveDialog(&out_path, open_dialog_filter_list, 1, NULL, default_path);
  if (NFD_OKAY == result) {
    nhlog_info("UI: selected file = %s", out_path);
    App::get_global_context()->editor.save_image(out_path);
    NFD_FreePath(out_path);
  } else if (NFD_ERROR == result) {
    nhlog_error("UI: failed to open file dialog.");
  }
}
