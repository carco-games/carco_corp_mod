#pragma once

#include "../context.hpp"
#include "hooks.hpp"
#include "../config/config.hpp"

class AbilityHooksHandler {
public:
    AbilityHooksHandler(ConfigHandler* config_handler);
    ModResult initialize(ModError* error);
    ConfigHandler* getConfigHandler() { return configHandler; }
private:
    ConfigHandler* configHandler;
};