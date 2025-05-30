#include "Input/GamepadState.h"

#include "Input/ActionMapping.h"
#include "Input/GLFWInputStringMap.h"

#include "Core/JsonDataLoader.h"

#include <iostream>

const float GamepadState::DEAD_ZONE = 0.05f; // TODO: configurable dead zone
const float GamepadState::AXIS_PRESS_ZONE = 0.6f; // TODO: configurable

GamepadState::GamepadState()
{
    // for (auto& handle : handles) {
    //     handle = nullptr;
    // }
}

void GamepadState::init()
{
    for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; ++jid) {
        if (glfwJoystickIsGamepad(jid)) {
            std::cout << "Gamepad connected: id=" << jid << std::endl;

            if (connectedIds[jid] != 0) {
                continue;
            }

            connectedIds[jid] = jid;

            if (jid >= MAX_CONTROLLERS) {
                break;
            }
        }
    }

    const auto allAxes = {
        GLFW_GAMEPAD_AXIS_LEFT_X,
        GLFW_GAMEPAD_AXIS_LEFT_Y,
        GLFW_GAMEPAD_AXIS_RIGHT_X,
        GLFW_GAMEPAD_AXIS_RIGHT_Y,
        GLFW_GAMEPAD_AXIS_LEFT_TRIGGER,
        GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER,
    };

    for (auto axis : allAxes) {
        axes.emplace(axis, Axis{axis, DEAD_ZONE / 32767.0f, false});
    }
}

void GamepadState::handleJoystickCallback(int jid, bool connected) {
    if (connected) {
        connectedIds[jid] = jid;
    } else {
        connectedIds[jid] = 0;
    }
}

void GamepadState::cleanup()
{

}

void GamepadState::loadMapping(const JsonDataLoader& loader, ActionMapping& actionMapping)
{
    auto parseAxisGamepadActionMapping =
        [](const std::string& buttonStr) -> std::pair<ActionAxisMapping, bool> {
        std::string axisStr;
        bool positive{false};
        if (buttonStr.ends_with("+")) {
            axisStr = buttonStr.substr(0, buttonStr.size() - 1); // trim suffix
            positive = true;
        } else if (buttonStr.ends_with("-")) {
            axisStr = buttonStr.substr(0, buttonStr.size() - 1); // trim suffix
            positive = false;
        } else {
            return {{}, false};
        }
        const auto axis = toGLFWGameControllerAxis(axisStr);
        return {ActionAxisMapping{.axis = axis, .positive = positive}, true};
    };

    for (auto& [tagStr, buttonsLoader] : loader.getLoader("action").getKeyValueMap()) {
        const auto tag = actionMapping.getActionTagHash(tagStr);
        for (const auto& buttonStr : buttonsLoader.asVectorOf<std::string>()) {
            // try axis mapping
            const auto [aam, ok] = parseAxisGamepadActionMapping(buttonStr);
            if (ok) {
                addAxisActionMapping(aam, tag);
                continue;
            }
            const auto button = toGLFWGameControllerButton(buttonStr);
            addActionMapping(button, tag);
        }
    }

    for (auto& mappingLoader : loader.getLoader("axis").getVector()) {
        std::string axisStr;
        mappingLoader.get("axis", axisStr);

        std::string tagStr;
        mappingLoader.get("tag", tagStr);

        const auto axis = toGLFWGameControllerAxis(axisStr);
        const auto tag = actionMapping.getActionTagHash(tagStr);
        addAxisMapping(axis, tag);
    }

    for (auto& mappingLoader : loader.getLoader("axisButton").getVector()) {
        if (mappingLoader.hasKey("button")) {
            std::string tagStr;
            mappingLoader.get("tag", tagStr);

            std::string buttonStr;
            mappingLoader.get("button", buttonStr);

            float scale{};
            mappingLoader.get("scale", scale);

            const auto button = toGLFWGameControllerButton(buttonStr);
            const auto tag = actionMapping.getActionTagHash(tagStr);
            addButtonAxisMapping(button, tag, scale);
        }
    }
}

void GamepadState::onNewFrame()
{
    for (auto& buttonState : buttonStates) {
        buttonState.onNewFrame();
    }
}

// void GamepadState::handleEvent(const SDL_Event& event, ActionMapping& actionMapping)
// {
//     switch (event.type) {
//     case SDL_CONTROLLERBUTTONDOWN:
//     case SDL_CONTROLLERBUTTONUP:
//         handleButtonEvent(event, actionMapping);
//         break;
//         break;
//     case SDL_CONTROLLERDEVICEADDED: {
//         const auto gamepadId = event.cdevice.which;
//         if (!handles[gamepadId]) {
//             handles[gamepadId] = SDL_GameControllerOpen(gamepadId);
//             id = gamepadId;
//             std::cout << "Controller connected: " << gamepadId << std::endl;
//         }
//     } break;
//     case SDL_CONTROLLERDEVICEREMOVED: {
//         const auto gamepadId = event.cdevice.which;
//         if (id == gamepadId) {
//             assert(handles[gamepadId]);
//             SDL_GameControllerClose(handles[gamepadId]);
//             handles[gamepadId] = nullptr;
//         }
//         std::cout << "Controller removed: " << gamepadId << std::endl;
//         findConnectedGamepad();
//         std::cout << "New gamepad id: " << id << std::endl;
//     } break;
//     default:
//         break;
//     }
// }

void GamepadState::findConnectedGamepad()
{
    id = -1;
    for (int i = 0; i < MAX_CONTROLLERS; ++i) {
        // if (handles[i]) {
        //     id = i;
        //     break;
        // }
    }
}

void GamepadState::update(float /*dt*/, ActionMapping& actionMapping)
{
#if 0
    if (id == -1) { // gamepad not connected
        return;
    }

    auto handle = handles[id];
    assert(handle);

    for (const auto& [tag, buttons] : buttonActionBindings) {
        for (const auto& button : buttons) {
            if (buttonStates[button].pressed) {
                actionMapping.setActionPressed(tag);
            }
        }
    }

    for (auto& [axis, axisState] : axes) {
        auto value = SDL_GameControllerGetAxis(handle, axis);
        axisState.setValue(value);

        auto it = axisBindings.find(axis);
        if (it != axisBindings.end()) {
            for (auto& binding : it->second) {
                actionMapping.updateAxisState(binding, axisState.getValue());
            }
        }
    }

    for (auto& [tag, axesMappings] : axisActionBindings) {
        for (auto& axisMapping : axesMappings) {
            auto& as = axes.at(axisMapping.axis);
            if ((axisMapping.positive && as.getValue() > AXIS_PRESS_ZONE) ||
                (!axisMapping.positive && as.getValue() < -AXIS_PRESS_ZONE)) {
                actionMapping.setActionPressed(tag);
            }
        }
    }

    for (auto& [button, binding] : buttonAxisBindings) {
        if (buttonStates[button].pressed) {
            actionMapping.updateAxisState(binding.tag, binding.scale);
        }
    }
#endif
}

// void GamepadState::handleButtonEvent(const SDL_Event& event, ActionMapping& actionMapping)
// {
//     const bool isPressed = (event.type == SDL_CONTROLLERBUTTONDOWN);
//     buttonStates[event.cbutton.button].pressed = isPressed;
// }

void GamepadState::addActionMapping(GamepadButton button, ActionTagHash tag)
{
    buttonActionBindings[tag].push_back(button);
}

void GamepadState::addAxisMapping(GamepadAxis axis, ActionTagHash tag)
{
    axisBindings[axis].push_back(tag);
}

void GamepadState::addButtonAxisMapping(GamepadButton button, ActionTagHash tag, float scale)
{
    buttonAxisBindings.emplace(button, ButtonAxisBinding{tag, scale});
}

void GamepadState::addAxisActionMapping(ActionAxisMapping mapping, ActionTagHash tag)
{
    axisActionBindings[tag].push_back(mapping);
}

float GamepadState::getAxisValue(GamepadAxis axis) const
{
    // if (axis == SDL_CONTROLLER_AXIS_INVALID) {
        return 0.f;
    // }

    return axes.at(axis).getValue();
}

void GamepadState::setAxisInverted(GamepadAxis axis, bool b)
{
    // if (axis != SDL_CONTROLLER_AXIS_INVALID) {
        return axes.at(axis).setInverted(b);
    // }
}

void GamepadState::resetInput()
{
    for (auto& [axis, axisState] : axes) {
        axisState.reset();
    }
}

GamepadState::Axis::Axis(GamepadAxis axis, float deadZone, bool inverted) :
    axis(axis), deadZone(deadZone), inverted(inverted), value(0.f)
{}

void GamepadState::Axis::reset()
{
    value = 0.f;
}

float GamepadState::Axis::getValue() const
{
    return value;
}

void GamepadState::Axis::setValue(std::int16_t rawValue)
{
    static constexpr int range = 32767;
    value = rawValue / (float)range; // so it's scaled from -1 to 1 (or 0 to 1 for triggers)
    if (inverted) {
        value = -value;
    }
    if (std::abs(value) < deadZone) {
        value = 0.f;
    }
}

void GamepadState::Axis::setInverted(bool b)
{
    value = -value;
    inverted = b;
}
