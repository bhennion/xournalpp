#include "stroke-demo.h"

#include <array>
#include <random>
#include <vector>

#include <cairo/cairo.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "model/MathVect.h"
#include "model/Point.h"
#include "model/StrokeContour.h"

#define PTS_PER_STROKE 50      ///< number of points in strokes - little effect on performances
#define STRETCH_FACTOR 2.      ///< affects strokes' lengths - can change performances
#define STROKE_FIRST_WIDTH 5.  ///< affects strokes' widths - can change performances
#define SPEED_FACTOR 1         ///< strokes' speed - little to no effect on performances

static std::vector<double> dashes = {3., 5., 0., 2.};  ///< dashes used in dashed mode...


/* No science here, just a hack from toying */
struct Color {
    double red, green, blue;
};

static const Color colors[] = {
        {0.71, 0.81, 0.83}, {1.00, 0.78, 0.57}, {0.64, 0.30, 0.35}, {0.73, 0.40, 0.39}, {0.91, 0.56, 0.64},
        {0.70, 0.47, 0.45}, {0.92, 0.75, 0.60}, {0.82, 0.86, 0.85}, {0.51, 0.56, 0.67}, {1.00, 0.79, 0.58},
};

struct Stroke {
    std::vector<Point> path;
    const Color* color;
    int x, y, v;
    double rot, rv;

    Stroke(int w, int h);
    Stroke() = default;
};


struct Strokes {
    std::vector<Stroke> strokes;
    size_t nbUsed;

    struct Iteratable {
        decltype(strokes)::iterator b, e;
        decltype(strokes)::iterator begin() { return b; }
        decltype(strokes)::iterator end() { return e; }
    };
    Iteratable getIteratable() { return {strokes.begin(), std::next(strokes.begin(), nbUsed)}; }
};

static std::vector<Point> getRandomPath(size_t nbPts) {
    std::random_device rd;
    std::default_random_engine e(rd());
    std::uniform_real_distribution<double> angle(-M_PI, M_PI);
    std::normal_distribution<double> normalD(0.0, 1.0);

    std::vector<Point> pts;
    pts.reserve(nbPts);
    pts.emplace_back(0.0, 0.0, STROKE_FIRST_WIDTH);
    double a = angle(e);
    pts.emplace_back(STRETCH_FACTOR * cos(a), STRETCH_FACTOR * sin(a), STROKE_FIRST_WIDTH * (1. + 0.1 * normalD(e)));

    auto beforeLast = pts.begin();
    auto last = std::prev(pts.end());
    while (pts.size() != nbPts) {
        MathVect2 v(*beforeLast, *last);
        double b = angle(e);
        double c = STRETCH_FACTOR * 0.3 * normalD(e);
        v = v + MathVect2(c * cos(b), c * sin(b));
        double dp = normalD(e);
        double m = std::min(0.5 * STROKE_FIRST_WIDTH, dp > 0 ? 2. * STROKE_FIRST_WIDTH - last->z : last->z);
        dp *= 0.2 * m;
        pts.emplace_back(last->x + v.dx, last->y + v.dy, std::abs(last->z + dp));
        beforeLast = last++;
        // printf("pt (%f ; %f ; %f)\n", pts.back().x, pts.back().y, pts.back().z);
    }
    return pts;
}

Stroke::Stroke(int w, int h):
        path(getRandomPath(PTS_PER_STROKE)),
        color(&colors[rand() % 10]),
        x(rand() % w),
        y(rand() % h),
        v(SPEED_FACTOR * (rand() % 10 + 2)),
        rot(fmod(rand(), 2 * M_PI)),
        rv(SPEED_FACTOR * (fmod(rand(), 5.) + 1) * M_PI / 360.) {}

static void xopp_original_stroke_draw(StrokesDemo* demo, cairo_t* cr) {
    for (auto& stroke: demo->strokes->getIteratable()) {
        cairo_save(cr);

        cairo_translate(cr, stroke.x, stroke.y);
        cairo_rotate(cr, stroke.rot);
        cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
        cairo_set_source_rgba(cr, stroke.color->red, stroke.color->green, stroke.color->blue, 1.);

        for (auto it1 = stroke.path.begin(), it2 = std::next(it1); it2 != stroke.path.end(); it1++, it2++) {
            cairo_set_line_width(cr, it1->z);
            cairo_move_to(cr, it1->x, it1->y);
            cairo_line_to(cr, it2->x, it2->y);
            cairo_stroke(cr);
        }
        cairo_restore(cr);
    }
}

static void xopp_original_dash_stroke_draw(StrokesDemo* demo, cairo_t* cr) {
    for (auto& stroke: demo->strokes->getIteratable()) {
        cairo_save(cr);

        cairo_translate(cr, stroke.x, stroke.y);
        cairo_rotate(cr, stroke.rot);
        cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
        cairo_set_source_rgba(cr, stroke.color->red, stroke.color->green, stroke.color->blue, 1.);

        double dashOffset = 0.;

        for (auto it1 = stroke.path.begin(), it2 = std::next(it1); it2 != stroke.path.end(); it1++, it2++) {
            cairo_set_line_width(cr, it1->z);
            cairo_set_dash(cr, dashes.data(), dashes.size(), dashOffset);
            dashOffset += std::hypot(it1->x - it2->x, it1->y - it2->y);
            cairo_move_to(cr, it1->x, it1->y);
            cairo_line_to(cr, it2->x, it2->y);
            cairo_stroke(cr);
        }
        cairo_restore(cr);
    }
}

static void xopp_contour_stroke_draw(StrokesDemo* demo, cairo_t* cr) {
    for (auto& stroke: demo->strokes->getIteratable()) {
        cairo_save(cr);

        cairo_translate(cr, stroke.x, stroke.y);
        cairo_rotate(cr, stroke.rot);
        cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
        cairo_set_source_rgba(cr, stroke.color->red, stroke.color->green, stroke.color->blue, 1.);

        xoj::view::StrokeContour(stroke.path).addToCairo(cr);

        cairo_fill(cr);

        cairo_restore(cr);
    }
}

static void xopp_contour_dash_stroke_draw(StrokesDemo* demo, cairo_t* cr) {
    for (auto& stroke: demo->strokes->getIteratable()) {
        cairo_save(cr);

        cairo_translate(cr, stroke.x, stroke.y);
        cairo_rotate(cr, stroke.rot);
        cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
        cairo_set_source_rgba(cr, stroke.color->red, stroke.color->green, stroke.color->blue, 1.);

        xoj::view::StrokeContourDashes(stroke.path, dashes).addToCairo(cr);

        cairo_fill(cr);

        cairo_restore(cr);
    }
}

struct {
    const char* name;
    void (*draw)(StrokesDemo* demo, cairo_t* cr);
} RENDERERS[StrokesDemo::N_RENDERERS] = {{"XOPP original", xopp_original_stroke_draw},
                                         {"XOPP contour", xopp_contour_stroke_draw},
                                         {"XOPP original dashed", xopp_original_dash_stroke_draw},
                                         {"XOPP contour dashed", xopp_contour_dash_stroke_draw}};

StrokesDemo::StrokesDemo(): strokes(std::make_unique<Strokes>()), draw(xopp_original_stroke_draw) {}
StrokesDemo::~StrokesDemo() = default;


void StrokesDemo::tick(int width, int height) {
    for (auto& stroke: this->strokes->getIteratable()) {
        stroke.y += stroke.v;
        if (stroke.y > height || stroke.y < 0) {
            stroke.v = -stroke.v;
        }
        stroke.rot += stroke.rv;
    }
}

void StrokesDemo::setParameters(size_t nStrokes, int width, int height) {
    auto& strokes = this->strokes->strokes;
    if (this->strokes->nbUsed == nStrokes) {
        return;
    }

    this->strokes->nbUsed = nStrokes;
    if (strokes.size() < nStrokes) {
        while (strokes.size() < nStrokes) {
            strokes.emplace_back(width, height);
        }
    }
}

void StrokesDemo::setActiveRenderer(int id) {
    id = id % N_RENDERERS;
    id = id < 0 ? id + N_RENDERERS : id;  // Some compilers may need that for negative id
    this->draw = RENDERERS[id].draw;
}

const char* StrokesDemo::getRendererName(int id) {
    id = id % N_RENDERERS;
    id = id < 0 ? id + N_RENDERERS : id;  // Some compilers may need that for negative id
    return RENDERERS[id].name;
}
