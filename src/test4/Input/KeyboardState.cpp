#include <Input/KeyboardState.h>

#include <Input/ActionMapping.h>
#include <Input/GLFWInputStringMap.h>

#include <Core/JsonDataLoader.h>

#include <GLFW/glfw3.h>
// #include <SDL_events.h>

void KeyboardState::loadMapping(const JsonDataLoader& loader, ActionMapping& actionMapping)
{
    for (auto& [tagStr, keysLoader] : loader.getLoader("action").getKeyValueMap()) {
        const auto tag = actionMapping.getActionTagHash(tagStr);
        for (const auto& keyStr : keysLoader.asVectorOf<std::string>()) {
            const auto scancode = toGLFWKey(keyStr);
            addActionMapping(scancode, tag);
        }
    }

    for (auto& mappingLoader : loader.getLoader("axisButton").getVector()) {
        std::string keyStr;
        mappingLoader.get("key", keyStr);

        std::string tagStr;
        mappingLoader.get("tag", tagStr);

        float scale{};
        mappingLoader.get("scale", scale);

        const auto scancode = toGLFWKey(keyStr);
        const auto tag = actionMapping.getActionTagHash(tagStr);

        addAxisMapping(scancode, tag, scale);
    }
}

void KeyboardState::onNewFrame()
{
    for (auto& keyState : keyStates) {
        keyState.onNewFrame();
    }
}


void KeyboardState::handleEvent(int key, int scancode, int action, int mods, ActionMapping& actionMapping) {
     const bool isPressed = (action == GLFW_PRESS);
     keyStates[key].pressed = isPressed;
}
// void KeyboardState::handleEvent(const SDL_Event& event, ActionMapping& actionMapping)
// {
//     const auto key = event.key.keysym.scancode;
//     if (key == SDL_SCANCODE_UNKNOWN) {
//         return;
//     }
// 
//     const bool isPressed = (event.type == SDL_KEYDOWN);
//     keyStates[key].pressed = isPressed;
// }

void KeyboardState::update(float /*dt*/, ActionMapping& actionMapping)
{
    for (auto& [tag, keys] : keyActionBindings) {
        for (const auto& key : keys) {
            if (keyStates[key].pressed) {
                actionMapping.setActionPressed(tag);
            }
        }
    }

    for (auto& [key, bindings] : axisButtonBindings) {
        if (keyStates[key].pressed) {
            for (const auto& binding : bindings) {
                actionMapping.updateAxisState(binding.tag, binding.scale);
            }
        }
    }
}

void KeyboardState::addActionMapping(int key, ActionTagHash tag)
{
    keyActionBindings[tag].push_back(key);
}

void KeyboardState::addAxisMapping(int key, ActionTagHash tag, float scale)
{
    axisButtonBindings[key].push_back(ButtonAxisBinding{tag, scale});
}

bool KeyboardState::wasJustPressed(int key) const
{
    return keyStates[key].wasJustPressed();
}

bool KeyboardState::wasJustReleased(int key) const
{
    return keyStates[key].wasJustReleased();
}

bool KeyboardState::isPressed(int key) const
{
    return keyStates[key].isPressed();
}

bool KeyboardState::isHeld(int key) const
{
    return keyStates[key].isHeld();
}

void KeyboardState::resetInput()
{
    for (auto& keyState : keyStates) {
        keyState.reset();
    }
}
