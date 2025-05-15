#pragma once

#include <filesystem>

#include "Input/ActionMapping.h"
#include "Input/GamepadState.h"
#include "Input/KeyboardState.h"
#include "Input/MouseState.h"

// union SDL_Event;

class InputManager {
public:
    InputManager();

    // Stuff for main loop
    void loadMapping(
        const std::filesystem::path& inputActionsPath,
        const std::filesystem::path& inputMappingPath);

    void onNewFrame();
    // void handleEvent(const SDL_Event& event);
    void handleKeyCallback(int key, int scancode, int action, int mods);
    void handleJoystickCallback(int pad_id, bool connected) { gamepad.handleJoystickCallback(pad_id, connected); };
    void handleCursorPosCallback(double xpos, double ypos) { mouse.handleCursorPosCallback(xpos, ypos); }
    void update(float dt);

    void cleanup();

    void resetInput();

    const ActionMapping& getActionMapping() const { return actionMapping; }
    ActionMapping& getActionMapping() { return actionMapping; }

    KeyboardState& getKeyboard() { return keyboard; }
    const KeyboardState& getKeyboard() const { return keyboard; }

    const MouseState& getMouse() const { return mouse; }
    const GamepadState& getGamepad() const { return gamepad; }

private:
    enum class InputEventCategory { Keyboard, Mouse, Gamepad, None };

    // using SDLEventType = std::uint32_t;
    // InputEventCategory getEventCategory(SDLEventType type) const;

    ActionMapping actionMapping;
    KeyboardState keyboard;
    MouseState mouse;
    GamepadState gamepad;

    bool usingGamepad = false;
};
