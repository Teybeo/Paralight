#ifndef TEST3D_APP_H
#define TEST3D_APP_H

#include "core/Scene.h"
#include "core/CameraControls.h"
#include "core/Options.h"
#include "Window.h"
#include "gui/Gui.h"

class App {

//    Window window;
    Window render_window;
    Window gui_window;
    Scene scene;
    BaseRenderer* renderer;
    CameraControls camera_controls;
    GUI* overlay;
    Options options;
    bool is_running;

public:
    App(std::string);

    void Draw();
    void Update();
    void Event();
    void Run();
};


#define STRINGIFY2( x) #x
#define STRINGIFY(x) STRINGIFY2(x)

#define DUMP_SIZE(type) std::cout << "sizeof " STRINGIFY(type) " " << sizeof(type) << "  alignof " STRINGIFY(type) " " << alignof(type) << std::endl;

#endif //TEST3D_APP_H
