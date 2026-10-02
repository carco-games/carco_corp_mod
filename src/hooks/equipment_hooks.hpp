#pragma once

#include "../context.hpp"
#include "hooks.hpp"
#include "../config/config.hpp"

class EquipmentHooksHandler {
public:
    EquipmentHooksHandler(ConfigHandler* config_handler);
    ModResult initialize(ModError* error);
    ConfigHandler* getConfigHandler() { return configHandler; }
private:
    ConfigHandler* configHandler;
};