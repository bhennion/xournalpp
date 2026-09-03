i18n = import('i18n')
fs = import('fs')

# The source .desktop file
xournalpp_desktop_in_file = meson.project_source_root() / 'desktop/com.github.xournalpp.xournalpp.desktop.in'
# An array whose entries have a source xml file and the subdirectory of datadir to install it in
xournalpp_xml_in_files_data = [{'file' : meson.project_source_root() / 'resources-templates/com.github.xournalpp.xournalpp.appdata.xml.in', 'install_subdir' : 'metainfo'},
     {'file' : meson.project_source_root() / 'resources-templates/com.github.xournalpp.xournalpp.xml.in', 'install_subdir' : 'mime/packages'}]
# An array of source ini files
xournalpp_ini_in_files = [meson.project_source_root() / 'resources-templates/pagetemplates.ini.in',
     meson.project_source_root() / 'resources-templates/toolbar.ini.in']

xournalpp_xml_in_files = []
foreach d : xournalpp_xml_in_files_data
    xournalpp_xml_in_files += d['file']
endforeach

add_project_arguments(
    '-DGETTEXT_PACKAGE=' + meson.project_name(),
    '-DGETTEXT_PACKAGE_NAME=' + meson.project_name(),
    '-DGETTEXT_PACKAGE_VERSION=' + meson.project_version(),
    language:'c')
config.set('GETTEXT_PACKAGE', meson.project_name())
config.set('ENABLE_NLS', true)

xgettext_default_args = [
    '--package-name=' + meson.project_name(),
    '--package-version=' + meson.project_version(),
    '--escape',
    '--from-code=UTF-8',
    '--add-comments=TRANSLATORS:']

xournalpp_ini_pot = i18n.xgettext('inipot',
    xournalpp_ini_in_files,
    args : [xgettext_default_args, '--language=Desktop', '--keyword', '--keyword=name']
)

xournalpp_xml_pot = i18n.xgettext('xmlpot',
    xournalpp_xml_in_files,
    args : xgettext_default_args
)

find = find_program('find')

xournalpp_glade_pot = i18n.xgettext('gladepot',
    run_command(find, meson.project_source_root() / 'ui', '-name', '*.glade', '-o', '-name', '*.xml', check: true).stdout().strip().split('\n'),
    args : [xgettext_default_args, '--language=Glade', '--omit-header']
)

xournalpp_desktop_pot = i18n.xgettext('desktoppot',
    xournalpp_desktop_in_file,
    args : [xgettext_default_args, '--language=Desktop']
)

xournalpp_source_pot = i18n.xgettext('sourcepot',
    run_command(find, meson.project_source_root() / 'src', '-name', '*.cpp', check: true).stdout().strip().split('\n'),
    args : [xgettext_default_args, '--language=C++', '--keyword=_F', '--keyword=C_F:1c,2', '--keyword=_', '--keyword=N_', '--keyword=C_:1c,2', '--keyword=NC_:1c,2']
)

msgcat = find_program('msgcat')
xournalpp_pot = custom_target('xournalpp_pot',
    output: 'xournalpp.pot',
    depends: [xournalpp_ini_pot, xournalpp_xml_pot, xournalpp_glade_pot, xournalpp_desktop_pot, xournalpp_source_pot],
    command: [msgcat, '-o', meson.project_source_root() / 'po/xournalpp.pot', '--use-first', 'po/sourcepot.pot', 'po/inipot.pot', 'po/xmlpot.pot', 'po/gladepot.pot', 'po/desktoppot.pot']
)

find_program('sort')

xournalpp_languages = run_command(find, '.', '-name', '*.po', '-printf', '%f\n', check: true).stdout().strip().replace('.po','').split('\n')

xournalpp_languages_tgts = []
xournalpp_copy_po_tgts = []

msgfmt = find_program('msgfmt')

foreach lang : xournalpp_languages
    out = lang / 'LC_MESSAGES' / 'xournalpp.mo'
    xournalpp_languages_tgts += custom_target(
        lang + '.gmo',
        input : lang + '.po',
        output : 'xournalpp.mo',
        command : [msgfmt, '-o', meson.current_build_dir() / lang / 'xournalpp.mo', '@INPUT@'],
        build_subdir : lang,
        install : true,
        install_dir : get_option('localedir') / lang / 'LC_MESSAGES'
    )
    xournalpp_copy_po_tgts += fs.copyfile(lang + '.po')
endforeach

xournalpp_LINGUAS = custom_target('LINGUAS_file',
    command : ['echo', '\n'.join(xournalpp_languages)],
    capture : true,
    output : 'LINGUAS'
)

foreach xml_file : xournalpp_xml_in_files_data
    # We cannot use i18n.merge_file() because it does not support `depends`
    out = fs.stem(xml_file['file'])
    xournalpp_languages_tgts += custom_target(
        out,
        command : [msgfmt, '--xml', '--template=' + xml_file['file'], '-d', meson.current_build_dir(), '-o', meson.current_build_dir() / out],
        output : out,
        depends : xournalpp_copy_po_tgts + xournalpp_LINGUAS,
        install : true,
        install_dir : get_option('datadir') / xml_file['install_subdir']
    )
endforeach

foreach ini_file : xournalpp_ini_in_files
    # We cannot use i18n.merge_file() because it does not support `depends`
    out = fs.stem(ini_file)
    xournalpp_languages_tgts += custom_target(
        out,
        command : [msgfmt, '--desktop', '--keyword=name', '--template=' + ini_file, '-d', meson.current_build_dir(), '-o', meson.current_build_dir() / out],
        output : out,
        depends : xournalpp_copy_po_tgts + xournalpp_LINGUAS,
        install : true,
        install_dir : get_option('datadir') / meson.project_name() / 'ui'
    )
endforeach

if get_option('INSTALL_DESKTOP_FILES')
    # We cannot use i18n.merge_file() because it does not support `depends`
    out = fs.stem(xournalpp_desktop_in_file)
    xournalpp_languages_tgts += custom_target(
        out,
        command : [msgfmt, '--desktop', '--template=' + xournalpp_desktop_in_file, '-d', meson.current_build_dir(),  '-o', meson.current_build_dir() / out],
        output : out,
        depends : xournalpp_copy_po_tgts + xournalpp_LINGUAS,
        install : true,
        install_dir : get_option('datadir') / 'applications'
    )
endif

alias_target('translations', xournalpp_languages_tgts)
