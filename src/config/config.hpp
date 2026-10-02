#pragma once

#include "../context.hpp"
#include <mods/svc/config.h>

#define GENERAL_SETTINGS(X) \
    X(ApplicationStatus, int64_t, STATUS_NOT_STARTED)   \
    X(ApplicationTimer, int64_t, 6000)                  \
    X(GameoverPayEnabled, bool, true)                   \
    X(PriceChangeEnabled, bool, true)                   \
    X(PriceChangeTimer, int64_t, 9000)                  \
    X(RndReturnEnabled, bool, true)                     \
    X(RndReturnChance, int64_t, 10)                     \
    X(RndReturnTimer, int64_t, 54000)                   \
    X(RndReturnMoreThanOne, bool, true)

#define SHOP_ITEMS(X)   \
    X(Dig)              \
    X(HorseRiding)      \
    X(Roll)             \
    X(Sense)            \
    X(Transform)        \
    X(Warp)             \
    X(BallAndChain)     \
    X(Bombs)            \
    X(Boomerang)        \
    X(Bow)              \
    X(Clawshot)         \
    X(DominionRod)      \
    X(DoubleClawshot)   \
    X(FishingRod)       \
    X(IronBoots)        \
    X(Lantern)          \
    X(LightSword)       \
    X(MasterSword)      \
    X(OrdonSword)       \
    X(Shield)           \
    X(Slingshot)        \
    X(Spinner)          \
    X(WoodenSword)      \
    X(LinkSfx)          \
    X(EnemySfx)         \
    X(EnvSfx)           \
    X(NpcSfx)           \
    X(Bgm)              \
    X(Ast)

#define SHOP_MSGS(X)            \
    X(Digging, Ability)         \
    X(Horse Riding, Ability)    \
    X(Rolling, Ability)         \
    X(Wolf Senses, Ability)     \
    X(Transform, Ability)       \
    X(Warp, Ability)            \
    X(Ball And Chain, Usage)    \
    X(Bombs, Usage)             \
    X(Boomerang, Usage)         \
    X(Bow, Usage)               \
    X(Clawshot, Usage)          \
    X(Dominion Rod, Usage)      \
    X(Double Clawshot, Usage)   \
    X(Fishing Rod, Usage)       \
    X(Iron Boots, Usage)        \
    X(Lantern, Usage)           \
    X(Light Sword, Usage)       \
    X(Master Sword, Usage)      \
    X(Ordon Sword, Usage)       \
    X(Shield, Usage)            \
    X(Slingshot, Usage)         \
    X(Spinner, Usage)           \
    X(Wooden Sword, Usage)      \
    X(Link SFX, Access)         \
    X(Enemy SFX, Access)        \
    X(Environment SFX, Access)  \
    X(NPC SFX, Access)          \
    X(Background Music, Access) \
    X(Audio Stream, Access)

class ConfigHandler {
public:
    #define GENERAL_FUNCS(name, type, fallback)                             \
        ConfigVarHandle getCvar##name() { return *name; }                   \
        type get##name() { return getOption<type>(*name, fallback); }       \
        void setCvar##name(type value) { setOption<type>(*name, value); }

    #define PAYWALL_GETTERS(name)                                                       \
        ConfigVarHandle getCvar##name##Paywall() { return *name##Paywall; }             \
        bool get##name##Paywall() { return getOption<bool>(*name##Paywall, false); }

    #define UNLOCKED_GETTERS(name)                                                      \
        ConfigVarHandle getCvar##name##Unlocked() { return *name##Unlocked; }           \
        bool get##name##Unlocked() { return getOption<bool>(*name##Unlocked, false); }

    #define MEMBER_GENERAL_CVARS(name, type, fallback)  \
        ConfigVarHandle* name;

    #define MEMBER_ITEM_CVARS(name)         \
        ConfigVarHandle* name##Paywall;     \
        ConfigVarHandle* name##Unlocked;

    ConfigHandler();
    ModResult initialize(ModError* error);

    template <typename T>
    T getOption(ConfigVarHandle handle, T fallback) {
        T value = fallback;
        auto result = MOD_OK;
        if constexpr (std::is_same_v<T, bool>) {
            result = svc_config->get_bool(mod_ctx, handle, &value);
        } else {
            result = svc_config->get_int(mod_ctx, handle, &value);
        }
        if (handle == 0 || result != MOD_OK) {
            return fallback;
        }
        return value;
    }

    template <typename T>
    void setOption(ConfigVarHandle handle, T value) {
        if constexpr (std::is_same_v<T, bool>) {
            svc_config->set_bool(mod_ctx, handle, value);
        } else {
            svc_config->set_int(mod_ctx, handle, value);
        }
    }

    GENERAL_SETTINGS(GENERAL_FUNCS)
    SHOP_ITEMS(PAYWALL_GETTERS)
    SHOP_ITEMS(UNLOCKED_GETTERS)

    bool getProcessApplication() { return processApplication; }
    void setProcessApplication(bool value) { processApplication = value; }
    bool getUpdateFlowFlag() { return updateFlowFlag; }
    void setUpdateFlowFlag(bool value) { updateFlowFlag = value; }
    std::vector<int64_t>& getReturnedItemIds() { return returnedItemIds; }
    std::array<uint16_t, ITEM_COUNT>& getBasePrices() { return basePrices; }
    std::array<uint16_t, ITEM_COUNT>& getPrices() { return prices; }
    bool sendLockToast(int64_t item);
    std::string getShopLockItemMsg(int64_t item) { return shopLockItemMsgs[item]; }
    bool getChangePrices() { return changePrices; }
    void setChangePrices(bool value) { changePrices = value; }
private:
    bool processApplication;
    bool updateFlowFlag;
    std::vector<int64_t> returnedItemIds;
    std::array<bool, ITEM_COUNT> sentLockToasts;
    bool changePrices;
    GENERAL_SETTINGS(MEMBER_GENERAL_CVARS)
    SHOP_ITEMS(MEMBER_ITEM_CVARS)
    std::array<uint16_t, ITEM_COUNT> basePrices = {
        15, 20, 15, 15, 25, 50, // Abilities
        50, 25, 25, 30, 40, 75, 70, 10, 20, 15, 150, 100, 15, 30, 10, 50, 5, // Equipment
        30, 45, 55, 60, 75, 75 // Sound
    };
    std::array<uint16_t, ITEM_COUNT> prices = {
        15, 20, 15, 15, 25, 50, // Abilities
        50, 25, 25, 30, 40, 75, 70, 10, 20, 15, 150, 100, 15, 30, 10, 50, 5, // Equipment
        30, 45, 55, 60, 75, 75 // Sound
    };
    std::array<std::string, ITEM_COUNT> shopLockItemMsgs;
};

using paywallCvarFunc = ConfigVarHandle (ConfigHandler::*)();
extern paywallCvarFunc paywallCvarFuncs[];


using unlockedCvarFunc = ConfigVarHandle (ConfigHandler::*)();
extern unlockedCvarFunc unlockedCvarFuncs[];

using configFunc = bool (ConfigHandler::*)();
struct shopCheckFuncs{
    configFunc pywlFunc;
    configFunc unlockedFunc;
};
extern shopCheckFuncs shopChecks[];