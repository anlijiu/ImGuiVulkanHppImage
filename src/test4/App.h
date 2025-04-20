#ifndef _APP_H_
#define _APP_H_

#include <string>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <filesystem>

#include "Version.h"

class App
{
public:
    App ();
    virtual ~App ();

    void run();

private:
    GLFWwindow* window;

    bool vSync{true};
    glm::ivec2 windowSize{}; // default window size
    glm::ivec2 renderSize{}; // size of presented draw image
    std::string appName{"Test4 Application"};
    std::string windowTitle; // if not set, set to appName
    Version version{};
    std::string sourceSubDirName;

    bool isRunning{false};
    bool gamePaused{false};

    bool prodMode{false}; // if true - ignores dev env vars
    bool isDevEnvironment{false};
    std::string imguiIniPath;
    std::filesystem::path devDirPath;

    bool frameLimit{true};
    float frameTime{0.f};
    float avgFPS{0.f};

};

#endif // _APP_H_
