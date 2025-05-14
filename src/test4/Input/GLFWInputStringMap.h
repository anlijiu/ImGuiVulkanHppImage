#pragma once

#include <string>

// #include <SDL_gamecontroller.h>
// #include <SDL_scancode.h>

int toGLFWKey(const std::string& str);
const std::string& keyToString(int key);

int toGLFWGameControllerButton(const std::string& str);
const std::string& gameControllerButtonKeyToString(int key);

int toGLFWGameControllerAxis(const std::string& str);
const std::string& gameControllerAxisToString(int axis);
