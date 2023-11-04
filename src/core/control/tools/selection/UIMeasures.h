/*
 * Xournal++
 *
 * Measures for EditSelection's interface (padding, handle size and so on)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "CursorSelectionType.h"  // for CursorSelectionType, CURS...

namespace EditSelectionMeasures {
/// Padding between the content and the frame, in pixels
constexpr int CONTENT_PADDING = 28;

/// Halfwidth of the stretch handles, in pixels
constexpr int STRETCH_HANDLE_SIZE = 10;

/// Distance between the center of the rotation handle and the frame, in pixels
constexpr int ROTATION_HANDLE_DISTANCE = 38;
/// Halfwidth of the rotation handle, in pixels
constexpr int ROTATION_HANDLE_SIZE = 10;

/// Distance between the center of the deletion handle and the frame, in pixels
constexpr int DELETION_HANDLE_DISTANCE = 42;
/// Halfwidth of the rotation handle, in pixels
constexpr int DELETION_HANDLE_SIZE = 10;

// Make sure the handles don't overlap
static_assert(ROTATION_HANDLE_DISTANCE > STRETCH_HANDLE_SIZE + ROTATION_HANDLE_SIZE);
static_assert(DELETION_HANDLE_DISTANCE > STRETCH_HANDLE_SIZE + DELETION_HANDLE_SIZE);
// Make sure the handles don't overlap with the selected elements
static_assert(CONTENT_PADDING > STRETCH_HANDLE_SIZE);
};  // namespace EditSelectionMeasures
