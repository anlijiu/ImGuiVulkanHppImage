project('test', 'c','cpp',
  version : '0.1',
  )

cpp_compiler = meson.get_compiler('cpp')
c_compiler = meson.get_compiler('c')
not_found = dependency('', required: false)
host_os = host_machine.system()
bt = get_option('buildtype')

if bt == 'debug'
	if meson.get_compiler('c').get_id() == 'msvc'
		add_project_arguments(['/D_ITERATOR_DEBUG_LEVEL=2'], language: ['c', 'cpp'])
	else
		add_project_arguments(['-D_ITERATOR_DEBUG_LEVEL=2'], language: ['c', 'cpp'])
	endif
endif

if meson.get_compiler('c').get_id() != 'msvc'
	add_project_arguments(['-Wno-nullability-completeness'], language: ['c', 'cpp'])
endif

cmake_build_type = ''
cmake_msvc_lib = 'MultiThreaded'
if bt.contains('debug')
	if bt == 'debug'
		cmake_build_type = 'Debug'
		cmake_msvc_lib += 'Debug'
	else
		cmake_build_type = 'RelWithDebInfo'
	endif
elif bt == 'plain' or bt == 'minsize'
	cmake_build_type = 'MinSizeRel'
elif bt == 'release'
	cmake_build_type = 'Release'
else
	error('Unsupported build type!')
endif
cmake_msvc_lib += 'DLL'

cpp_args = [
  '-DVULKAN_HPP_NO_NODISCARD_WARNINGS',
  '-DVULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1',
  '-DVMA_VULKAN_VERSION=1003000',
  '-DVMA_STATIC_VULKAN_FUNCTIONS=0',
  '-DVMA_DYNAMIC_VULKAN_FUNCTIONS=1',
  '-DVK_NO_PROTOTYPES',
  '-DGLM_ENABLE_EXPERIMENTAL'
]

# glslc = find_program('glslc')
# 
# glslc_text_gen = generator(glslc, arguments: ['-w', '-mfmt=c', '-o', '@OUTPUT@', '@INPUT@'], output: '@PLAINNAME@.txt')
# core_shaders_s = glslc_text_gen.process([
# 	'cacao/shaders/skybox.vert', 
# 	'cacao/shaders/skybox.frag',
# 	'cacao/shaders/uiquad.vert', 
# 	'cacao/shaders/uiquad.frag',
# 	'cacao/shaders/text.vert',
# 	'cacao/shaders/text.frag',
# 	'cacao/shaders/image.vert',
# 	'cacao/shaders/image.frag'
# ])
# 
# core_shaders = declare_dependency(sources: [
# 	core_shaders_s
# ])





incdir = include_directories('src')

cmake = import('cmake')


VulkanSDK = '/home/an/VulkanSDK/1.4.309.0/x86_64/'

vulkan_dep = declare_dependency(
    include_directories : include_directories(
      VulkanSDK + 'include',
      VulkanSDK + 'include/vulkan',
      VulkanSDK + 'include/volk',
      VulkanSDK + 'include/glslang',
      VulkanSDK + 'include/spirv',
    ),
    version : '1.4',
    link_args : [
    '-L' + VulkanSDK + 'lib' ,
    '-lvulkan',
    '-lglslang',
    '-lvolk',
    '-lMachineIndependent',
    '-lGenericCodeGen',
    '-lSPIRV',
    ]
)

# if host_os == 'windows'
#   vulkan = declare_dependency(compile_args: ['-DVK_USE_PLATFORM_WIN32_KHR'])
# elif host_os == 'linux'
#   vulkan = dependency('vulkan')
# endif

# volk_opts = cmake.subproject_options()
# volk_opts.add_cmake_defines({'VOLK_STATIC_DEFINES': 'VK_NO_PROTOTYPES'})
# if host_os == 'windows'
#   volk_opts.append_compile_args('c', '-DVK_USE_PLATFORM_WIN32_KHR=1')
# endif
# 
# volk_subproj = cmake.subproject('volk', options: volk_opts)
# volk_dep = declare_dependency(
#   compile_args: ['-DVK_NO_PROTOTYPES', '-DVOLK_STATIC_DEFINES'],
#   include_directories: volk_subproj.include_directories('volk'),
#   link_with: volk_subproj.target('volk'),
#   dependencies: vulkan_dep)

sdl_opts = cmake.subproject_options()
sdl_opts.add_cmake_defines({
	'SDL_TESTS': 'OFF',
	'SDL_SHARED': 'OFF',
	'SDL_STATIC': 'ON',
	'SDL_STATIC_PIC': 'ON',
	'SDL_TEST_LIBRARY': 'OFF',
	'SDL_AUDIO': 'OFF',
	'SDL_VIDEO': 'ON',
	'SDL_RENDER': 'OFF',
	'SDL_CAMERA': 'OFF',
	'SDL_JOYSTICK': 'ON',

    'SDL_RENDER_GPU'             : 'ON',
    'SDL_RENDER_VULKAN'          : 'ON',
	'SDL_HAPTIC': 'OFF',
	'SDL_HIDAPI': 'OFF',
	'SDL_POWER': 'OFF',
	'SDL_SENSOR': 'OFF',
	'SDL_DIALOG': 'OFF',
	'CMAKE_POSITION_INDEPENDENT_CODE': 'ON',
	'SDL_WAYLAND': 'ON',
	'SDL_WAYLAND_SHARED': 'ON',
	'SDL_WAYLAND_LIBDECOR': 'ON',
	'SDL_WAYLAND_LIBDECOR_SHARED': 'ON',
	'SDL_METAL': 'OFF',
	'SDL_VULKAN': 'ON',
	'SDL_X11': 'ON',
	'SDL_X11_SHARED': 'ON',
	'CMAKE_BUILD_TYPE': cmake_build_type,
	'CMAKE_MSVC_RUNTIME_LIBRARY': cmake_msvc_lib,
	'CMAKE_POLICY_DEFAULT_CMP0091': 'NEW'
})
sdl_opts.append_compile_args('c', '-Dhid_darwin_set_open_exclusive(x)=;')
# sdl_proj = cmake.subproject('SDL', options: sdl_opts, required: true)
# sdl_dep = sdl_proj.dependency('SDL3')

stb_dep = subproject('stb').get_variable('stb_dep')

fmt_dep = cmake.subproject('fmt').get_variable('fmt_dep')

glfw_proj = cmake.subproject('glfw')
glfw_dep = glfw_proj.dependency('glfw')

# use std::format over bundled fmt::format
cmake_opt_var = cmake.subproject_options()
# cmake_opt_var.add_cmake_defines({'SPDLOG_FMT_EXTERNAL': 'On'})
# cmake_opt_var.add_cmake_defines({'CMAKE_PREFIX_PATH': meson.project_build_root() / 'subprojects' / 'fmt' / '__CMake_build'})
spdlog_proj = cmake.subproject('spdlog', required: true, options: cmake_opt_var)
spdlog_dep = spdlog_proj.dependency('spdlog')

tracy_dep = subproject('tracy').get_variable('tracy_dep')

freetype2_dep = subproject('freetype2').get_variable('freetype_dep')

imgui_inc = include_directories(
  './externals/imgui',
  './externals/imgui/backends',
  './externals/imgui/misc/cpp',
  './externals/imgui/misc/freetype'
)
imgui_sources = [
  './externals/imgui/imgui.cpp',
  './externals/imgui/imgui_draw.cpp',
  './externals/imgui/imgui_widgets.cpp',
  './externals/imgui/imgui_demo.cpp',
  './externals/imgui/imgui_tables.cpp',
  './externals/imgui/backends/imgui_impl_glfw.cpp',
  './externals/imgui/backends/imgui_impl_vulkan.cpp',
  './externals/imgui/misc/cpp/imgui_stdlib.cpp',
  './externals/imgui/misc/freetype/imgui_freetype.cpp',
]

imgui_dll_defs = ['-DIMGUI_API=__attribute__((visibility("default")))']
if host_machine.system() == 'windows'
  imgui_dll_defs += '-DIMGUI_API=@0@'.format(get_option('default_library') != 'static' ? '__declspec(dllexport)' : '')
endif
imgui = static_library('imgui',
                  imgui_sources,
                  include_directories: imgui_inc,
                  install: false,
                  cpp_args: imgui_dll_defs,
                  gnu_symbol_visibility: 'inlineshidden',
                  dependencies: [vulkan_dep, glfw_dep, freetype2_dep ]
                  )

imgui_dep = declare_dependency(link_with: imgui, include_directories: imgui_inc)

vk_bootstrap_opts = cmake.subproject_options()
vk_bootstrap_opts.add_cmake_defines({'VK_BOOTSTRAP_TEST': false})
vk_bootstrap_proj = cmake.subproject('vk-bootstrap', options: vk_bootstrap_opts)
vk_bootstrap_dep = vk_bootstrap_proj.dependency('vk-bootstrap')

# message('CMake targets:\n - ' + '\n - '.join(cmake.subproject('VulkanMemoryAllocator-Hpp').target_list()))

# vma_dep = cmake.subproject('VulkanMemoryAllocator').dependency('VulkanMemoryAllocator')

glm_opts = cmake.subproject_options()
glm_opts.add_cmake_defines({'GLM_ENABLE_EXPERIMENTAL': true})
glm_proj = cmake.subproject('glm', options: glm_opts)
glm_dep = glm_proj.dependency('glm')


app_deps = [
  vulkan_dep,
  fmt_dep,
  # vma_dep,
  vk_bootstrap_dep,
  # volk_dep,
  glfw_dep,
  glm_dep,
  tracy_dep,
  imgui_dep,
  spdlog_dep,
  stb_dep,
]

sources = files(
  'src/main.cpp',
  'src/Init.cpp',
  'src/Util.cpp',
  'src/Color.cpp',
)

executable('m',
  sources,
  include_directories : incdir,
  dependencies : app_deps,
  cpp_args: cpp_args
)

Asources = files('src/mainA.cpp')

executable('m1',
  Asources,
  include_directories : incdir,
  dependencies : app_deps,
)

Bsources = files('src/mainB.cpp')

executable('m2',
  Bsources,
  include_directories : incdir,
  dependencies : app_deps,
)


Csources = files('src/test1/main.cpp')
executable('m3',
  Csources,
  include_directories : incdir,
  dependencies : app_deps,
)

Dsources = files('src/test2/volk_init.cpp', 'src/test2/main.cpp')
executable('m4',
  Dsources,
  include_directories: include_directories('src/test2', 'src/test2/TinyVulkan'),
  dependencies : app_deps,
)

Esources = files(
  'src/test3/app.cpp',
  'src/test3/lve_device.cpp',
  'src/test3/lve_model.cpp',
  'src/test3/lve_pipeline.cpp',
  'src/test3/lve_swap_chain.cpp',
  'src/test3/lve_window.cpp',
  'src/test3/main.cpp',
  'src/test3/volk_init.cpp'
)
executable('m5',
  Esources,
  include_directories: include_directories('src/test3'),
  dependencies : app_deps,
)


Fsources = files(
  'src/test4/Math/IndexRange2.cpp',
  'src/test4/Math/Transform.cpp',
  'src/test4/Math/Util.cpp',
  'src/test4/VolkInit.cpp',
  'src/test4/VmaInit.cpp',
  'src/test4/GfxDevice.cpp',
  'src/test4/BindlessSetManager.cpp',
  'src/test4/Util.cpp',
  'src/test4/Init.cpp',
  'src/test4/Swapchain.cpp',
  'src/test4/MeshPipeline.cpp',
  'src/test4/FrustumCulling.cpp',
  'src/test4/ImageCache.cpp',
  'src/test4/ImageLoader.cpp',
  'src/test4/MipMapGeneration.cpp',
  'src/test4/VulkanImmediateExecutor.cpp',
  'src/test4/Camera.cpp',
  'src/test4/MeshCache.cpp',
  'src/test4/Pipelines.cpp',
  'src/test4/Renderer.cpp',
  'src/test4/App.cpp',
  'src/test4/main.cpp',
)


executable('m6',
  Fsources,
  include_directories: include_directories('src/test4'),
  dependencies : app_deps,
  cpp_args: cpp_args
)
