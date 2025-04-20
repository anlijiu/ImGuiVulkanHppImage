#include "App.h"
#include <print>

App::App() {

    std::print("App::App\n");
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
    // glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(params.windowSize.x, params.windowSize.y, params.windowTitle.c_str(), NULL, NULL);
      // 设置窗口位置
    glfwSetWindowPos(window, 100, 100);

    // make the context of the specified window current on the calling thread
    glfwMakeContextCurrent(window);
}

void App::run() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glfwSwapBuffers(window);
        // int res = draw_frame(init, render_data);
        // if (res != 0) {
        //     std::cout << "failed to draw frame \n";
        //     return -1;
        // }
    }
}

void App::cleanup() {

}

void App::loadDevSettings(const std::filesystem::path& configPath) {

}
