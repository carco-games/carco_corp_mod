#include "ui.hpp"

IMPORT_SERVICE(UiService, svc_ui);

UiHandler* g_self;
UiWindowHandle g_optionsWindow;
UiWindowHandle g_carcoShopWindow;

ModResult add_control(UiElementHandle pane, const UiControlDesc& desc) {
    return svc_ui->pane_add_control(mod_ctx, pane, &desc, nullptr);
}

void add_toggle(UiElementHandle pane, const char* label, ConfigVarHandle cvar, const char* help) {
    UiControlDesc control = UI_CONTROL_DESC_INIT;
    control.kind = UI_CONTROL_TOGGLE;
    control.label = label;
    control.help_rml = help;
    control.binding = UI_BINDING_CONFIG_VAR;
    control.config_var = cvar;
    add_control(pane, control);
}

#define GET_ENUM                                                    \
    static_cast<ShopItems_e>(reinterpret_cast<uintptr_t>(user_data))

static void buy(ModContext*, void* user_data) {
    ConfigVarHandle cvar = (g_self->getConfigHandler()->*unlockedCvarFuncs[GET_ENUM])();
    std::array<uint16_t, ITEM_COUNT> priceTable = g_self->getConfigHandler()->getPrices();
    uint16_t price = priceTable[GET_ENUM];
    if (g_self->getCarcoShop()->checkFunds(price)) {
        g_self->getCarcoShop()->unlockItem(cvar, price);
        g_self->getCarcoShop()->checkReturnItem(GET_ENUM);
    }
}

static bool isItemBought(ModContext*, void* user_data) {
    ShopItems_e itemID = GET_ENUM;
    return (g_self->getConfigHandler()->*shopChecks[itemID].unlockedFunc)();
}

static ModResult buildOptGeneralTab(ModContext*, UiWindowHandle, UiElementHandle left, UiElementHandle, void*, ModError*) {
    svc_ui->pane_add_section(mod_ctx, left, "Application Status");
    static const char* kAppStatusOpts[] = {"Not Started", "Ongoing", "Complete"};
    UiControlDesc control = UI_CONTROL_DESC_INIT;
    control.kind = UI_CONTROL_SELECT;
    control.label = "Application Status";
    control.help_rml = "You can apply for a CarcoCorp membership by talking to Midna!\nOr you can manually choose your CarcoCorp application status here";
    control.binding = UI_BINDING_CONFIG_VAR;
    control.config_var = g_self->getConfigHandler()->getCvarApplicationStatus();
    control.options = kAppStatusOpts;
    control.option_count = 3;
    add_control(left, control);

    svc_ui->pane_add_section(mod_ctx, left, "Application Timer");
    control = UI_CONTROL_DESC_INIT;
    control.kind = UI_CONTROL_NUMBER;
    control.label = "Application Timer";
    control.help_rml = "Amount of time (in milliseconds) a CarcoCorp application takes to process";
    control.binding = UI_BINDING_CONFIG_VAR;
    control.config_var = g_self->getConfigHandler()->getCvarApplicationTimer();
    control.min = 1;
    control.max = 180000;
    add_control(left, control);

    add_toggle(left, "Gameover Fee", g_self->getConfigHandler()->getCvarGameoverPayEnabled(),
               "Pay for CarcoCare any time you get a game over. (Cost scales with # of game overs)");

    add_toggle(left, "Price Changing", g_self->getConfigHandler()->getCvarPriceChangeEnabled(),
               "Shop prices change every 5 minutes (by default; timer can be changed)!");

    control = UI_CONTROL_DESC_INIT;
    control.kind = UI_CONTROL_NUMBER;
    control.label = "Price Change Timer";
    control.help_rml = "Amount of time (in seconds) that it takes prices to change";
    control.binding = UI_BINDING_CONFIG_VAR;
    control.config_var = g_self->getConfigHandler()->getCvarPriceChangeTimer();
    control.min = 1;
    control.max = 999999;
    add_control(left, control);

    add_toggle(left, "Random Returns", g_self->getConfigHandler()->getCvarRndReturnEnabled(),
               "Every 10 minutes (by default), there is a 10% chance (by default) for a \nrandom purchase to be returned to CarcoCorp!");

    control = UI_CONTROL_DESC_INIT;
    control.kind = UI_CONTROL_NUMBER;
    control.label = "Random Return Chance";
    control.help_rml = "Percentage chance for item to be returned upon check";
    control.binding = UI_BINDING_CONFIG_VAR;
    control.config_var = g_self->getConfigHandler()->getCvarRndReturnChance();
    control.min = 1;
    control.max = 100;
    add_control(left, control);

    control = UI_CONTROL_DESC_INIT;
    control.kind = UI_CONTROL_NUMBER;
    control.label = "Random Return Timer";
    control.help_rml = "How long (in seconds) between random return checks";
    control.binding = UI_BINDING_CONFIG_VAR;
    control.config_var = g_self->getConfigHandler()->getCvarRndReturnTimer();
    control.min = 1;
    control.max = 999999;
    add_control(left, control);

    return MOD_OK;
}

#define PYWL_TOGGLE(item, lbl_str, help)                                                        \
    add_toggle(left, lbl_str, (g_self->getConfigHandler()->*paywallCvarFuncs[item])(), help)


#define AB_OPT_TAB(name, item, help)                    \
    if(std::string_view(#item).starts_with("AB_")) {    \
        PYWL_TOGGLE(item, #name " Paywall", help);      \
    }

static ModResult buildOptAbilitiesTab(ModContext*, UiWindowHandle, UiElementHandle left, UiElementHandle, void*, ModError*) {
    SHOP_ITEMS_AB_UI(AB_OPT_TAB)
    return MOD_OK;
}

#define EQ_OPT_TAB(name, item, help)                    \
    if(std::string_view(#item).starts_with("EQ_")) {    \
        PYWL_TOGGLE(item, #name " Paywall", help);      \
    }

static ModResult buildOptEquipmentTab(ModContext*, UiWindowHandle, UiElementHandle left, UiElementHandle, void*, ModError*) {
    SHOP_ITEMS_EQ_UI(EQ_OPT_TAB)
    return MOD_OK;
}

#define SND_OPT_TAB(name, item, help)           \
    PYWL_TOGGLE(item, #name " Paywall", help);  \

static ModResult buildOptSoundTab(ModContext*, UiWindowHandle, UiElementHandle left, UiElementHandle, void*, ModError*) {
    SHOP_ITEMS_SND_UI(SND_OPT_TAB)
    return MOD_OK;
}

#define SEND_ENUM(enum)                                     \
    reinterpret_cast<void*>(static_cast<uintptr_t>(enum))

std::string item_lbls[] = {
    "Digging", "Horse Riding", "Rolling", "Wolf Senses", "Transforming", "Warping",
    "Ball And Chain", "Bombs", "Boomerang", "Bow", "Clawshot", "Dominion Rod", "Double Clawshot",
    "Fishing Rod", "Iron Boots", "Lantern", "Light Sword", "Master Sword", "Ordon Sword", "Shield",
    "Slingshot", "Spinner", "Wooden Sword", "Link SFX", "Enemy SFX", "Environment SFX", "NPC SFX",
    "Background Music", "Audio Streams"
};

#define ADD_UNLOCK_CONTROL(enum, label_str)                         \
    price_str = item_lbls[enum] + " Price: " +                      \
        std::to_string(                                             \
            g_self->getConfigHandler()->getPriceChangeEnabled()     \
                ? g_self->getConfigHandler()->getPrices()[enum]     \
                : g_self->getConfigHandler()->getBasePrices()[enum] \
        ) + " Rupees";                                              \
    svc_ui->pane_add_section(mod_ctx, right, price_str.c_str());    \
    control = UI_CONTROL_DESC_INIT;                                 \
    control.kind = UI_CONTROL_BUTTON;                               \
    control.label = label_str;                                      \
    control.user_data = SEND_ENUM(enum);                            \
    control.on_pressed = &buy;                                      \
    control.is_disabled = &isItemBought;                            \
    add_control(left, control);

#define AB_EQ_TAB(name, item, help)                 \
    ADD_UNLOCK_CONTROL(item, "Buy " #name " Usage");

static ModResult buildAbilitiesTab(ModContext*, UiWindowHandle, UiElementHandle left, UiElementHandle right, void*, ModError*) {
    UiControlDesc control;
    std::string price_str;
    SHOP_ITEMS_AB_UI(AB_EQ_TAB)
    return MOD_OK;
}

static ModResult buildEquipmentTab(ModContext*, UiWindowHandle, UiElementHandle left, UiElementHandle right, void*, ModError*) {
    UiControlDesc control;
    std::string price_str;
    SHOP_ITEMS_EQ_UI(AB_EQ_TAB)
    return MOD_OK;
}

#define SND_TAB(name, item, help)                                           \
    if (item == SND_AST) {                                                  \
        ADD_UNLOCK_CONTROL(item, "Buy " #name " (Mostly Cutscene Music)");  \
    } else {                                                                \
        ADD_UNLOCK_CONTROL(item, "Buy " #name " Usage");                    \
    }

static ModResult buildSoundTab(ModContext*, UiWindowHandle, UiElementHandle left, UiElementHandle right, void*, ModError*) {
    UiControlDesc control;
    std::string price_str;
    SHOP_ITEMS_SND_UI(SND_TAB)
    return MOD_OK;
}

void on_options_menu_close(ModContext*, UiWindowHandle, void*) {
    g_optionsWindow = 0;
}

static void onOpenOptionsMenu(ModContext*, void*) {
    if (g_optionsWindow != 0) {
        return;
    }

    UiTabDesc tabs[4] = {UI_TAB_DESC_INIT, UI_TAB_DESC_INIT, UI_TAB_DESC_INIT, UI_TAB_DESC_INIT};
    tabs[0].title = "General";
    tabs[0].build = &buildOptGeneralTab;

    tabs[1].title = "Abilities";
    tabs[1].build = &buildOptAbilitiesTab;

    tabs[2].title = "Equipment";
    tabs[2].build = &buildOptEquipmentTab;

    tabs[3].title = "Sound";
    tabs[3].build = &buildOptSoundTab;

    UiWindowDesc desc = UI_WINDOW_DESC_INIT;
    desc.tabs = tabs;
    desc.tab_count = 4;
    desc.on_closed = on_options_menu_close;

    if (svc_ui->window_push(mod_ctx, &desc, &g_optionsWindow) != MOD_OK) {
        mods::log::debug("failed to open options menu");
    }
}

static void onApply(ModContext*, void*) {
    UiToastDesc desc = UI_TOAST_DESC_INIT;
    switch (g_self->getConfigHandler()->getApplicationStatus()) {
        case STATUS_NOT_STARTED:
            g_self->getConfigHandler()->setProcessApplication(true);
            g_self->getConfigHandler()->setCvarApplicationStatus(STATUS_ONGOING);
            g_self->getConfigHandler()->setUpdateFlowFlag(true);
            desc.title_rml = "CarcoCorp";
            desc.body_rml = "We've started your membership application!";
            desc.duration_ms = 2500;
            svc_ui->push_toast(mod_ctx, &desc);
            break;

        case STATUS_ONGOING:
            desc.title_rml = "CarcoCorp";
            desc.body_rml = "We're still working on your application!\nCheck back later!";
            desc.duration_ms = 2500;
            svc_ui->push_toast(mod_ctx, &desc);
            break;

        case STATUS_FINISHED:
            desc.title_rml = "CarcoCorp";
            desc.body_rml = "Your application has been processed!\nYou can now access the CarcoCorp Shop!";
            desc.duration_ms = 2500;
            svc_ui->push_toast(mod_ctx, &desc);
            break;
    }
}

void on_carco_shop_close(ModContext*, UiWindowHandle, void*) {
    g_carcoShopWindow = 0;
}

static void onOpenCarcoShop(ModContext*, void*) {
    if (g_carcoShopWindow != 0) {
        return;
    }

    if (g_self->getConfigHandler()->getApplicationStatus() != STATUS_FINISHED) {
        UiToastDesc desc = UI_TOAST_DESC_INIT;
        desc.title_rml = "CarcoCorp";
        desc.body_rml = "You haven't applied to be a CarcoCorp member!";
        desc.duration_ms = 2500;
        svc_ui->push_toast(mod_ctx, &desc);
        return;
    }

    UiTabDesc tabs[3] = {UI_TAB_DESC_INIT, UI_TAB_DESC_INIT, UI_TAB_DESC_INIT};
    tabs[0].title = "Abilities";
    tabs[0].build = &buildAbilitiesTab;

    tabs[1].title = "Equipment";
    tabs[1].build = &buildEquipmentTab;

    tabs[2].title = "Sound";
    tabs[2].build = &buildSoundTab;

    UiWindowDesc desc = UI_WINDOW_DESC_INIT;
    desc.tabs = tabs;
    desc.tab_count = 3;
    desc.on_closed = on_carco_shop_close;

    if (svc_ui->window_push(mod_ctx, &desc, &g_carcoShopWindow) != MOD_OK) {
        mods::log::debug("failed to open carco shop");
    }
}

void UiHandler::openCarcoShop() {
    onOpenCarcoShop(mod_ctx, nullptr);
}

static ModResult buildInitialPanel(ModContext*, UiElementHandle panel, void*, ModError* error) {
    UiControlDesc control = UI_CONTROL_DESC_INIT;
    control.kind = UI_CONTROL_BUTTON;
    control.label = "Open Options Menu";
    control.on_pressed = &onOpenOptionsMenu;
    ModResult result = add_control(panel, control);
    if (result != MOD_OK) {
        return mods::set_error(error, result, "Failed to add options menu button");
    }

    control.label = "Apply To CarcoCorp Membership";
    control.on_pressed = &onApply;
    result = add_control(panel, control);
    if (result != MOD_OK) {
        return mods::set_error(error, result, "Failed to add apply button");
    }

    control.label = "Open CarcoCorp Shop";
    control.on_pressed = &onOpenCarcoShop;
    result = add_control(panel, control);
    if (result != MOD_OK) {
        return mods::set_error(error, result, "Failed to add CarcoCorp shop button");
    }

    return result;
}

ModResult UiHandler::initialize(ConfigHandler* config_handler, CarcoShop* carco_shop) {
    g_self = this;
    configHandler = config_handler;
    carcoShop = carco_shop;
    UiModsPanelDesc panelDesc = UI_MODS_PANEL_DESC_INIT;
    panelDesc.build = &buildInitialPanel;
    return svc_ui->register_mods_panel(mod_ctx, &panelDesc);
}