#include "Builder.h"

#include "util/StringUtils.h"

#include "GladeSearchpath.h"  // for GladeSearchpath
#include "config.h"           // for XOURNALPP_GRESOURCE_NAMESPACE
#include "filesystem.h"       // for path

Builder::Builder(GladeSearchpath* gladeSearchPath, const std::string& uiFile):
        builder(gtk_builder_new_from_resource((std::string(XOURNALPP_GRESOURCE_NAMESPACE) + "/" + uiFile).c_str()),
                xoj::util::adopt) {}
