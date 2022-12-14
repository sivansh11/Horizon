#include "app.h"

#include <exception>
#include <iostream>

int main() {
    App *application = App::init();
    try {
        application->run();
    } catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    
    App::shutdown(application);
    return EXIT_SUCCESS;
}