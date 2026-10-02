#include "hooks.hpp"
#include "ability_hooks.hpp"
#include "equipment_hooks.hpp"
#include "sound_hooks.hpp"
#include "../ui/ui.hpp"
#include "d/d_com_inf_game.h"
#include "d/d_menu_save.h"
#include "d/actor/d_a_alink.h"

#include <string>

HooksHandler* g_hooksSelf;

IMPORT_SERVICE(HookService, svc_hook);

DEFINE_HOOK(&dMenu_save_c::restartInit, RestartInit);
DEFINE_HOOK(&daAlink_c::orderZTalk, OrderZTalk);

inline UiToastDesc* withdraw_toast(const char* title, uint32_t duration, u16 payment_amount) {
    static std::string withdraw_msg = std::string("Withdrawing ") + std::to_string(payment_amount) + std::string(" from your account...");
    static UiToastDesc desc = UI_TOAST_DESC_INIT;
    desc.title_rml = title;
    desc.body_rml = withdraw_msg.c_str();
    desc.duration_ms = duration;
    return &desc;
}

inline UiToastDesc* toast_msg(const char* title, uint32_t duration, const char* msg) {
    static UiToastDesc desc = UI_TOAST_DESC_INIT;
    desc.title_rml = title;
    desc.body_rml = msg;
    desc.duration_ms = duration;
    return &desc;
}

HookAction pay_func(bool enabled_flag, u16 payment_amount, const char* toast_title,
                    const char* insufficient_msg, const char* success_msg) {
    if (g_hooksSelf->getRunPayFunc()) {
        if (enabled_flag) {
            if (g_hooksSelf->getPayTimer() == 0) {
                g_hooksSelf->setPayTimer(60);
            }

            u16 rupees = dComIfGs_getRupee();
            UiToastDesc desc = UI_TOAST_DESC_INIT;
            desc.title_rml = toast_title;
            desc.duration_ms = 4000;

            if ((rupees - payment_amount) < 0) {
                desc.body_rml = insufficient_msg;
                svc_ui->push_toast(mod_ctx, &desc);
                return HOOK_SKIP_ORIGINAL;
            }

            std::string withdraw = "Withdrawing ";
            std::string pay_amount = std::to_string(payment_amount);
            std::string withdraw_end = " Rupees from your account...";
            std::string full_msg = withdraw + pay_amount + withdraw_end;
            desc.body_rml = full_msg.c_str();
            svc_ui->push_toast(mod_ctx, &desc);

            dComIfGs_setRupee(rupees - payment_amount);
            desc.body_rml = success_msg;
            svc_ui->push_toast(mod_ctx, &desc);
        }
    }

    return HOOK_CONTINUE;
}

static void onRestartInit(ModContext*, void*, void*, void*) {
    if (g_hooksSelf->getRunPayFunc()) {
        if (g_hooksSelf->getConfigHandler()->getGameoverPayEnabled()) {
            if (g_hooksSelf->getPayTimer() == 0) {
                g_hooksSelf->setPayTimer(60);
            }

            u16 rupees = dComIfGs_getRupee();
            u16 paymentAmount = dComIfGs_getDeathCount() * 5;
            svc_ui->push_toast(mod_ctx, withdraw_toast("CarcoCorp Medical", 4000, paymentAmount));

            if (rupees - paymentAmount < 0) {
                dComIfGs_setRupee(0);
                dComIfGs_setLife(1);
                svc_ui->push_toast(mod_ctx, toast_msg("CarcoCorp Medical", 4000, "Insufficient Funds! We were only able to offer minimal services."));
            } else {
                dComIfGs_setRupee(rupees - paymentAmount);
                svc_ui->push_toast(mod_ctx, toast_msg("CarcoCorp Medical", 4000, "Thank you for using CarcoCorp Medical!"));
            }
        }
    }
}

static HookAction onOrderZTalk(ModContext*, void* args, void* retval, void* user_data) {
    g_hooksSelf->getFlowHandler()->updateFlow();
    return HOOK_CONTINUE;
}

ModResult HooksHandler::initialize(ConfigHandler* config_handler, FlowHandler* flow_handler, ModError* error) {
    configHandler = config_handler;
    flowHandler = flow_handler;
    soundHooksHandler = new SoundHooksHandler(config_handler);
    if (soundHooksHandler) {
        soundHooksHandler->initialize(error);
    }
    svc_log->info(mod_ctx, "Sound hooks initialized");
    equipmentHooksHandler = new EquipmentHooksHandler(config_handler);
    if (equipmentHooksHandler) {
        equipmentHooksHandler->initialize(error);
    }
    abilityHooksHandler = new AbilityHooksHandler(config_handler);
    if (abilityHooksHandler) {
        abilityHooksHandler->initialize(error);
    }
    g_hooksSelf = this;
    payTimer = 0;
    runPayFunc = true;

    
    ModResult result;
    POST_HOOK(RestartInit, onRestartInit, "Failed to hook restartInit");
    PRE_HOOK(OrderZTalk, onOrderZTalk, "Failed to hook orderZTalk");

    return result;
}

bool HooksHandler::getSoundLockedToastStatus() { return soundHooksHandler->getSoundLockedToastStatus(); }

void HooksHandler::setSoundLockedToastStatus(bool value) { soundHooksHandler->setSoundLockedToastStatus(value); }