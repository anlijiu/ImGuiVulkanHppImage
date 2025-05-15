#include <Input/MouseState.h>
#include <Input/ActionMapping.h>

#include "spdlog/spdlog.h"

// #include <SDL_events.h>

MouseState::MouseState() : prevPosition(getPosition())
{}

void MouseState::onNewFrame()
{
    prevPosition = position;
    // SDL_GetMouseState(&position.x, &position.y);
    // for (auto& mouseButtonState : mouseButtonStates) {
    //     mouseButtonState.onNewFrame();
    // }
}

void MouseState::handleCursorPosCallback(double xpos, double ypos) {
    spdlog::info("handleCursorPosCallback {:f}x{:f}", xpos, ypos);
}

// void MouseState::handleEvent(const SDL_Event& event, ActionMapping& actionMapping)
// {
//     switch (event.type) {
//     case SDL_MOUSEBUTTONDOWN:
//     case SDL_MOUSEBUTTONUP: {
//         const bool isPressed = (event.type == SDL_MOUSEBUTTONDOWN);
//         const auto button = (GLFWMouseButtonID)event.button.button;
//         auto& buttonState = mouseButtonStates[button];
// 
//         buttonState.pressed = isPressed;
// 
//         // handle button press
//         auto actionTag = getActionTag(button);
//         if (actionTag != ACTION_NONE_HASH && isPressed) {
//             actionMapping.setActionPressed(actionTag);
//         }
//         break;
//     }
//     default:
//         break;
//     }
// }

void MouseState::addActionMapping(GLFWMouseButtonID button, ActionTagHash tag)
{
    mouseActionBindings.emplace(button, tag);
}

bool MouseState::wasJustPressed(GLFWMouseButtonID button) const
{
    return mouseButtonStates[button].wasJustPressed();
}

bool MouseState::isHeld(GLFWMouseButtonID button) const
{
    return mouseButtonStates[button].isHeld();
}

bool MouseState::wasJustReleased(GLFWMouseButtonID button) const
{
    return mouseButtonStates[button].wasJustReleased();
}

void MouseState::resetInput()
{
    for (auto& mouseButtonState : mouseButtonStates) {
        mouseButtonState.reset();
    }
}

ActionTagHash MouseState::getActionTag(GLFWMouseButtonID button) const
{
    auto it = mouseActionBindings.find(button);
    return (it != mouseActionBindings.end()) ? it->second : ACTION_NONE_HASH;
}

const glm::ivec2& MouseState::getPosition() const
{
    return position;
}

glm::ivec2 MouseState::getDelta() const
{
    return position - prevPosition;
}
