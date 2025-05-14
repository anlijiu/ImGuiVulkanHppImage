#pragma once

#include <string>

// #include <SDL_gamecontroller.h>
// #include <SDL_scancode.h>

int toGLFWKey(const std::string& str);
const std::string& toString(int key);

int toGLFWGameControllerButton(const std::string& str);

int toGLFWGameControllerAxis(const std::string& str);
// const std::string& toString(SDL_GameControllerAxis axis);
