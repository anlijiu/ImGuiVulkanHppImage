#pragma once

#include <array>
#include <unordered_map>

#include "ActionTagHash.h"
#include "ButtonState.h"

// #include <SDL_mouse.h>

#include <glm/vec2.hpp>

class ActionMapping;

// union SDL_Event;
using GLFWMouseButtonID = std::uint8_t;

class MouseState {
public:
    MouseState();

    void onNewFrame();
    // void handleEvent(const SDL_Event& event, ActionMapping& actionMapping);
    void handleCursorPosCallback(double xpos, double ypos);

    void addActionMapping(GLFWMouseButtonID button, ActionTagHash tag);

    const glm::ivec2& getPosition() const;
    glm::ivec2 getDelta() const;

    bool wasJustPressed(GLFWMouseButtonID button) const;
    bool isHeld(GLFWMouseButtonID button) const;
    bool wasJustReleased(GLFWMouseButtonID button) const;

    void resetInput();

private:
    ActionTagHash getActionTag(GLFWMouseButtonID button) const;

    glm::ivec2 position; // current frame position
    glm::ivec2 prevPosition; // previous frame position

    std::unordered_map<GLFWMouseButtonID, ActionTagHash> mouseActionBindings;
    std::array<ButtonState, 16> mouseButtonStates;
};
