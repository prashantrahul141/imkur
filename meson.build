project('imkur', ['cpp', 'c'], default_options : ['cpp_std=c++23', 'default_library=static'], license: 'Apache-2.0', license_files: ['LICENSE'], version: '0.1')

cmake = import('cmake')
fs = import('fs')
cxxc = meson.get_compiler('cpp')

warning_level = 'everything'
prefer_static = true
werror = true
b_pch = true

if get_option('buildtype') == 'debug'
  add_project_arguments('-DDEBUG_BUILD', language: ['c', 'cpp'])
  strip = false
  optimization = 'plain'
  b_lto = false
  b_sanitize = ['address', 'memory', 'leak']
else
  strip = true
  optimization = 3
  b_lto = true
  b_lto_threads = 1
endif

src_files = files(
  # app src files
  'src/main.cpp',
  'src/ui.cpp',
  'src/editor.cpp',
  'src/app.cpp',
  'src/plugins_manager.cpp',
  'src/common.cpp',

  # nhlog
  'thirdparty/nhlog.cpp',

  # glad
  'thirdparty/glad/src/glad.c',

  # imgui core files
  'thirdparty/imgui/imgui.cpp',
  'thirdparty/imgui/imgui_draw.cpp',
  'thirdparty/imgui/imgui_widgets.cpp',
  'thirdparty/imgui/imgui_tables.cpp',

  # imgui additional files
  'thirdparty/imgui/backends/imgui_impl_opengl3.cpp',
  'thirdparty/imgui/backends/imgui_impl_glfw.cpp',

)

deps = []
includes = []

includes += include_directories('./thirdparty')
includes += include_directories('./thirdparty/imgui')
includes += include_directories('./thirdparty/imgui/backends')
includes += include_directories('./thirdparty/glad/include')

# all platform required dependencies
## math lib
m_dep = cxxc.find_library('m', required: true)
deps += m_dep

# opengl
opengl_dep = dependency('opengl', required: true)
deps += opengl_dep

glfw_opt_vars = cmake.subproject_options()

deps += dependency('GL', required: true)

# linux only
if build_machine.system() == 'linux'
  glfw_opt_vars.add_cmake_defines({'GLFW_BUILD_WAYLAND': false, 'GLFW_BUILD_X11': true, 'GLFW_BUILD_EXAMPLES': false, 'GLFW_BUILD_TESTS': false, 'GLFW_BUILD_DOCS': false})
  deps += dependency('dl', required: true)
  deps += dependency('dbus-1', required: true)
  deps += dependency('X11', required: true)

# windows only
elif build_machine.system() == 'windows'

# mac only
else
  deps += dependency('dl', required: true)
  deps += dependency('appleframeworks', modules: ['IOKit', 'CoreFoundation'])

endif

# native file dialog
nfd_project = subproject('nativefiledialog-extended')
nfd_dep = nfd_project.get_variable('nfde_dep')
deps += nfd_dep

glfw_subproj = cmake.subproject('glfw', options: glfw_opt_vars)
glfw_dep  = glfw_subproj.dependency('glfw')
deps += glfw_dep

executable('imkur', src_files, dependencies: deps, include_directories: includes, cpp_args : ['-Wconversion', '-Wsign-conversion'])

# plugins
message('building plugins')

plugins_out_dir = join_paths(meson.project_build_root(), 'plugins')

plugin_src = files(
  'plugins/pencil.cpp'
)
foreach plugin_file: plugin_src
  plugin_target = shared_module(fs.stem(plugin_file), plugin_file, native: true)
endforeach


# tools
# message('building tools')

# executable('img2c_array', ['tools/img2c_array.c'], dependencies: [raylib_dep, m_dep], include_directories: thirdparty_includes)
