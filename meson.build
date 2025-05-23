project('Granite', 'cpp',
  version : '0.1.0',
  default_options : ['cpp_std=c++20', 'warning_level=2']
)


glfw_dep = dependency('glfw3', fallback: ['glfw', 'glfw_dep'], required : true)
vulkan_dep = dependency('vulkan', required : true)
glm_dep = dependency('glm', fallback: ['glm', 'glm_dep'], required : true)
spdlog_dep = dependency('spdlog', fallback: ['spdlog', 'spdlog_dep'], required : true)
#imgui_dep = dependency('imgui', fallback: ['imgui', 'imgui_dep'], required : true)



engine_includes = include_directories('engine/src')
editor_includes = include_directories('engine/src')

engine_sources = files(
  'engine/src/utils/Log.cpp',
  'engine/src/engine.cpp',
  'engine/src/renderer/vk_renderer.cpp',
)

engine_lib = library('Granite',
  engine_sources,
  include_directories : engine_includes,
  install : true,
  dependencies: [vulkan_dep, glfw_dep, glm_dep, spdlog_dep]
)

executable('Granite Editor',
  'editor/src/editor.cpp',
  link_with : engine_lib,
  include_directories : engine_includes,
  dependencies: [spdlog_dep]
)