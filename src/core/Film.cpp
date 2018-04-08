#include "Film.h"

#include <iostream>

using std::cout;
using std::endl;

void Film::Update() {

    has_changed = false;

    if (is_dirty) {
        width = int(roundf(base_width * render_scale));
        height = int(roundf(base_height * render_scale));
        pixels.resize(width * height);
        is_dirty = false;
        has_changed = true;
    }
}

Film::Film(int width, int height) {
    SetBaseFilmSize(width, height);
    pixels.resize(width * height);
}
