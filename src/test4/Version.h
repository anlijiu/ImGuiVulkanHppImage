#pragma once

#include <fmt/format.h>

struct Version {
    uint32_t major{0};
    uint32_t minor{0};
    uint32_t patch{0};

    std::string toString(bool addV = true) const
    {
        return fmt::format("{}{}.{}.{}", addV ? "v" : "", major, minor, patch);
    }
};
