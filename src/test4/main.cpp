#include "Game1.h"
#include <print>
#include "spdlog/spdlog.h"
#include "spdlog/version.h"
#include "format.h"

int main(int argc, char **argv) {
    std::print("main in\n");
    spdlog::set_level(spdlog::level::info);  // Set global log level to info
    spdlog::info("Welcome to spdlog version {}.{}.{}  !", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);
    Game1 app;
    app.init({
        .windowSize = glm::ivec2{1280, 720},
        .appName = "test4",
        .sourceSubDirName = "test4-4",
    });
    app.run();
    return 0;
} 
