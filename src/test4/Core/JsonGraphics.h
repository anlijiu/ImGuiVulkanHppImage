#pragma once

#include <nlohmann/json_fwd.hpp>

#include "Color.h"

void from_json(const nlohmann::json& j, LinearColor& c);
