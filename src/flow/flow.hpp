#pragma once

#include "../context.hpp"
#include "../config/config.hpp"
#include "../ui/ui.hpp"
#include "mods/service.hpp"
#include "mods/svc/flow.hpp"

class FlowHandler {
public:
    ModResult initialize(ConfigHandler* config_handler, UiHandler* ui_handler, ModError* error);
    ModResult buildMidnaPromptFlow();
    void updateFlow();
    int64_t getUpdateStatus() { return updateStatus; }
    void setUpdatedStatus(int64_t status) { updateStatus = status; }
    ConfigHandler* getConfigHandler() { return configHandler; }
    UiHandler* getUiHandler() { return uiHandler; }
    void shutdown();
private:
    ConfigHandler* configHandler;
    UiHandler* uiHandler;
    int64_t updateStatus;
};