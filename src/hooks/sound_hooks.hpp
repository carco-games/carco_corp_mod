#pragma once

#include "../context.hpp"
#include "hooks.hpp"
#include "../config/config.hpp"

class SoundHooksHandler {
public:
    SoundHooksHandler(ConfigHandler* config_handler);
    ModResult initialize(ModError* error);
    ConfigHandler* getConfigHandler() { return configHandler; }
    bool getSoundLockedToastStatus() { return soundLockedToastStatus; }
    void setSoundLockedToastStatus(bool value) { soundLockedToastStatus = value; }
private:
    ConfigHandler* configHandler;
    bool soundLockedToastStatus;
};