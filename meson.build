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


vulkan = not_found
libglslang = not_found

if host_os == 'windows'
  vulkan = declare_dependency(compile_args: ['-DVK_USE_PLATFORM_WIN32_KHR'])
elif host_os == 'linux'
  vulkan = dependency('vulkan')
endif

if vulkan.found()
  libglslang = cpp_compiler.find_library('glslang', required: false)
  if not libglslang.found()
    # FIXME: Get spirv-tools to enable opt.
    glslang_opts = cmake.subproject_options()
    glslang_opts.add_cmake_defines({'ENABLE_OPT': false, 'ENABLE_HLSL': false})
    glslang_subpro = cmake.subproject('glslang', options: glslang_opts)
    libglslang = declare_dependency(link_with: [
          glslang_subpro.target('glslang'),
          glslang_subpro.target('MachineIndependent'),
          glslang_subpro.target('GenericCodeGen'),
          glslang_subpro.target('SPIRV'),
        ], include_directories: glslang_subpro.include_directories('glslang')
      )
  endif

  volk_opts = cmake.subproject_options()
  volk_opts.add_cmake_defines({'VOLK_STATIC_DEFINES': 'VK_NO_PROTOTYPES'})
  if host_os == 'windows'
    volk_opts.append_compile_args('c', '-DVK_USE_PLATFORM_WIN32_KHR=1')
  endif
  volk_subproj = cmake.subproject('volk', options: volk_opts)
  volk_dep = declare_dependency(
    compile_args: ['-DVK_NO_PROTOTYPES'],
    include_directories: volk_subproj.include_directories('volk'),
    link_with: volk_subproj.target('volk'),
    dependencies: vulkan)
endif

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

fmt_dep = cmake.subproject('fmt').get_variable('fmt_dep')

glfw_proj = cmake.subproject('glfw')
glfw_dep = glfw_proj.dependency('glfw')


vk_bootstrap_opts = cmake.subproject_options()
vk_bootstrap_opts.add_cmake_defines({'VK_BOOTSTRAP_TEST': false})
vk_bootstrap_proj = cmake.subproject('vk-bootstrap', options: vk_bootstrap_opts)
vk_bootstrap_dep = vk_bootstrap_proj.dependency('vk-bootstrap')

# message('CMake targets:\n - ' + '\n - '.join(cmake.subproject('VulkanMemoryAllocator-Hpp').target_list()))

vma_dep = cmake.subproject('VulkanMemoryAllocator').dependency('VulkanMemoryAllocator')

glm_proj = cmake.subproject('glm')
glm_dep = glm_proj.dependency('glm')


app_deps = [
  fmt_dep,
  vma_dep,
  vk_bootstrap_dep,
  volk_dep,
  glfw_dep,
  glm_dep,
]

sources = files('src/main.cpp')

executable('m',
  sources,
  include_directories : incdir,
  dependencies : app_deps,
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
