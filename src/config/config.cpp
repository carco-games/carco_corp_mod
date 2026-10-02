#include "config.hpp"
#include "mods/svc/log.hpp"
#include <mods/svc/ui.hpp>

#include <variant>

IMPORT_SERVICE(ConfigService, svc_config);

#define GENERAL_CVARS(name, type, default_value)    \
    ConfigVarHandle g_cvar##name##;

#define ITEM_CVARS(name)                    \
    ConfigVarHandle g_cvar##name##Paywall;  \
    ConfigVarHandle g_cvar##name##Unlocked;

#define REGISTER_GENERAL(name, type, default_value) \
    {#name, default_value, g_cvar##name##},

#define REGISTER_PAYWALL(name)  \
    {#name "Paywall", true, g_cvar##name##Paywall},

#define REGISTER_UNLOCK(name)  \
    {#name "Unlocked", false, g_cvar##name##Unlocked},

#define GET_PAYWALL_CVAR_FUNC(name)  \
    &ConfigHandler::getCvar##name##Paywall,

#define GET_UNLOCKED_CVAR_FUNC(name)  \
    &ConfigHandler::getCvar##name##Unlocked,

#define GET_BOTH_FUNCS(name)                \
    {&ConfigHandler::get##name##Paywall,    \
     &ConfigHandler::get##name##Unlocked},

// Global cvars
GENERAL_SETTINGS(GENERAL_CVARS)
SHOP_ITEMS(ITEM_CVARS);

paywallCvarFunc paywallCvarFuncs[] = {
    SHOP_ITEMS(GET_PAYWALL_CVAR_FUNC)
};

unlockedCvarFunc unlockedCvarFuncs[] = {
    SHOP_ITEMS(GET_UNLOCKED_CVAR_FUNC)
};

shopCheckFuncs shopChecks[] = {
    SHOP_ITEMS(GET_BOTH_FUNCS)
};

// Helpers
using DefaultValue = std::variant<bool, int64_t, std::string, float>;
struct cVarRegistration {
    const char* name;
    DefaultValue defaultValue;
    ConfigVarHandle& cVar;
};

template <typename T>
ModResult register_option(const char* name, T defaultValue, ConfigVarHandle& outHandle, ModError* error) {
    ConfigVarDesc cvarDesc = CONFIG_VAR_DESC_INIT;
    cvarDesc.name = name;
    if constexpr (std::is_same_v<T, bool>) {
        cvarDesc.type = CONFIG_VAR_BOOL;
        cvarDesc.default_bool = defaultValue;
    } else if constexpr (std::is_same_v<T, int64_t>) {
        cvarDesc.type = CONFIG_VAR_INT;
        cvarDesc.default_int = defaultValue;
    } else if constexpr (std::is_same_v<T, std::string>) {
        cvarDesc.type = CONFIG_VAR_STRING;
        cvarDesc.default_string = defaultValue.c_str();
    } else if constexpr (std::is_same_v<T, float>) {
        cvarDesc.type = CONFIG_VAR_FLOAT;
        cvarDesc.default_float = defaultValue;
    }

    if (svc_config->register_var(mod_ctx, &cvarDesc, &outHandle) != MOD_OK) {
        return mods::set_error(error, MOD_ERROR, "failed to register CarcoCorp option");
    }
    return MOD_OK;
}

ModResult register_cvar(const char* name, const DefaultValue& value, ConfigVarHandle& handle, ModError* error) {
    return std::visit([&](const auto& value) -> ModResult {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, bool>) {
            return register_option<bool>(name, value, handle, error);
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return register_option<int64_t>(name, value, handle, error);
        } else if constexpr (std::is_same_v<T, std::string>) {
            return register_option<std::string>(name, value, handle, error);
        } else if constexpr (std::is_same_v<T, float>) {
            return register_option<float>(name, value, handle, error);
        }
    }, value);
}

static constexpr cVarRegistration registrations[] = {
    GENERAL_SETTINGS(REGISTER_GENERAL)
    SHOP_ITEMS(REGISTER_PAYWALL)
    SHOP_ITEMS(REGISTER_UNLOCK)
};

#define MAP_GENERAL_CVARS(name, type, default_value)    \
    name## = &g_cvar##name##;

#define MAP_ITEM_CVARS(name)                    \
    name##Paywall = &g_cvar##name##Paywall;     \
    ##name##Unlocked = &g_cvar##name##Unlocked;

#define BUILD_SHOP_MSG(name, suffix)    \
    #name " " #suffix,

ConfigHandler::ConfigHandler() {
    GENERAL_SETTINGS(MAP_GENERAL_CVARS)
    SHOP_ITEMS(MAP_ITEM_CVARS)
    for (int64_t i = 0; i < ITEM_COUNT; i++) {
        sentLockToasts[i] = false;
    }

    shopLockItemMsgs = {
        SHOP_MSGS(BUILD_SHOP_MSG)
    };

    changePrices = true;
}

ModResult ConfigHandler::initialize(ModError* error) {
    for (auto& reg : registrations) {
        ModResult result = register_cvar(reg.name, reg.defaultValue, reg.cVar, error);
        if (result != MOD_OK) {
            return mods::set_error(error, result, "Failed to register CarcoCorp cvar");
        }
    }

    return MOD_OK;
}

bool ConfigHandler::sendLockToast(int64_t item) {
    if (!sentLockToasts[item]) {
        UiToastDesc desc = UI_TOAST_DESC_INIT;
        desc.title_rml = "CarcoCorp";
        desc.duration_ms = 2000;
        std::string body = "You haven't bought CarcoCorp's\n " +
            shopLockItemMsgs[item] + "!";
        desc.body_rml = body.c_str();
        svc_ui->push_toast(mod_ctx, &desc);
        sentLockToasts[item] = true;
    }

    return true;
}