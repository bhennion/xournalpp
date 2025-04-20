#include "StrokeContour.h"

#include <algorithm>
#include <cmath>

#include "model/MathVect.h"
#include "model/Point.h"
#include "util/Assert.h"

static_assert(std::numeric_limits<double>::is_iec559);  // Ensures atan2(0., 0.) does not error

#include "StrokeContourInsides.h"
