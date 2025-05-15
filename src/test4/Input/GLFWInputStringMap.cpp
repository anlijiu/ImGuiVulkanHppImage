#include <Input/GLFWInputStringMap.h>

#include <GLFW/glfw3.h>

#include <unordered_map>

template<typename T>
class StringBimap {
public:
    void addMapping(const T& value, const std::string& str)
    {
        strValueMap.emplace(value, str);
        valueStrMap.emplace(str, value);
    }

    const std::string& at(const T& value) const { return strValueMap.at(value); }

    const T& at(const std::string& str) const { return valueStrMap.at(str); }

private:
    std::unordered_map<T, std::string> strValueMap;
    std::unordered_map<std::string, T> valueStrMap;
};

StringBimap<int> initKeyboardMapGLFW()
{
    StringBimap<int> bimap;
    bimap.addMapping(GLFW_KEY_0, "0");
    bimap.addMapping(GLFW_KEY_1, "1");
    bimap.addMapping(GLFW_KEY_2, "2");
    bimap.addMapping(GLFW_KEY_3, "3");
    bimap.addMapping(GLFW_KEY_4, "4");
    bimap.addMapping(GLFW_KEY_5, "5");
    bimap.addMapping(GLFW_KEY_6, "6");
    bimap.addMapping(GLFW_KEY_7, "7");
    bimap.addMapping(GLFW_KEY_8, "8");
    bimap.addMapping(GLFW_KEY_9, "9");

    bimap.addMapping(GLFW_KEY_A, "A");
    bimap.addMapping(GLFW_KEY_B, "B");
    bimap.addMapping(GLFW_KEY_C, "C");
    bimap.addMapping(GLFW_KEY_D, "D");
    bimap.addMapping(GLFW_KEY_E, "E");
    bimap.addMapping(GLFW_KEY_F, "F");
    bimap.addMapping(GLFW_KEY_G, "G");
    bimap.addMapping(GLFW_KEY_H, "H");
    bimap.addMapping(GLFW_KEY_I, "I");
    bimap.addMapping(GLFW_KEY_J, "J");
    bimap.addMapping(GLFW_KEY_K, "K");
    bimap.addMapping(GLFW_KEY_L, "L");
    bimap.addMapping(GLFW_KEY_M, "M");
    bimap.addMapping(GLFW_KEY_N, "N");
    bimap.addMapping(GLFW_KEY_O, "O");
    bimap.addMapping(GLFW_KEY_P, "P");
    bimap.addMapping(GLFW_KEY_Q, "Q");
    bimap.addMapping(GLFW_KEY_R, "R");
    bimap.addMapping(GLFW_KEY_S, "S");
    bimap.addMapping(GLFW_KEY_T, "T");
    bimap.addMapping(GLFW_KEY_U, "U");
    bimap.addMapping(GLFW_KEY_V, "V");
    bimap.addMapping(GLFW_KEY_W, "W");
    bimap.addMapping(GLFW_KEY_X, "X");
    bimap.addMapping(GLFW_KEY_Y, "Y");
    bimap.addMapping(GLFW_KEY_Z, "Z");

    bimap.addMapping(GLFW_KEY_F1, "F1");
    bimap.addMapping(GLFW_KEY_F2, "F2");
    bimap.addMapping(GLFW_KEY_F3, "F3");
    bimap.addMapping(GLFW_KEY_F4, "F4");
    bimap.addMapping(GLFW_KEY_F5, "F5");
    bimap.addMapping(GLFW_KEY_F6, "F6");
    bimap.addMapping(GLFW_KEY_F7, "F7");
    bimap.addMapping(GLFW_KEY_F8, "F8");
    bimap.addMapping(GLFW_KEY_F9, "F9");
    bimap.addMapping(GLFW_KEY_F10, "F10");
    bimap.addMapping(GLFW_KEY_F11, "F11");
    bimap.addMapping(GLFW_KEY_F12, "F12");

    bimap.addMapping(GLFW_KEY_MINUS,         "Minus");
    bimap.addMapping(GLFW_KEY_DELETE,        "Delete");
    bimap.addMapping(GLFW_KEY_SPACE,         "Space");
    bimap.addMapping(GLFW_KEY_LEFT,          "Left");
    bimap.addMapping(GLFW_KEY_RIGHT,         "Right");
    bimap.addMapping(GLFW_KEY_UP,            "Up");
    bimap.addMapping(GLFW_KEY_DOWN,          "Down");
    bimap.addMapping(GLFW_KEY_LEFT_SHIFT,    "LeftShift");
    bimap.addMapping(GLFW_KEY_RIGHT_SHIFT,   "RightShift");
    bimap.addMapping(GLFW_KEY_ESCAPE,        "Escape");
    bimap.addMapping(GLFW_KEY_KP_ADD,        "KPAdd");
    bimap.addMapping(GLFW_KEY_COMMA,         "Comma");
    bimap.addMapping(GLFW_KEY_BACKSPACE,     "Backspace");
    bimap.addMapping(GLFW_KEY_ENTER,         "Enter");
    bimap.addMapping(GLFW_KEY_LEFT_SUPER,    "LeftSuper");
    bimap.addMapping(GLFW_KEY_RIGHT_SUPER,   "RightSuper");
    bimap.addMapping(GLFW_KEY_LEFT_ALT,      "LeftAlt");
    bimap.addMapping(GLFW_KEY_RIGHT_ALT,     "RightAlt");
    bimap.addMapping(GLFW_KEY_LEFT_CONTROL,  "LeftControl");
    bimap.addMapping(GLFW_KEY_RIGHT_CONTROL, "RightControl");
    bimap.addMapping(GLFW_KEY_TAB,           "Tab");

    return bimap;
}

static StringBimap<int> keyboardMapGLFW = initKeyboardMapGLFW();

int toGLFWKey(const std::string& str)
{
    return keyboardMapGLFW.at(str);
}

const std::string& keyToString(int key)
{
    return keyboardMapGLFW.at(key);
}

StringBimap<int> initGamepadButtonMapGLFW()
{
    StringBimap<int> bimap;

    // cross = a,
    // circle = b,
    // square = x,
    // triangle = y,
    bimap.addMapping(GLFW_GAMEPAD_BUTTON_A,            "A");
    bimap.addMapping(GLFW_GAMEPAD_BUTTON_B,            "B");
    bimap.addMapping(GLFW_GAMEPAD_BUTTON_X,            "X");
    bimap.addMapping(GLFW_GAMEPAD_BUTTON_Y,            "Y");
    bimap.addMapping(GLFW_GAMEPAD_BUTTON_LEFT_BUMPER,  "LeftBumper");
    bimap.addMapping(GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, "RightBumper");
    bimap.addMapping(GLFW_GAMEPAD_BUTTON_BACK,         "Back");
    bimap.addMapping(GLFW_GAMEPAD_BUTTON_START,        "Start");
    bimap.addMapping(GLFW_GAMEPAD_BUTTON_GUIDE,        "Guide");
    bimap.addMapping(GLFW_GAMEPAD_BUTTON_LEFT_THUMB,   "LeftThumb");
    bimap.addMapping(GLFW_GAMEPAD_BUTTON_RIGHT_THUMB,  "RightThumb");
    bimap.addMapping(GLFW_GAMEPAD_BUTTON_DPAD_UP,      "DPadUp");
    bimap.addMapping(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT,   "DPadDown");
    bimap.addMapping(GLFW_GAMEPAD_BUTTON_DPAD_DOWN,    "DPadLeft");
    bimap.addMapping(GLFW_GAMEPAD_BUTTON_DPAD_LEFT,    "DPadRight");

    return bimap;
}

static StringBimap<int> gamepadButtonMapGLFW = initGamepadButtonMapGLFW();

int toGLFWGameControllerButton(const std::string& str)
{
    return gamepadButtonMapGLFW.at(str);
}


const std::string& gameControllerButtonKeyToString(int key)
{
    return gamepadButtonMapGLFW.at(key);
}

StringBimap<int> initGamepadAxisMapGLFW()
{
    StringBimap<int> bimap;

    bimap.addMapping( GLFW_GAMEPAD_AXIS_LEFT_X,        "LeftX");
    bimap.addMapping( GLFW_GAMEPAD_AXIS_LEFT_Y,        "LeftY");
    bimap.addMapping( GLFW_GAMEPAD_AXIS_RIGHT_X,       "RightX");
    bimap.addMapping( GLFW_GAMEPAD_AXIS_RIGHT_Y,       "RightY");
    bimap.addMapping( GLFW_GAMEPAD_AXIS_LEFT_TRIGGER,  "LeftTrigger");
    bimap.addMapping( GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER, "RightTrigger");

    return bimap;
}

static StringBimap<int> gamepadAxisMapGLFW = initGamepadAxisMapGLFW();

int toGLFWGameControllerAxis(const std::string& str)
{
    return gamepadAxisMapGLFW.at(str);
}

const std::string& gameControllerAxisToString(int axis)
{
    return gamepadAxisMapGLFW.at(axis);
}
