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

#ifndef _GLIBCXX_FILESYSTEM
#define _GLIBCXX_FILESYSTEM 1

#ifdef _GLIBCXX_SYSHDR
#pragma GCC system_header
#endif

#include <bits/requires_hosted.h>

#define __glibcxx_want_filesystem
#include <bits/version.h>

#ifdef __cpp_lib_filesystem // C++ >= 17 && HOSTED

/**
 * @defgroup filesystem File System
 *
 * Utilities for performing operations on file systems and their components,
 * such as paths, regular files, and directories.
 *
 * @since C++17
 */

#include <bits/fs_fwd.h>
#include "fs_path.h"
#include <bits/fs_dir.h>
#include <bits/fs_ops.h>

#endif // __cpp_lib_filesystem

#endif // _GLIBCXX_FILESYSTEM

// #include <filesystem>  // IWYU pragma: export

// enable fs
namespace fs = std::filesystem;  // NOLINT(misc-unused-alias-decls)
