/*
 * Xournal++
 *
 * This file is part of the Xournal UnitTests
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#include <gtest/gtest.h>

#include "plugin/Plugin.h"
#include "util/StringUtils.h"

#include "config-features.h"
#include "config-test.h"

#ifdef ENABLE_PLUGINS
TEST(PluginTest, testLoadPluginUTF8) {
    Plugin p(nullptr, "Test UTF8 Plugin", fs::path(GET_TESTFILE(u8"plugins/µtf∞-插件-folder")).make_preferred());
    p.setEnabled(true);
    EXPECT_TRUE(p.loadScript(/* nogui = */ true));
}
#endif
