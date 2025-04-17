#include "app.hpp"

#include <iostream>
#include <cstdlib>
#include <iostream>

int main() {
    lve::App app{};

    try{
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
