#include "context.hpp"
#include "config/config.hpp"
#include "flow/flow.hpp"
#include "ui/ui.hpp"
#include "hooks/hooks.hpp"
#include "shop/shop.hpp"

DEFINE_MOD();

IMPORT_SERVICE(LogService, svc_log);

ConfigHandler g_configHandler;
UiHandler g_uiHandler;
FlowHandler g_flowHandler;
HooksHandler g_hooksHandler;
CarcoShop g_carcoShop;
int64_t g_applicationTimer;
int64_t g_prevApplicationTimer;
int64_t g_randomReturnTimer;
int64_t g_prevRandomReturnTimer;

void check_timer_change() {
    int64_t current_application_timer = g_configHandler.getApplicationTimer();
    if (g_prevApplicationTimer != current_application_timer) {
        g_applicationTimer = current_application_timer;
        g_prevApplicationTimer = g_applicationTimer;
    }

    int64_t current_random_return_timer = g_configHandler.getRndReturnTimer();
    if (g_prevRandomReturnTimer != current_random_return_timer) {
        g_randomReturnTimer = current_random_return_timer;
        g_prevRandomReturnTimer = g_randomReturnTimer;
    }
}

extern "C" {
MOD_EXPORT ModResult mod_initialize(ModError* error) {
    ModResult result;
    svc_log->info(mod_ctx, "CarcoCorp mod initialized");

    result = g_configHandler.initialize(error);
    if (result != MOD_OK) {
        mods::set_error(error, result, "Error in config.cpp");
    }
    svc_log->info(mod_ctx, "config initialized");

    g_carcoShop.initialize(&g_configHandler);
    svc_log->info(mod_ctx, "carcoShop initialized");

    result = g_uiHandler.initialize(&g_configHandler, &g_carcoShop);
    if (result != MOD_OK) {
        mods::set_error(error, result, "Error in ui.cpp");
    }
    svc_log->info(mod_ctx, "ui initialized");

    result = g_flowHandler.initialize(&g_configHandler, &g_uiHandler, error);
    if (result != MOD_OK) {
        mods::set_error(error, result, "Error in flow.cpp");
    }
    svc_log->info(mod_ctx, "flow initialized");

    result = g_hooksHandler.initialize(&g_configHandler, &g_flowHandler, error);
    if (result != MOD_OK) {
        mods::set_error(error, result, "Error in hooks.cpp");
    }
    svc_log->info(mod_ctx, "hooks initialized");
    
    // Set Timers
    g_prevApplicationTimer = g_configHandler.getApplicationTimer();
    g_applicationTimer = (g_prevApplicationTimer / 1000) * 30;
    g_randomReturnTimer = g_configHandler.getRndReturnTimer();
    g_prevRandomReturnTimer = g_randomReturnTimer;

    return result;
}

MOD_EXPORT ModResult mod_update(ModError* error) {
    if (g_configHandler.getProcessApplication()) {
        if (g_applicationTimer != 0) {
            g_applicationTimer--;
        } else if (g_applicationTimer == 0) {
            g_configHandler.setProcessApplication(false);
            g_configHandler.setUpdateFlowFlag(true);
            g_configHandler.setCvarApplicationStatus(STATUS_FINISHED);
            g_applicationTimer = (g_prevApplicationTimer / 1000) * 30;
        }
    }

    // Check Pay Timer
    if (g_hooksHandler.getPayTimer() != 0) {
        g_hooksHandler.setRunPayFunc(false);
        g_hooksHandler.decreasePayTimer();
    } else {
        g_hooksHandler.setRunPayFunc(true);
    }

    // Check Price Change Timer
    if (g_configHandler.getPriceChangeEnabled()) {
        g_carcoShop.changePrices();
    }

    // Check Random Return Timer
    if (g_configHandler.getRndReturnEnabled()) {
        // Check if Random Return can return more than one item
        if (!g_configHandler.getRndReturnMoreThanOne()) {
            // If no, check if there are any items currently returned
            // that have not been bought back
            if (!g_configHandler.getReturnedItemIds().empty()) {
                return MOD_OK;
            }
        }

        // If yes, continue normal return check logic
        if (g_randomReturnTimer != 0) {
            g_randomReturnTimer--;
        } else {
            g_carcoShop.randomReturn();
            g_randomReturnTimer = g_prevRandomReturnTimer;
        }
    }
    check_timer_change();

    return MOD_OK;
}

MOD_EXPORT ModResult mod_shutdown(ModError* error) {
    // g_flowHandler.shutdown();
    return MOD_OK;
}
}
