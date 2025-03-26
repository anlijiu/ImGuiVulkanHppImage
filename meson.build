project('test', 'c','cpp',
  version : '0.1',
  )


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
	'SDL_JOYSTICK': 'OFF',
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
sdl_proj = cmake.subproject('SDL', options: sdl_opts, required: true)
sdl_dep = sdl_proj.dependency('SDL3-static')
# sdl_proj = cmake.subproject('SDL')
# sdl_dep = sdl_proj.dependency('SDL')

volk_proj = cmake.subproject('volk')
volk_dep = volk_proj.dependency('volk')

vk_bootstrap_proj = cmake.subproject('vk-bootstrap')
vk_bootstrap_dep = vk_bootstrap_proj.dependency('vk-bootstrap')

vma_proj = cmake.subproject('VulkanMemoryAllocator')
vma_dep = vma_proj.dependency('VulkanMemoryAllocator')


sources = files('src/main.cpp')
