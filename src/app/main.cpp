#include "App.h"

#include <iostream>
#include <CL/cl.hpp>
#include <renderers/OpenCLRenderer.h>

using std::cout;

int main() {

    puts("I am alive !");

    try {
        App app("Paralight");
        app.Run();
    }
#ifdef __CL_ENABLE_EXCEPTIONS
    catch (const cl::Error& error) {
        std::cerr << "Exception catched in main: " << OpenCLRenderer::GetCLErrorString(error.err()) << ", at: " << error.what() << std::endl;
        return EXIT_FAILURE;
    }
#endif
    catch (const std::exception& e) {
        std::cerr << "Exception catched in main: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    return 0;
}


