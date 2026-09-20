glib_conf = configuration_data()

glib_conf.set('CAIRO_FEATURE', '#define POPPLER_HAS_CAIRO 1')
glib_conf.set('POPPLER_MAJOR_VERSION', poppler_major_version)
glib_conf.set('POPPLER_MINOR_VERSION', poppler_minor_version)
glib_conf.set('POPPLER_MICRO_VERSION', poppler_micro_version)

configure_file(
  input: 'poppler-features.h.cmake',
  output: 'poppler-features.h',
  configuration: glib_conf,
  format: 'cmake@',
)
poppler_glib_public_headers = files(
  'poppler-media.h',
  'poppler-layer.h',
  'poppler-cached-file-loader.h',
  'poppler-form-field.h',
  'poppler.h',
  'poppler-annot.h',
  'poppler-action.h',
  'poppler-structure-element.h',
  'poppler-attachment.h',
  'poppler-document.h',
  'poppler-private.h',
  'poppler-input-stream.h',
  'poppler-date.h',
  'poppler-page.h',
  'poppler-movie.h',
)

poppler_enums = gnome.mkenums('poppler-enums',
  sources: poppler_glib_public_headers,
  c_template: 'poppler-enums.c.template',
  h_template: 'poppler-enums.h.template',
)

poppler_glib_sources = files(
  'poppler-action.cc',
  'poppler-cached-file-loader.cc',
  'poppler.cc',
  'poppler-media.cc',
  'poppler-layer.cc',
  'poppler-document.cc',
  'poppler-input-stream.cc',
  'poppler-annot.cc',
  'poppler-form-field.cc',
  'poppler-date.cc',
  'poppler-structure-element.cc',
  'poppler-attachment.cc',
  'poppler-page.cc',
  'poppler-movie.cc',
)

poppler_glib_generated_sources = files([
  '../poppler/CairoFontEngine.cc',
  '../poppler/CairoOutputDev.cc',
  '../poppler/CairoRescaleBox.cc',
])
poppler_glib_generated_sources += poppler_enums
poppler_glib_sources += poppler_glib_generated_sources

poppler_glib = static_library('poppler-glib',
  poppler_glib_sources,
  dependencies: [all_poppler_glib_deps, poppler_dep],
  include_directories: include_directories('.', '../goo', '../poppler', '..'),
)

poppler_glib_dep = declare_dependency(
  link_with: poppler_glib,
  include_directories : include_directories('.'),
)

meson.override_dependency('poppler-glib', poppler_glib_dep)
