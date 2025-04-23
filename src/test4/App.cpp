#include "App.h"
#include <print>
#include <chrono>
#include "spdlog/spdlog.h"

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

    auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
    // 处理鼠标移动
}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    spdlog::info("framebufferResizeCallback width: {}, height:{} ", width, height);
    auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
    // 设置标志： resize 了
    // app->framebufferResized = true;
}

App::App() {

    spdlog::info("App::App");
}

App::~App() {

}


void App::init(const Params& ps) {
    std::print("App::init\n");

    params = ps;

    std::print("111\n");
    std::print("windowSize x{} y{}\n", params.windowSize.x, params.windowSize.y);
    std::print("222\n");

    if (params.windowTitle.empty()) {
        params.windowTitle = params.appName;
    }
    std::print("windowTitle {}\n", params.windowTitle);

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(params.windowSize.x, params.windowSize.y, params.windowTitle.c_str(), NULL, NULL);
      // 设置窗口位置
    glfwSetWindowPos(window, 100, 100);

    // make the context of the specified window current on the calling thread
    glfwMakeContextCurrent(window);

    // 设置回调上下文为 this 指针 , 
    // 这样真实回调发生的时候，就可以拿到 App 指针
    glfwSetWindowUserPointer(window, this);

    // 设置键盘回调
    glfwSetKeyCallback(window, key_callback);

    // 设置鼠标回调
    glfwSetCursorPosCallback(window, mouse_callback);

    // 实际检测调整大小，我们可以使用glfwSetFramebufferSizeCallbackGLFW 框架中的函数来设置回调：
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

    gfxDevice.init(window, params.appName.c_str(), params.version, vSync);
}

void App::customDraw() {

}

void App::run() {
    // Fix your timestep! game loop
    const float FPS = 60.f;
    const float dt = 1.f / FPS;

    auto prevTime = std::chrono::high_resolution_clock::now();
    float accumulator = dt; // so that we get at least 1 update before render
    while (!glfwWindowShouldClose(window)) {
        const auto newTime = std::chrono::high_resolution_clock::now();
        frameTime = std::chrono::duration<float>(newTime - prevTime).count();

        if (frameTime > 0.07f && frameTime < 5.f) { // if >=5.f - debugging?
            printf("frame drop, time: %.4f\n", frameTime);
        }

        accumulator += frameTime;
        prevTime = newTime;

        // moving average
        float newFPS = 1.f / frameTime;
        if (newFPS == std::numeric_limits<float>::infinity()) {
            // can happen when frameTime == 0
            newFPS = 0;
        }
        avgFPS = std::lerp(avgFPS, newFPS, 0.1f);

        if (accumulator > 10 * dt) { // game stopped for debug
            accumulator = dt;
        }

        while (accumulator >= dt) {
            ZoneScopedN("Tick");

            glfwPollEvents();

            if (gfxDevice.needsSwapchainRecreate()) {
                gfxDevice.recreateSwapchain(params.windowSize.x, params.windowSize.y);
                onWindowResize();
            }

        }
        if (!gfxDevice.needsSwapchainRecreate()) {
            customDraw();
        }
        FrameMark;

        if (frameLimit) {
            // Delay to not overload the CPU
            const auto now = std::chrono::high_resolution_clock::now();
            const auto frameTime = std::chrono::duration<float>(now - prevTime).count();
            if (dt > frameTime) {
                std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<std::uint32_t>(dt - frameTime)));
            }
        }
 
        // glfwSwapBuffers(window);
        // int res = draw_frame(init, render_data);
        // if (res != 0) {
        //     std::cout << "failed to draw frame \n";
        //     return -1;
        // }
    }
}

void App::cleanup() {
    customCleanup();

    gfxDevice.cleanup();


    glfwDestroyWindow(window);

    glfwTerminate();
}

void App::loadDevSettings(const std::filesystem::path& configPath) {

}

void App::handleBaseDevInput()
{
    // if (inputManager.getKeyboard().wasJustPressed(SDL_SCANCODE_F11)) {
    //     const auto mainImageId = getMainDrawImageId();
    //     if (mainImageId != NULL_IMAGE_ID) {
    //         screenshotTaker.takeScreenshot(gfxDevice, mainImageId);
    //     } else {
    //         fmt::println("[dev] can't take screenshot: getMainDrawImageId returned NULL_IMAGE_ID");
    //     }
    // }
}
