#pragma once

#include "../context.hpp"
#include "../config/config.hpp"
#include "../flow/flow.hpp"
#include "mods/hook.hpp"
#include <mods/svc/hook.hpp>

#define POST_HOOK(entry, function, error_msg)               \
    result = mods::hook::add_post<entry>(function);         \
    if (result != MOD_OK) {                                 \
        return mods::set_error(error, result, error_msg);   \
    }

#define PRE_HOOK(entry, function, error_msg)                \
    result = mods::hook::add_pre<entry>(function);          \
    if (result != MOD_OK) {                                 \
        return mods::set_error(error, result, error_msg);   \
    }

#define CHECK_LOCK(self, item)                                          \
    ((self->getConfigHandler()->*shopChecks[item].pywlFunc)() &&        \
     !(self->getConfigHandler()->*shopChecks[item].unlockedFunc)() &&   \
     self->getConfigHandler()->sendLockToast(item)                      \
        ? HOOK_SKIP_ORIGINAL                                            \
        : HOOK_CONTINUE)

class AbilityHooksHandler;
class EquipmentHooksHandler;
class SoundHooksHandler;

class HooksHandler {
public:
    ModResult initialize(ConfigHandler* config_handler, FlowHandler* flow_handler, ModError* error);
    ConfigHandler* getConfigHandler() { return configHandler; }
    FlowHandler* getFlowHandler() { return flowHandler; }
    int64_t getPayTimer() { return payTimer; }
    void setPayTimer(int64_t value) { payTimer = value; }
    void decreasePayTimer() { payTimer--; }
    bool getRunPayFunc() { return runPayFunc; }
    void setRunPayFunc(bool value) { runPayFunc = value; }
    bool getSoundLockedToastStatus();
    void setSoundLockedToastStatus(bool value);
private:
    AbilityHooksHandler* abilityHooksHandler;
    EquipmentHooksHandler* equipmentHooksHandler;
    SoundHooksHandler* soundHooksHandler;
    ConfigHandler* configHandler;
    FlowHandler* flowHandler;
    HookOptions options;
    int64_t payTimer;
    bool runPayFunc;
};