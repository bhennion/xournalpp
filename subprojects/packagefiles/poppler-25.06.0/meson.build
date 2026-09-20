project('poppler', 'cpp', 'c',
      version: '25.06.0',
      meson_version: '>= 0.60.2'
)

gnome = import('gnome')

poppler_version = meson.project_version()
poppler_version_array = poppler_version.split('.')
poppler_major_version = poppler_version_array[0].to_int()
poppler_minor_version = poppler_version_array[1].to_int()
poppler_micro_version = poppler_version_array[2].to_int()

os_unix   = false
os_linux  = false
os_win32  = false
os_darwin = false

# Some windowing system backends depend on the platform we're
# building for, so we need to ensure they are disabled; in other
# cases, they are the only windowing system available, so we need
# to ensure they are enabled
if host_machine.system() == 'darwin'
  os_darwin = true
elif host_machine.system() == 'windows'
  os_win32 = true
elif host_machine.system() == 'linux'
  os_linux = true
endif
os_unix = not os_win32

poppler_prefix = get_option('prefix')
poppler_includedir = join_paths(poppler_prefix, get_option('includedir'))
poppler_libdir = join_paths(poppler_prefix, get_option('libdir'))
poppler_pkgconfigdir = join_paths(poppler_libdir, 'pkgconfig')

cc = meson.get_compiler('cpp')

cairo_dep = dependency('cairo')

all_poppler_deps = []
all_poppler_deps += cairo_dep
all_poppler_deps += dependency('freetype2')
all_poppler_deps += dependency('openjpeg')
all_poppler_deps += dependency('zlib')
all_poppler_deps += dependency('fontconfig')
all_poppler_deps += dependency('libtiff-4', fallback: 'libtiff')
all_poppler_deps += dependency('libpng')

all_poppler_glib_deps = []
all_poppler_glib_deps += dependency('glib-2.0', fallback: 'glib')
all_poppler_glib_deps += dependency('gobject-2.0')
all_poppler_glib_deps += dependency('gio-2.0')
all_poppler_glib_deps += cairo_dep
all_poppler_glib_deps += dependency('cairo-gobject', fallback: 'cairo')

cdata = configuration_data()
# Header checks
cdata.set('HAVE_DLFCN_H', cc.has_header('dlfcn.h'))
cdata.set('HAVE_FCNTL_H', cc.has_header('fcntl.h'))
cdata.set('HAVE_STDLIB_H', cc.has_header('stdlib.h'))
cdata.set('HAVE_SYS_MMAN_H', cc.has_header('sys/mman.h'))
cdata.set('HAVE_SYS_STAT_H', cc.has_header('sys/stat.h'))
cdata.set('HAVE_UNISTD_H', cc.has_header('unistd.h'))

cdata.set('HAVE_FSEEK64', cc.has_function('fseek64'))
cdata.set('HAVE_FSEEKO', cc.has_header_symbol('stdio.h', 'fseeko'))
cdata.set('HAVE_FTELL64', cc.has_function('ftell64'))
cdata.set('HAVE_PREAD64', cc.has_function('pread64'))
cdata.set('HAVE_LSEEK64', cc.has_function('lseek64'))
cdata.set('HAVE_GMTIME_R', cc.has_function('gmtime_r'))
cdata.set('HAVE_GETTIMEOFDAY', cc.has_function('gettimeofday'))
cdata.set('HAVE_LOCALTIME_R', cc.has_function('localtime_r'))
cdata.set('HAVE_POPEN', cc.has_function('popen'))
cdata.set('HAVE_MKSTEMP', cc.has_function('mkstemp'))
cdata.set('HAVE_STRTOK_R', cc.has_function('strtok_r'))

dir_code = '''
int main(int argc, char *argv[])
{
  DIR* d = 0;
  return 0;
}
'''
cdata.set('HAVE_DIRENT_H', cc.compiles('#include <dirent.h>\n' + dir_code, name : 'Header <dirent.h> defines DIR'))
cdata.set('HAVE_NDIR_H', cc.compiles('#include <ndir.h>\n' + dir_code, name : 'Header <ndir.h> defines DIR'))
cdata.set('HAVE_SYS_DIR_H', cc.compiles('#include <sys/dir.h>\n' + dir_code, name : 'Header <sys/dir.h> defines DIR'))
cdata.set('HAVE_SYS_NDIR_H', cc.compiles('#include <sys/ndir.h>\n' + dir_code, name : 'Header <sys/ndir.h> defines DIR'))

cdata.set('HAVE_NANOSLEEP', cc.has_function('nanosleep'))

# Enable these unconditionally.
cdata.set('OPI_SUPPORT', '1')
cdata.set('TEXTOUT_WORD_LIST', '1')

# Stubs
cdata.set('ICONV_CONST', '')
cdata.set('POPPLER_DATADIR', '')
cdata.set('POPPLER_VERSION', poppler_version)

# Features
cdata.set('ENABLE_LIBCURL', false)
cdata.set('ENABLE_LIBJPEG', false)
cdata.set('ENABLE_LIBOPENJPEG', true)
cdata.set('ENABLE_LIBTIFF', true)
cdata.set('ENABLE_LIBPNG', true)
cdata.set('ENABLE_ZLIB_UNCOMPRESS', false)
cdata.set('ENABLE_NSS3', false)
cdata.set('ENABLE_GPGME', false)
cdata.set('ENABLE_PGP_SIGNATURES', false)
cdata.set('ENABLE_SIGNATURES', false)
cdata.set('DEFAULT_SIGNATURE_BACKEND', 'None')
cdata.set('HAVE_CAIRO', true)
cdata.set('WITH_FONTCONFIGURATION_FONTCONFIG', true)
cdata.set('WITH_FONTCONFIGURATION_WIN32', false)
cdata.set('WITH_FONTCONFIGURATION_ANDROID', true)
cdata.set('ENABLE_PGP_SIGNATURES', false)
cdata.set('ENABLE_PGP_SIGNATURES', false)

configure_file(
  input: 'config.h.cmake',
  output: 'config.h',
  configuration: cdata,
  format: 'cmake',
)

pkgconfig_cdata = configuration_data()

pkgconfig_cdata.set('CMAKE_INSTALL_PREFIX', poppler_prefix)
pkgconfig_cdata.set('CMAKE_INSTALL_FULL_LIBDIR', poppler_libdir)
pkgconfig_cdata.set('CMAKE_INSTALL_FULL_INCLUDEDIR', poppler_includedir)
pkgconfig_cdata.set('POPPLER_VERSION', poppler_version)
pkgconfig_cdata.set('GLIB_REQUIRED', true)
pkgconfig_cdata.set('CAIRO_VERSION', cairo_dep.version())

# Configure "Requires" field & install .pc files for packagers
pkgconfig_cdata.set('PC_REQUIRES', '')
pkgconfig_cdata.set('PC_REQUIRES_PRIVATE', 'Requires.private: poppler = @0@'.format(poppler_version))

# put config.h in an internal directory so
# users don't include it by accident

subdir('xopp_customization')
conf_inc = include_directories('xopp_customization')

subdir('poppler')
subdir('glib')

#### Summary ####

summary('Compiler', cc.get_id(), section: 'Toolchain')
summary('Linker', cc.get_linker_id(), section: 'Toolchain')

# Directories
summary('prefix', poppler_prefix, section: 'Directories')
summary('includedir', poppler_includedir, section: 'Directories')
summary('libdir', poppler_libdir, section: 'Directories')
