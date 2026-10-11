/*
 * Xournal++
 *
 * Paths to resources
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <filesystem>  // IWYU pragma: export

#include "util/raii/GObjectSPtr.h"

// enable fs
namespace fs = std::filesystem;  // NOLINT(misc-unused-alias-decls)

namespace xoj {
using FilePath = xoj::util::GFileSPtr;
}
