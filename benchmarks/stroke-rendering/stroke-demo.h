#pragma once

#include <memory>

#include <cairo/cairo.h>

struct Strokes;

struct StrokesDemo {
    StrokesDemo();
    ~StrokesDemo();

    void setParameters(size_t nStrokes, int width, int height);
    void tick(int width, int height);
    void setActiveRenderer(int id);

    static const char* getRendererName(int id);
    static constexpr int N_RENDERERS = 4;


    std::unique_ptr<Strokes> strokes;
    void (*draw)(struct StrokesDemo*, cairo_t*);
};
