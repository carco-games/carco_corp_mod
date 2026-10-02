#include "equipment_hooks.hpp"
#include "d/actor/d_a_alink.h"

EquipmentHooksHandler* g_self;

DEFINE_HOOK(&daAlink_c::swordSwingTrigger, CheckCutAction);
DEFINE_HOOK(&daAlink_c::itemEquip, ItemEquip);
DEFINE_HOOK(&daAlink_c::procBootsEquipInit, ProcBootsEquip);
DEFINE_HOOK(&daAlink_c::procSpinnerReadyInit, ProcSpinnerReadyInit);
DEFINE_HOOK(&daAlink_c::checkGuardAccept, ProcGuardSlipInit);
DEFINE_HOOK(&daAlink_c::procCutHeadInit, ProcCutHeadInit);
DEFINE_HOOK(&daAlink_c::procCutLargeJumpChargeInit, ProcCutLargeJumpChargeInit);
DEFINE_HOOK(&daAlink_c::procCutFinishInit, ProcCutFinishInit);

EquipmentHooksHandler::EquipmentHooksHandler(ConfigHandler* config_handler) {
    configHandler = config_handler;
    g_self = this;
}

HookAction checkIronBoots(ModContext*, void*, void*, void*) {
    return CHECK_LOCK(EQ_IRON_BOOTS);
}
HookAction checkSpinner(ModContext*, void*, void*, void*) {
    return CHECK_LOCK(EQ_SPINNER);
}
HookAction checkShield(ModContext*, void*, void*, void*) {
    return CHECK_LOCK(EQ_SHIELD);
}

HookAction checkItem(ModContext*, void* args, void*, void*) {
    daAlink_c* link = mods::arg<daAlink_c*>(args, 0);
    u16 itemID = mods::arg<u16>(args, 1);
    
    switch (itemID) {
        case dItemNo_IRONBALL_e:
            return CHECK_LOCK(EQ_BALL_AND_CHAIN);

        case dItemNo_NORMAL_BOMB_e:
        case dItemNo_WATER_BOMB_e:
        case dItemNo_POKE_BOMB_e:
            return CHECK_LOCK(EQ_BOMBS);

        case dItemNo_BOMB_ARROW_e:
            if (CHECK_LOCK(EQ_BOMBS) == HOOK_CONTINUE &&
                CHECK_LOCK(EQ_BOW) == HOOK_CONTINUE) {
                return HOOK_CONTINUE;
            }
            return HOOK_SKIP_ORIGINAL;

        case dItemNo_BOOMERANG_e:
            return CHECK_LOCK(EQ_BOOMERANG);

        case dItemNo_BOW_e:
            return CHECK_LOCK(EQ_BOW);

        case dItemNo_HOOKSHOT_e:
            return CHECK_LOCK(EQ_CLAWSHOT);

        case dItemNo_COPY_ROD_e:
            return CHECK_LOCK(EQ_DOMINION_ROD);

        case dItemNo_W_HOOKSHOT_e:
            return CHECK_LOCK(EQ_DOUBLE_CLAWSHOT);

        case dItemNo_FISHING_ROD_1_e:
        case dItemNo_JEWEL_ROD_e:
        case dItemNo_JEWEL_BEE_ROD_e:
        case dItemNo_JEWEL_WORM_ROD_e:
            return CHECK_LOCK(EQ_FISHING_ROD);

        case dItemNo_KANTERA_e:
            return CHECK_LOCK(EQ_LANTERN);

        case dItemNo_PACHINKO_e:
            return CHECK_LOCK(EQ_SLINGSHOT);
    }

    return HOOK_CONTINUE;
}

HookAction checkSword(ModContext*, void* args, void*, void*) {
    daAlink_c* link = mods::arg<daAlink_c*>(args, 0);
    if (dComIfGs_isCollectSword(COLLECT_LIGHT_SWORD) &&
        CHECK_LOCK(EQ_MASTER_SWORD) == HOOK_CONTINUE && CHECK_LOCK(EQ_ORDON_SWORD) == HOOK_CONTINUE &&
        CHECK_LOCK(EQ_WOODEN_SWORD) == HOOK_CONTINUE) {
        // If Light Sword has been collected and Master Sword,
        // Ordon Sword, and Wooden Sword have been bought
        return CHECK_LOCK(EQ_LIGHT_SWORD);
    } else if (dComIfGs_isCollectSword(COLLECT_MASTER_SWORD) &&
               CHECK_LOCK(EQ_ORDON_SWORD) == HOOK_CONTINUE && CHECK_LOCK(EQ_WOODEN_SWORD) == HOOK_CONTINUE) {
        return CHECK_LOCK(EQ_MASTER_SWORD);
    } else if (dComIfGs_isCollectSword(COLLECT_ORDON_SWORD) &&
               CHECK_LOCK(EQ_WOODEN_SWORD) == HOOK_CONTINUE) {
        return CHECK_LOCK(EQ_ORDON_SWORD);
    } else {
        return CHECK_LOCK(EQ_WOODEN_SWORD);
    }

    return HOOK_CONTINUE;
}

ModResult EquipmentHooksHandler::initialize(ModError* error) {
    ModResult result;

    PRE_HOOK(CheckCutAction, checkSword, "Failed to hook checkCutAction");
    PRE_HOOK(ItemEquip, checkItem, "Failed to hook itemEquip");
    PRE_HOOK(ProcBootsEquip, checkIronBoots, "Failed to hook procBootsEquip");
    PRE_HOOK(ProcSpinnerReadyInit, checkSpinner, "Failed to hook procSpinnerReadyInit");
    PRE_HOOK(ProcGuardSlipInit, checkShield, "Failed to hook procGuardSlipInit");
    PRE_HOOK(ProcCutHeadInit, checkSword, "Failed to hook procCutHeadInit");
    PRE_HOOK(ProcCutLargeJumpChargeInit, checkSword, "Failed to hook procCutLargeJumpChargeInit");
    PRE_HOOK(ProcCutFinishInit, checkSword, "Failed to hook procCutFinishInit");

    return result;
}