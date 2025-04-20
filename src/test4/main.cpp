#include "App.h"
#include <print>

int main(int argc, char **argv) {
    std::print("main in\n");

    App app;
    app.init({
        .windowSize = glm::ivec2{1280, 720},
        .appName = "test4",
        .sourceSubDirName = "test4-4",
    });
    app.run();
    return 0;
} 
