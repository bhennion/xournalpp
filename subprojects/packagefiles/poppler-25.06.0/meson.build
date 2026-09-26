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

config = configuration_data()
config.set('HAVE_DLFCN_H', cc.has_header('dlfcn.h'))
config.set('HAVE_FCNTL_H', cc.has_header('fcntl.h'))
config.set('HAVE_STDLIB_H', cc.has_header('stdlib.h'))
config.set('HAVE_SYS_MMAN_H', cc.has_header('sys/mman.h'))
config.set('HAVE_SYS_STAT_H', cc.has_header('sys/stat.h'))
config.set('HAVE_UNISTD_H', cc.has_header('unistd.h'))
config.set('HAVE_FSEEK64', cc.has_function('fseek64'))
config.set('HAVE_FSEEKO', cc.has_header_symbol('stdio.h', 'fseeko'))
config.set('HAVE_FTELL64', cc.has_function('ftell64'))
config.set('HAVE_PREAD64', cc.has_function('pread64'))
config.set('HAVE_LSEEK64', cc.has_function('lseek64'))
config.set('HAVE_GMTIME_R', cc.has_function('gmtime_r'))
config.set('HAVE_GETTIMEOFDAY', cc.has_function('gettimeofday'))
config.set('HAVE_LOCALTIME_R', cc.has_function('localtime_r'))
config.set('HAVE_POPEN', cc.has_function('popen'))
config.set('HAVE_MKSTEMP', cc.has_function('mkstemp'))
config.set('HAVE_STRTOK_R', cc.has_function('strtok_r'))
config.set('HAVE_DIRENT_H', cc.has_header('dirent.h') and cc.has_header_symbol('dirent.h', 'DIR')
config.set('HAVE_NDIR_H', cc.has_header('ndir.h') and cc.has_header_symbol('ndir.h', 'DIR')
config.set('HAVE_SYS_DIR_H', cc.has_header('sys/dir.h') and cc.has_header_symbol('sys/dir.h', 'DIR')
config.set('HAVE_SYS_NDIR_H', cc.has_header('sys/ndir.h') and cc.has_header_symbol('sys/ndir.h', 'DIR')
config.set('HAVE_NANOSLEEP', cc.has_function('nanosleep'))

config.set('OPI_SUPPORT', '1')
config.set('TEXTOUT_WORD_LIST', '1')

config.set('ICONV_CONST', '')
config.set('POPPLER_DATADIR', '')
config.set('POPPLER_VERSION', meson.project_version())

# Features
config.set('ENABLE_LIBCURL', false)
config.set('ENABLE_LIBJPEG', false)
config.set('ENABLE_LIBOPENJPEG', true)
config.set('ENABLE_LIBTIFF', true)
config.set('ENABLE_LIBPNG', true)
config.set('ENABLE_ZLIB_UNCOMPRESS', false)
config.set('ENABLE_NSS3', false)
config.set('ENABLE_GPGME', false)
config.set('ENABLE_PGP_SIGNATURES', false)
config.set('ENABLE_SIGNATURES', false)
config.set('DEFAULT_SIGNATURE_BACKEND', 'None')
config.set('HAVE_CAIRO', true)
config.set('WITH_FONTCONFIGURATION_FONTCONFIG', true)
config.set('WITH_FONTCONFIGURATION_WIN32', false)
config.set('WITH_FONTCONFIGURATION_ANDROID', true)
config.set('ENABLE_PGP_SIGNATURES', false)
config.set('ENABLE_PGP_SIGNATURES', false)

configure_file(
  input: 'config.h.cmake',
  output: 'config.h',
  configuration: config,
  format: 'cmake',
)

subdir('poppler')
subdir('glib')
