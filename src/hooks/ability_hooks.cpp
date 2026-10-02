#include "ability_hooks.hpp"
#include "d/actor/d_a_alink.h"

AbilityHooksHandler* g_self;

DEFINE_HOOK(&daAlink_c::procHorseRideInit, ProcHorseRideInit);
DEFINE_HOOK(&daAlink_c::procFrontRollInit, ProcFrontRollInit);
DEFINE_HOOK(&daAlink_c::procCoGetItemInit, ProcCoOpenTreasureInit);
DEFINE_HOOK(&daAlink_c::procCoMetamorphoseInit, ProcCoMetamorphoseInit);
DEFINE_HOOK(&daAlink_c::checkAcceptWarp, CheckAcceptWarp);
DEFINE_HOOK(&daAlink_c::execute, Execute);
DEFINE_HOOK(&daAlink_c::procWolfDigInit, ProcWolfDigInit);
DEFINE_HOOK(&daAlink_c::onWolfEyeUp, OnWolfEyeUp);

bool g_overrideTransformLock = false;
bool g_startWarpTimer = false;
int64_t g_warpTimer = 0;

HookAction onProcHorseRideInit(ModContext*, void*, void*, void*) {
    return CHECK_LOCK(AB_HORSE_RIDING);
}

HookAction onProcFrontRollInit(ModContext*, void*, void*, void*) {
    return CHECK_LOCK(AB_ROLL);
}

HookAction onProcCoMetamorphoseInit(ModContext*, void* args, void*, void*) {
    daAlink_c* link = mods::arg<daAlink_c*>(args, 0);
    if (link->mDemo.getDemoMode() == daPy_demo_c::DEMO_METAMORPHOSE_UNK1_e ||
        link->mDemo.getDemoMode() == daPy_demo_c::DEMO_METAMORPHOSE_UNK2_e ||
        link->mDemo.getDemoMode() == daPy_demo_c::DEMO_METAMORPHOSE_ONLY_UNK1_e ||
        link->mDemo.getDemoMode() == daPy_demo_c::DEMO_METAMORPHOSE_ONLY_UNK2_e) {
        return HOOK_CONTINUE;
    }

    if (g_overrideTransformLock) {
        g_overrideTransformLock = false;
        return HOOK_CONTINUE;
    }
    return CHECK_LOCK(AB_TRANSFORM);
}

HookAction onCheckAcceptWarp(ModContext*, void*, void*, void*) {
    if (CHECK_LOCK(AB_WARP) == HOOK_CONTINUE) {
        g_overrideTransformLock = true;
        return HOOK_CONTINUE;
    }
    return HOOK_SKIP_ORIGINAL;
}

void onExecute(ModContext*, void* args, void*, void*) {
    daAlink_c* link = mods::arg<daAlink_c*>(args, 0);
    if (CHECK_LOCK(AB_TRANSFORM) != HOOK_CONTINUE) {
        if (link->mProcID == daAlink_c::PROC_WARP && g_warpTimer == 0) {
            mods::log::debug("setting warp timer");
            g_overrideTransformLock = true;
            g_startWarpTimer = true;
            g_warpTimer = 100;
            return;
        }

        if (g_startWarpTimer) {
            if (g_warpTimer != 0) {
                g_warpTimer--;
            } else {
                if (!g_overrideTransformLock) {
                    g_overrideTransformLock = true;
                }

                // Explode if you try to warp without unlocking Transforming
                mDoAud_seStartLevel(Z2SE_OBJ_BOMB_EXPLODE, &link->current.pos, 0, 0);
                g_env_light.settingTevStruct(0, &link->current.pos, &link->tevStr);
                static const u16 explosionEffects[] = {0x161, 0x162, 0x163, 0x164, 0x165,
                                                      0x166, 0x167, 0x168, 0x1EC};
                cXyz explosionScale(1.0f, 1.0f, 1.0f);
                for (int i = 0; i < 9; i++) {
                    dComIfGp_particle_setColor(explosionEffects[i], &link->current.pos, &link->tevStr, NULL, NULL, 0.0f, 0xFF,
                                               &link->current.angle, &explosionScale, NULL, -1, NULL);
                }
                link->setDamagePointNormal(99);
                g_warpTimer = 0;
                g_startWarpTimer = false;
            }
        }
    }
}

HookAction onProcWolfDigInit(ModContext*, void*, void*, void*) {
    return CHECK_LOCK(AB_DIG);
}

HookAction onOnWolfEyeUp(ModContext*, void*, void*, void*) {
    return CHECK_LOCK(AB_SENSE);
}

AbilityHooksHandler::AbilityHooksHandler(ConfigHandler* config_handler) {
    configHandler = config_handler;
    g_self = this;
}

ModResult AbilityHooksHandler::initialize(ModError* error) {
    ModResult result = MOD_OK;

    PRE_HOOK(ProcHorseRideInit, onProcHorseRideInit, "Failed to hook procHorseRideInit");
    PRE_HOOK(ProcFrontRollInit, onProcFrontRollInit, "Failed to hook procFrontRollInit");
    PRE_HOOK(ProcCoMetamorphoseInit, onProcCoMetamorphoseInit, "Failed to hook procCoMetamorphoseInit");
    PRE_HOOK(CheckAcceptWarp, onCheckAcceptWarp, "Failed to hook checkAcceptWarp");
    POST_HOOK(Execute, onExecute, "Failed to hook execute");
    PRE_HOOK(ProcWolfDigInit, onProcWolfDigInit, "Failed to hook procWolfDigInit");
    PRE_HOOK(OnWolfEyeUp, onOnWolfEyeUp, "Failed to hook onWolfEyeUp");

    return result;
}