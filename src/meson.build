# cmake_policy(['VERSION', '3.13'])
# Todo: add to target by generator expression
##############################
# xoj default option target
##############################

configure_file(input : 'config.h.meson.in', output : 'config.h', configuration: config)
configure_file(input : 'config-debug.h.meson.in', output : 'config-debug.h', configuration: config)
configure_file(input : 'config-dev.h.meson.in', output : 'config-dev.h', configuration: config)
configure_file(input : 'config-git.h.meson.in', output : 'config-git.h', configuration: config)
configure_file(input : 'config-paths.h.meson.in', output : 'config-paths.h', configuration: config)
configure_file(input : 'config-features.h.meson.in', output : 'config-features.h', configuration: config)

xournalpp_defaults = declare_dependency(
    include_directories : '.'
)
# if 'MSVC'
  # clang-cl.exe, cl.exe
  # target_compile_definitions(['defaults', 'INTERFACE', 'NOMINMAX', 'WIN32_LEAN_AND_MEAN'])
  # target_compile_options(['defaults', 'INTERFACE', '-utf-8'])
# else
  # gnu clang with gnu driver,
  # target_compile_options(['defaults', 'INTERFACE', '-Wall', '-Wreturn-type', '-Wuninitialized', '-Wunused-value', '-Wunused-variable', '-Wconversion'])
# endif
# Todo (support-v3.10): guard this variable with cmake_dependent_option for CMake 3.10, and NOT(GCC)
# then downgrade cmake_minimum_required to CMake 3.10
# if 'DEV_ENABLE_GCOV'
  # target_compile_options(['defaults', 'INTERFACE', '--coverage'])
  # target_link_options(['defaults', 'INTERFACE', '--coverage'])
  #cmake v3.13
# endif
##############################
# util
##############################
subdir('util')
##############################
# xournalpp-core
##############################
subdir('core')
##############################
# xournalpp-wrapper: catches crashes
##############################
# The crash dialog's ui file is stored as gresource to make sure the wrapper is standalone
gnome = import('gnome')
xournalpp_wrapper_resource = gnome.compile_resources(
    'xournalpp_wrapper_resource',
    'exe/xournalpp-wrapper.gresource.xml',
    source_dir : meson.project_source_root())

xournalpp_wrapper_exe = executable(
    'xournalpp-wrapper',
    sources : ['exe/Xournalpp-wrapper.cpp', xournalpp_wrapper_resource],
    dependencies: [xournalpp_defaults, xournalpp_util, xournalpp_external_modules],
    install : true)

##############################
# xournalpp main program
##############################
xournalpp_exe = executable(
    'xournalpp',
    sources : 'exe/Xournalpp.cpp',
    dependencies : [xournalpp_defaults, xournalpp_util, xournalpp_core, xournalpp_external_modules],
    install : true
    )
# target_link_libraries(['xournalpp', 'PRIVATE', 'xoj::core', 'xoj::util'])
# set_target_properties(['xournalpp', 'PROPERTIES', 'RUNTIME_OUTPUT_DIRECTORY', '${CMAKE_BINARY_DIR}', 'MACOSX_BUNDLE', '$<IF:$<PLATFORM_ID:Darwin>,TRUE,${CMAKE_MACOSX_BUNDLE}>', 'WIN32_EXECUTABLE', '$<IF:$<PLATFORM_ID:Windows>,TRUE,${CMAKE_WIN32_EXECUTABLE}>'])

###############################
# Generate icons
# Todo(fabian): move this into an own windows specific cmake script
# e.g. exe/win32/CMakeLists.txt
###############################
###############################
# Add MacOS specific files
# Todo(Roland): consider moving this into an own MacOS specific cmake script
# e.g. exe/macos/CMakeLists.txt
##############################
# if 'APPLE'
  # MacOS specific source files
  # file(['GLOB', 'xournalpp-sources-osx', 'exe/osx/*.cpp', 'exe/osx/*.h'])
  # target_sources(['xournalpp', 'PRIVATE', xournalpp-sources-osx])
  # target_include_directories(['xournalpp', 'PRIVATE', 'exe'])
# endif
# if 'WIN32'
#   # generate .ico from .png
#   ImageMagick_dep = dependency('ImageMagick')
#   icon_sizes = ['16', '32', '48', '256']
#   # foreach(['SIZE', 'IN', 'LISTS', 'ICON_SIZES'])
#   resized_icon = [project_binary_dir, '/xournalpp-icon-${SIZE}.png']
#   # add_custom_command(['OUTPUT', resized_icon, 'COMMAND', 'rsvg-convert', project_source_dir, '/ui/pixmaps/com.github.xournalpp.xournalpp.svg', '--width=${SIZE}', '--height=${SIZE}', '-o', resized_icon])
#   icon_deps = [icon_deps, resized_icon]
#   # unset('RESIZED_ICON')
#   # endforeach()
#   # add_custom_command(['OUTPUT', cmake_current_binary_dir, '/win32/xournalpp.ico', 'COMMAND', 'magick', 'convert', icon_deps, cmake_current_binary_dir, '/win32/xournalpp.ico', 'DEPENDS', icon_deps])
#   # unset('ICON_SIZES')
#   # unset('ICON_DEPS')
#   # add_custom_target(['xournalpp_icon', 'DEPENDS', cmake_current_binary_dir, '/win32/xournalpp.ico'])
#   # add_dependencies(['xournalpp', 'xournalpp_icon'])
#   # add_dependencies(['xournalpp-wrapper', 'xournalpp_icon'])
#   # windows specific source files
#   # target_sources(['xournalpp', 'PRIVATE', 'exe/win32/console.cpp'])
#   # target_sources(['xournalpp-wrapper', 'PRIVATE', 'exe/win32/console.cpp'])
#   if 'MSVC'
#     # target_sources(['xournalpp', 'PRIVATE', 'exe/win32/gui_main.cpp'])
#     # target_sources(['xournalpp-wrapper', 'PRIVATE', 'exe/win32/gui_main.cpp'])
#   endif
#   # windows specific appinfo
#   if 'NOT', 'MSVC'
#     cmake_rc_compile_object = '<CMAKE_RC_COMPILER> -O coff -i <SOURCE> -o <OBJECT>'
#     cmake_rc_compiler_init = 'windres'
#   endif
#   # enable_language('RC')
#   # configure_file(['exe/win32/xpp.rc.in', cmake_current_binary_dir, '/win32/xpp.rc'])
#   # target_sources(['xournalpp', 'PRIVATE', cmake_current_binary_dir, '/win32/xpp.rc'])
#   # target_sources(['xournalpp-wrapper', 'PRIVATE', cmake_current_binary_dir, '/win32/xpp.rc'])
# endif
# Until here
################################
subdir('xoj-preview-extractor')
