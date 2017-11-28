#ifndef TEST3D_WINDOW_H
#define TEST3D_WINDOW_H

#include <string>
// Forward declaration
typedef struct SDL_Window SDL_Window;

class Window {
    int width;
    int height;
    SDL_Window* sdl_window;
public:

    Window();
    Window(std::string title, int width, int height);

    SDL_Window *GetSDL_window() const {
        return sdl_window;
    }

};


#endif //TEST3D_WINDOW_H
