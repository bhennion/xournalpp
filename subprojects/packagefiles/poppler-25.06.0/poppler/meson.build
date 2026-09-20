configure_file(
  input: 'poppler-config.h.cmake',
  output: 'poppler-config.h',
  configuration: cdata,
  format: 'cmake',
)

poppler_sources = files(
  '../goo/GooString.cc',
  '../goo/GooTimer.cc',
  '../goo/ImgWriter.cc',
  '../goo/JpegWriter.cc',
  '../goo/NetPBMWriter.cc',
  '../goo/PNGWriter.cc',
  '../goo/TiffWriter.cc',
  '../goo/ft_utils.cc',
  '../goo/gbase64.cc',
  '../goo/gbasename.cc',
  '../goo/gfile.cc',
  '../goo/glibc.cc',
  '../goo/glibc_strtok_r.cc',
  '../goo/grandom.cc',
  '../goo/gstrtod.cc',
  '../fofi/FoFiBase.cc',
  '../fofi/FoFiEncodings.cc',
  '../fofi/FoFiTrueType.cc',
  '../fofi/FoFiType1.cc',
  '../fofi/FoFiType1C.cc',
  '../fofi/FoFiIdentifier.cc',
  '../poppler/Annot.cc',
  '../poppler/AnnotStampImageHelper.cc',
  '../poppler/Array.cc',
  '../poppler/CachedFile.cc',
  '../poppler/Catalog.cc',
  '../poppler/CharCodeToUnicode.cc',
  '../poppler/CMap.cc',
  '../poppler/CryptoSignBackend.cc',
  '../poppler/DateInfo.cc',
  '../poppler/Decrypt.cc',
  '../poppler/Dict.cc',
  '../poppler/Error.cc',
  '../poppler/FDPDFDocBuilder.cc',
  '../poppler/FILECacheLoader.cc',
  '../poppler/FileSpec.cc',
  '../poppler/FlateEncoder.cc',
  '../poppler/FontEncodingTables.cc',
  '../poppler/Form.cc',
  '../poppler/FontInfo.cc',
  '../poppler/Function.cc',
  '../poppler/Gfx.cc',
  '../poppler/GfxFont.cc',
  '../poppler/GfxState.cc',
  '../poppler/GlobalParams.cc',
  '../poppler/Hints.cc',
  '../poppler/ImageEmbeddingUtils.cc',
  '../poppler/JArithmeticDecoder.cc',
  '../poppler/JBIG2Stream.cc',
  '../poppler/JSInfo.cc',
  '../poppler/Lexer.cc',
  '../poppler/Link.cc',
  '../poppler/Linearization.cc',
  '../poppler/LocalPDFDocBuilder.cc',
  '../poppler/MarkedContentOutputDev.cc',
  '../poppler/NameToCharCode.cc',
  '../poppler/Object.cc',
  '../poppler/OptionalContent.cc',
  '../poppler/Outline.cc',
  '../poppler/OutputDev.cc',
  '../poppler/Page.cc',
  '../poppler/PageTransition.cc',
  '../poppler/Parser.cc',
  '../poppler/PDFDoc.cc',
  '../poppler/PDFDocBuilder.cc',
  '../poppler/PDFDocEncoding.cc',
  '../poppler/PDFDocFactory.cc',
  '../poppler/ProfileData.cc',
  '../poppler/PreScanOutputDev.cc',
  '../poppler/PSTokenizer.cc',
  '../poppler/SignatureInfo.cc',
  '../poppler/Stream.cc',
  '../poppler/StructTreeRoot.cc',
  '../poppler/StructElement.cc',
  '../poppler/UnicodeMap.cc',
  '../poppler/UnicodeMapFuncs.cc',
  '../poppler/UnicodeTypeTable.cc',
  '../poppler/UTF.cc',
  '../poppler/XRef.cc',
  '../poppler/PSOutputDev.cc',
  '../poppler/TextOutputDev.cc',
  '../poppler/PageLabelInfo.cc',
  '../poppler/SecurityHandler.cc',
  '../poppler/Sound.cc',
  '../poppler/ViewerPreferences.cc',
  '../poppler/Movie.cc',
  '../poppler/Rendition.cc',
  '../poppler/CertificateInfo.cc',
  '../poppler/BBoxOutputDev.cc',
  '../poppler/SplashOutputDev.cc',
  '../splash/Splash.cc',
  '../splash/SplashBitmap.cc',
  '../splash/SplashClip.cc',
  '../splash/SplashFTFont.cc',
  '../splash/SplashFTFontEngine.cc',
  '../splash/SplashFTFontFile.cc',
  '../splash/SplashFont.cc',
  '../splash/SplashFontEngine.cc',
  '../splash/SplashFontFile.cc',
  '../splash/SplashFontFileID.cc',
  '../splash/SplashPath.cc',
  '../splash/SplashPattern.cc',
  '../splash/SplashScreen.cc',
  '../splash/SplashState.cc',
  '../splash/SplashXPath.cc',
  '../splash/SplashXPathScanner.cc',
)

poppler_sources += files(
  'CourierWidths.pregenerated.c',
  'CourierBoldWidths.pregenerated.c',
  'CourierBoldObliqueWidths.pregenerated.c',
  'CourierObliqueWidths.pregenerated.c',
  'HelveticaWidths.pregenerated.c',
  'HelveticaBoldWidths.pregenerated.c',
  'HelveticaBoldObliqueWidths.pregenerated.c',
  'HelveticaObliqueWidths.pregenerated.c',
  'SymbolWidths.pregenerated.c',
  'TimesBoldWidths.pregenerated.c',
  'TimesBoldItalicWidths.pregenerated.c',
  'TimesItalicWidths.pregenerated.c',
  'TimesRomanWidths.pregenerated.c',
  'ZapfDingbatsWidths.pregenerated.c',
)

# if libcurl_opt.enabled()
#   poppler_sources += files([
#     'CurlCachedFile.cc',
#     'CurlPDFDocBuilder.cc',
#   ])
# endif

# if cairo_opt.enabled()
  poppler_sources += files([
    'CairoFontEngine.cc',
    'CairoOutputDev.cc',
    'CairoRescaleBox.cc',
  ])
# endif

# if jpeg_opt.enabled()
#   poppler_sources += files([
#     'DCTStream.cc',
#   ])
# endif

# if openjpeg_opt.enabled()
  poppler_sources += files([
    'JPEG2000Stream.cc',
  ])
# else
#   poppler_sources += files([
#     'JPXStream.cc',
#   ])
# endif

# if zlib_uncompress_opt.enabled()
#   poppler_sources += files([
#     'FlateStream.cc',
#   ])
# endif

poppler = build_target('poppler',
  poppler_sources,
  target_type : 'static_library',
  dependencies: all_poppler_deps,
  include_directories: include_directories('../xopp_customization', '.', '../goo', '..'),
  install: true,
)
poppler_dep = declare_dependency(
  link_with: poppler,
  include_directories : include_directories('.'),
)
meson.override_dependency('poppler', poppler_dep)
configure_file(
  input: '../poppler.pc.cmake',
  output: 'poppler.pc',
  configuration: pkgconfig_cdata,
  format: 'cmake@',
  install_dir: poppler_pkgconfigdir,
)
