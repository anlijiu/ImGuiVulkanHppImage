#ifndef _APP_H_
#define _APP_H_

#include <string>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <filesystem>

#include "GfxDevice.h"
#include "Version.h"

class App
{
public:
    struct Params {
        // Either windowSize or renderSize should be set
        // You can also load them in loadAppSettings before they're used
        glm::ivec2 windowSize{}; // default window size
        glm::ivec2 renderSize{}; // size of presented draw image

        std::string appName{"test 4 Application"};
        std::string windowTitle; // if not set, set to appName
        Version version{};

        std::string sourceSubDirName;
        // needed for finding dev files in source directory
        // they should be stored in ${EDBR_SOURCE_ROOT}/games/<sourceSubDirName>/dev/
    };
    App ();
    virtual ~App ();

    void init(const Params& ps);
    void run();
    void cleanup();

    virtual void onWindowResize(){};

    virtual void customInit() = 0;                                                                                                                               
    virtual void customUpdate(float dt) = 0;
    virtual void customDraw() = 0;
    virtual void customCleanup() = 0;

    const Version& getVersion() const { return params.version; }

    virtual ImageId getMainDrawImageId() const { return NULL_IMAGE_ID; }
protected:
    virtual void loadAppSettings(){};
    virtual void loadDevSettings(const std::filesystem::path& configPath);

    GfxDevice gfxDevice;

    GLFWwindow* window{nullptr};

    Params params;


    bool vSync{true};

    bool isRunning{false};
    bool gamePaused{false};

    bool prodMode{false}; // if true - ignores dev env vars
    bool isDevEnvironment{false};
    std::string imguiIniPath;
    std::filesystem::path devDirPath;

    bool frameLimit{true};
    float frameTime{0.f};
    float avgFPS{0.f};

private:
    void handleBaseDevInput();

    // std::unique_ptr<IAudioManager> audioManager;
};

#endif // _APP_H_
