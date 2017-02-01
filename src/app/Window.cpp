#include "Window.h"

#include <SDL_video.h>
#include <iostream>
#include <SDL_render.h>
#include <SDL_hints.h>

Window::Window(std::string title, int width, int height, int x) : width(width), height(height) {

    Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    sdl_window = SDL_CreateWindow(title.c_str(), x, SDL_WINDOWPOS_CENTERED, width, height, flags);

    if (sdl_window == nullptr) {
        std::cout << "Error creating window\n";
        throw std::bad_exception();
    }

    // Le plein écran utilisera désormais le meme mode que le bureau
    SDL_DisplayMode mode;
    SDL_GetDesktopDisplayMode(0, &mode);
    SDL_SetWindowDisplayMode(sdl_window, &mode);
    SDL_GL_CreateContext(sdl_window);
    SDL_GL_SetSwapInterval(0);
}


Window::Window() : width(0), height(0), sdl_window(nullptr) {

}
