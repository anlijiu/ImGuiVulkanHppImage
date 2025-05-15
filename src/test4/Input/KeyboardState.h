#pragma once

#include <array>
#include <unordered_map>
#include <vector>


#include <GLFW/glfw3.h>
// #include <SDL_scancode.h>

#include "ActionTagHash.h"
#include "ButtonState.h"

// union SDL_Event;

class ActionMapping;
class JsonDataLoader;

class KeyboardState {
public:
    void loadMapping(const JsonDataLoader& loader, ActionMapping& actionMapping);

    void onNewFrame();
    // void handleEvent(const SDL_Event& event, ActionMapping& actionMapping);
    void handleEvent(int key, int scancode, int action, int mods, ActionMapping& actionMapping);

    void update(float dt, ActionMapping& actionMapping);

    void addActionMapping(int key, ActionTagHash tag);
    void addAxisMapping(int key, ActionTagHash tag, float scale);

    bool wasJustPressed(int key) const;
    bool wasJustReleased(int key) const;
    bool isPressed(int key) const;
    bool isHeld(int key) const;

    void resetInput();

private:
    struct ButtonAxisBinding {
        ActionTagHash tag;
        float scale;
    };

    // state
    std::array<ButtonState, GLFW_KEY_LAST> keyStates;

    // bindings
    std::unordered_map<ActionTagHash, std::vector<int>> keyActionBindings;
    std::unordered_map<int, std::vector<ButtonAxisBinding>> axisButtonBindings;
};
