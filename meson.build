# cmake_policy(['VERSION', cmake_version])
## The CMAKE_OSX_DEPLOYMENT_TARGET must be set before project(), so don't move the following lines further down.
# # # if 'APPLE'
# # #   if 'NOT', 'CMAKE_OSX_DEPLOYMENT_TARGET'
# # #     # Use the same target as the dependencies shipped by the CI
# # #     # file(['STRINGS', 'mac-setup/jhbuild-version.lock', '_DEPLOY_TARGET', 'REGEX', '^deployment_target=.*'])
# # #     # string(['REGEX', 'REPLACE', '^[^=]*=', '', '_DEPLOY_TARGET_VALUE', _deploy_target])
# # #     cmake_osx_deployment_target = ['${_DEPLOY_TARGET_VALUE}', 'CACHE', 'STRING', 'Minimum OS X deployment version', 'FORCE']
# # #   endif
# # #   # message(['STATUS', 'MacOS X Deployment target: ${CMAKE_OSX_DEPLOYMENT_TARGET}'])
# # # endif
# # # if 'NOT', 'CMAKE_BUILD_TYPE'
# # #   # set default build type to RelWithDebInfo, we never supported multi config generators, therefore this is fine
# # #   cmake_build_type = ['RelWithDebInfo', 'CACHE', 'STRING', 'Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel.', 'FORCE']
# # # endif
## Project related Variables
## Also update changelog in debian folder!
project(
    'xournalpp',
    'cpp', 'c',
    version : '1.3.7+dev',
    meson_version : '>= 1.10.0',
    default_options : ['default_library=static', 'cpp_std=c++20', 'buildtype=debugoptimized']
    )

add_project_arguments('-DGDK_DISABLE_DEPRECATED', '-DGTK_DISABLE_DEPRECATED', language : 'c')

config = configuration_data()
config.set('PROJECT_NAME', meson.project_name())
config.set('PROJECT_VERSION', meson.project_version())
config.set('PROJECT_HOMEPAGE_URL', 'https://xournalpp.github.io/')

# string(['TIMESTAMP', 'PACKAGE_TIMESTAMP', '%Y%m%d.%H%M', 'UTC'])

# # # package_arch = cmake_system_processor
# # # if 'UNIX', 'AND', 'NOT', 'APPLE', 'AND', 'NOT', 'CYGWIN'
# # #   install_desktop_files = 'true'
# # # endif


## I18n ##
subdir('po')

# # # if 'ONLY_CONFIGURE_FOR_TARGET_POT'
# # #   # We dont need anything else for just generating the .pot file.
# # #   # This avoids having to install all the dependencies in the CI runs updating the translation files
# # #   # return()
# # # endif

# configure_file(['cmake/postinst.in', 'cmake/postinst', '@ONLY'])


################################
## Libraries ##
##############################
# option(ENABLE_CPPTRACE "Compile with cpptrace stacktrace library" ON)
# # Requires fetching content during build.
# if (CMAKE_BUILD_TYPE STREQUAL "Release")
#   set(ENABLE_CPPTRACE OFF)
#   message(STATUS "Disable cpptrace by lack of debug symbols in Release mode")
# endif ()
#
# if (ENABLE_CPPTRACE)
#     include(FetchContent)
#     FetchContent_Declare(
#         cpptrace
#         GIT_REPOSITORY https://github.com/jeremy-rifkin/cpptrace.git
#         GIT_TAG        v1.0.4 # <HASH or TAG>
#         EXCLUDE_FROM_ALL
#     )
#     set(FETCHCONTENT_UPDATES_DISCONNECTED ON) # Prevent reloading if already downloaded
#     set(CPPTRACE_BUILD_SHARED OFF)
#     set(CPPTRACE_USE_EXTERNAL_ZSTD ON)
#     if (UNIX)
#         # Otherwise uses libdwarf - adding yet another dependency
#         set(CPPTRACE_GET_SYMBOLS_WITH_LIBDL ON)
#     endif()
#     if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.28")
#         FetchContent_MakeAvailable(cpptrace)
#     else()
#         # FetchContent_Declare option EXCLUDE_FROM_ALL was only introduced in cmake 3.28
#         # Workaround it for older versions (ubuntu 22 has cmake 3.22)
#         FetchContent_GetProperties(cpptrace)
#         if(NOT cpptrace_POPULATED)
#             FetchContent_Populate(cpptrace)
#             add_subdirectory(${cpptrace_SOURCE_DIR} ${cpptrace_BINARY_DIR} EXCLUDE_FROM_ALL)
#         endif()
#     endif()
# endif()
##############################
# other externals
##############################
deps = []
deps += dependency('glib-2.0', version : '>= 2.32.0')
deps += dependency('gtk+-3.0', version : '>= 3.18.9')
deps += dependency('poppler-glib', version : '>= 0.41.0')
if host_machine.system() == 'darwin'
  deps += dependency('poppler-cpp', version : '>= 0.41.0')
endif
deps += dependency('gthread-2.0', version : '>= 2.4.0')
deps += dependency('libxml-2.0', version : '>= 2.0.0')
deps += dependency('libzip', version : '>= 1.0.1')
deps += dependency('librsvg-2.0', version : '>= 2.46')
# Profiling
if get_option('ENABLE_PROFILING')
  config.set('ENABLE_PROFILING', true)
  deps += dependency('libprofiler', version : '>= 2.5')
  deps += dependency('libtcmalloc', version : '>= 2.5')
endif
ZLIB_dep = dependency('zlib')
Threads_dep = dependency('threads')
if get_option('ENABLE_AUDIO')
  config.set('ENABLE_AUDIO', true)
  # message(['STATUS', 'Enable Audio support'])
  deps += dependency('portaudiocpp', version : '>= 12')
  deps += dependency('sndfile', version : '>= 1.0.25')
else
  # message(['STATUS', 'Disable Audio support'])
endif

if get_option('ENABLE_QPDF')
  config.set('ENABLE_QPDF', true)
  deps += dependency('libqpdf', modules : ['qpdf::libqpdf'], version : '>= 10.6.0')
  # message(['STATUS', 'Found qpdf: ${qpdf_VERSION}'])
endif

if get_option('ENABLE_PLUGINS')
  config.set('ENABLE_PLUGINS', true)
  # message(['STATUS', 'Enable Xournal++ Plugins'])
  deps += dependency('Lua')
else
  # message(['STATUS', 'Disable Xournal++ Plugins'])
endif
if get_option('ENABLE_GTK_SOURCEVIEW')
  config.set('ENABLE_GTK_SOURCEVIEW', true)
  deps += dependency('gtksourceview-4', version : '>= 4.0.0')
  # message(['STATUS', 'Enable GtkSourceView TeX syntax highlighting'])
else
  # message(['STATUS', 'Disable GtkSourceView TeX syntax highlighting'])
endif

if get_option('ENABLE_X11')
  config.set('ENABLE_X11', true)
  # message(['STATUS', 'Enable Xournal++ Plugins'])
  deps += dependency('x11')
endif

HAVE_FLOAT_FROM_CHARS = meson.get_compiler('cpp').compiles('''
     #include <charconv>
     #include <string_view>
     #include <system_error>
     int main() {
         const std::string_view sv = "7.381450";
         double val{};
         auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), val);
         return ptr != (sv.data() + sv.size()) || ec != std::errc{};
     }
''')
if HAVE_FLOAT_FROM_CHARS
    if get_option('ENABLE_FLOAT_FROM_CHARS')
        config.set10('ENABLE_FLOAT_FROM_CHARS', true)
#         message(STATUS "Enable floating point std::from_chars")
    else
        config.set10('ENABLE_FLOAT_FROM_CHARS', false)
#         message(WARNING "Floating point std::from_chars is disabled, but available. "
#                         "To avoid performance penalty, set ENABLE_FLOAT_FROM_CHARS to ON.")
    endif
else
    config.set10('ENABLE_FLOAT_FROM_CHARS', false)
    if get_option('ENABLE_FLOAT_FROM_CHARS')
#         message(SEND_ERROR "Floating point std::from_chars is not supported. Use a newer standard library, "
#                            "or set ENABLE_FLOAT_FROM_CHARS to OFF to fall back to g_ascii_strtod. "
#                            "std::from_chars offers higher performance and should be used whenever possible.")
    else
#         message(STATUS "Disable floating point std::from_chars")
    endif
endif

xournalpp_external_modules = declare_dependency(
    dependencies : deps,
)

# target_link_libraries(['external_modules', 'INTERFACE', 'PkgConfig::ExternalModules', '$<$<TARGET_EXISTS:cpptrace::cpptrace>:cpptrace::cpptrace>',
# target_compile_definitions(['external_modules', 'INTERFACE', 'GLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_2_40', 'GDK_VERSION_MIN_REQUIRED=GDK_VERSION_3_18', '$<$<TARGET_EXISTS:X11::X11>:X11_ENABLED>'])
# target_link_options(['external_modules', 'INTERFACE', '$<$<AND:$<NOT:$<PLATFORM_ID:Windows>>,$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>>:-rdynamic>'])

# add_compile_definitions(['G_LOG_DOMAIN=', 'xopp'])


# Advanced development config
config.set('DEV_TOOLBAR_CONFIG', 'toolbar.ini') #, 'CACHE', 'STRING', 'Toolbar config file name']
config.set('DEV_SETTINGS_XML_FILE', 'settings.xml') #, 'CACHE', 'STRING', 'Settings file name']
config.set('DEV_DEFAULT_PALETTE_FILE', 'xournal.gpl') #, 'CACHE', 'STRING', 'Default Color Palette']
config.set('DEV_PRINT_CONFIG_FILE', 'print-config.ini') #, 'CACHE', 'STRING', 'Print config file name']
config.set('DEV_METADATA_FILE', 'metadata.ini') #, 'CACHE', 'STRING', 'Metadata file name']
config.set('DEV_ERRORLOG_DIR', 'errorlogs') #, 'CACHE', 'STRING', 'Directory where errorlogfiles will be placed']
config.set('DEV_FILE_FORMAT_VERSION', 5) #, 'CACHE', 'STRING', 'File format version', 'FORCE']

# Get git info
git = find_program('git', required : false)
fs = import('fs')
if git.found() and fs.exists(meson.project_source_root() / '.git')
    config.set('GIT_BRANCH', run_command(git, 'rev-parse', '--abbrev-ref', 'HEAD', check : true).stdout().strip())
    xournalpp_git_origin = run_command(git, 'remote', 'get-url', 'origin', check : true).stdout().strip()

    config.set('GIT_ORIGIN_URL', xournalpp_git_origin)
    xournalpp_git_origin_components = xournalpp_git_origin.split(':')[1].split('/')
    message('xournalpp_git_origin_components: ', xournalpp_git_origin_components)

    config.set('GIT_ORIGIN_OWNER', xournalpp_git_origin_components[-2])
    config.set('GIT_ORIGIN_REPO', xournalpp_git_origin_components[-1].split('.')[-2])

    xournalpp_commit_hash = run_command(git, 'rev-parse', '--short=8', 'HEAD', check : true).stdout().strip()
    if run_command(git, 'diff-index', '--quiet', 'HEAD', check : false).returncode() != 0
        xournalpp_commit_hash += '-dirty'
    endif

    config.set('RELEASE_IDENTIFIER', xournalpp_commit_hash)
else
    config.set('RELEASE_IDENTIFIER', meson.project_version())
endif

config.set('PROJECT_CRASHREPORT', 'https://github.com/xournalpp/xournalpp/issues/new?template=crash_report.yml')

# Path configuration
config.set('PROJECT_INSTALL_DIR', get_option('prefix'))

# Generate VERSION file for packaging scripts
config.set('PACKAGE_ARCH', host_machine.cpu_family())
configure_file(
    input : 'cmake/VERSION.in',
    configuration : config,
    format : 'cmake@',
    output : 'VERSION'
)

## Source building ##
subdir('src')



## Tests ##
# # # if 'ENABLE_GTEST'
# # #   subdir('test')
# # # endif


## Man page generation ##
subdir('man')

# Install resources
install_subdir('ui', install_dir: get_option('datadir') / meson.project_name())
install_subdir('plugins', install_dir: get_option('datadir') / meson.project_name())
install_subdir('resources', install_dir: get_option('datadir') / meson.project_name())
install_subdir('palettes', install_dir: get_option('datadir') / meson.project_name())
# Install desktop shortcuts for Linux
if get_option('INSTALL_DESKTOP_FILES')
  install_data('ui/pixmaps/com.github.xournalpp.xournalpp.svg', install_dir : get_option('datadir') / 'icons/hicolor/scalable/apps')
  install_data('ui/pixmaps/application-x-xopp.svg', install_dir : get_option('datadir') / 'icons/hicolor/scalable/mimetypes')
  install_data('ui/pixmaps/application-x-xopt.svg', install_dir : get_option('datadir') / 'icons/hicolor/scalable/mimetypes/')
  install_data('ui/pixmaps/application-x-xojpp.svg', install_dir : get_option('datadir') / 'icons/hicolor/scalable/mimetypes/')
  install_data('ui/pixmaps/gnome-mime-application-x-xopp.svg', install_dir : get_option('datadir') / 'icons/hicolor/scalable/mimetypes/')
  install_data('ui/pixmaps/gnome-mime-application-x-xopt.svg', install_dir : get_option('datadir') / 'icons/hicolor/scalable/mimetypes/')
  install_data('desktop/com.github.xournalpp.xournalpp.thumbnailer', install_dir : get_option('datadir') / 'thumbnailers')
endif
# Uninstall target
# configure_file(['cmake/cmake_uninstall.cmake.in', 'cmake/cmake_uninstall.cmake', '@ONLY'])
# add_custom_target(['uninstall', 'COMMAND', cmake_command, '-P', cmake_current_binary_dir, '/cmake/cmake_uninstall.cmake', 'COMMENT', 'Uninstall entire Xournal++'])
# message('
# Configuration:
#     Compiler:                   ${CMAKE_CXX_COMPILER}
#     X11 support enabled:        ${X11_FOUND}
#     GTEST enabled:              ${ENABLE_GTEST}
#     GCOV enabled:               ${DEV_ENABLE_GCOV}
#     Profiling enabled:          ${ENABLE_PROFILING}
# ')
##############################
# Packaging options
# !!! only override defaults if necessary !!!
###
# Defaults:
# CPACK_PACKAGE_NAME
# CPACK_PACKAGE_VERSION
# CPACK_PACKAGE_DESCRIPTION_SUMMARY
# ...
##############################
# set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
# set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION}~git${PACKAGE_TIMESTAMP}-${RELEASE_IDENTIFIER}-${DISTRO_CODENAME})
# set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/package_description")
# set(CPACK_PACKAGE_CONTACT "${PROJECT_CONTACT}")
# set(CPACK_PACKAGE_INSTALL_DIRECTORY "Xournal++")
# set(CPACK_PACKAGE_FILE_NAME "xournalpp-${PROJECT_VERSION}-${DISTRO_NAME}-${DISTRO_CODENAME}-${PACKAGE_ARCH}")
# set(CPACK_OUTPUT_FILE_PREFIX packages)
# set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
# set(CPACK_STRIP_FILES ON) # Debian packages must be stripped from debug messages, so lintian doesn't complain
#
# # .deb package options
# set(CPACK_DEBIAN_PACKAGE_RELEASE 2)
# set(CPACK_DEBIAN_PACKAGE_VERSION "${CPACK_PROJECT_VERSION}")
# set(CPACK_DEBIAN_PACKAGE_SECTION "graphics")
# set(_LIBXML2_DEP "libxml2 (>= 2.0.0)")
# if (CPACK_GENERATOR MATCHES "DEB")
#     pkg_check_modules(LIBXML2 libxml-2.0)
#     if (LIBXML2_VERSION VERSION_GREATER_EQUAL "2.14")
#         set(_LIBXML2_DEP "libxml2-16 (>= 2.0.0)")
#     endif ()
# endif ()
# set(CPACK_DEBIAN_PACKAGE_DEPENDS
#         "libglib2.0-0 (>= 2.32), libgtk-3-0 (>= 3.18), libpoppler-glib8 (>= 0.41.0), ${_LIBXML2_DEP}, libportaudiocpp0 (>= 12), libsndfile1 (>= 1.0.25), liblua5.3-0 | liblua5.4-0, libzip4 (>= 1.0.1) | libzip5, zlib1g, libc6, qpdf (>= 10.6.0), libgtksourceview-4-0")
# set(CPACK_DEBIAN_PACKAGE_SUGGESTS "texlive-base, texlive-latex-extra")  # Latex tool
# # Use debian's arch scheme; we only care about x86/amd64 for now but feel free to add more
# if (${PACKAGE_ARCH} STREQUAL "x86_64")
#     set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
# endif ()
#
# if (CPACK_GENERATOR MATCHES "DEB")
#     message("Preparing documentation for DEB package")
#     add_custom_target(package_documentation ALL)
#
#     #Compress changelog and save it as share/doc/xournalpp/changelog.Debian.gz
#     add_custom_command(TARGET package_documentation PRE_BUILD
#             COMMAND gzip -c -9 -n "${PROJECT_SOURCE_DIR}/debian/changelog" > "changelog.Debian.gz" VERBATIM)
#     install(FILES "${CMAKE_CURRENT_BINARY_DIR}/changelog.Debian.gz" DESTINATION "share/doc/xournalpp/")
#
#     message("Install copyright for DEB package")
#     #Copy copyright to share/doc/xournalpp/copyright
#     install(FILES "${PROJECT_SOURCE_DIR}/debian/copyright" DESTINATION "share/doc/xournalpp/")
# endif ()
#
# if (NOT DEFINED CPACK_GENERATOR)
#     set(CPACK_GENERATOR "TGZ")
# endif ()
#
# include(CPack)
