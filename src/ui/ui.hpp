#pragma once

#include "../context.hpp"
#include "../config/config.hpp"
#include "../shop/shop.hpp"
#include <mods/svc/ui.hpp>

#define SHOP_ITEMS_AB_UI(X)                                 \
    X(Dig, AB_DIG,                                          \
      "Lock Digging behind a paywall")                      \
    X(Horse Riding, AB_HORSE_RIDING,                        \
      "Lock Horse Riding behind a paywall")                 \
    X(Roll, AB_ROLL,                                        \
      "Lock Rolling behind a paywall")                      \
    X(Sense, AB_SENSE,                                      \
      "Lock Wolf Senses behind a paywall")                  \
    X(Transform, AB_TRANSFORM,                              \
      "Lock Transforming behind a paywall")                 \
    X(Warp, AB_WARP,                                        \
      "Lock Warping behind a paywall")

#define SHOP_ITEMS_EQ_UI(X)                                 \
    X(Ball And Chain, EQ_BALL_AND_CHAIN,                    \
      "Lock usage of the Ball And Chain behind a paywall")  \
    X(Bombs, EQ_BOMBS,                                      \
      "Lock usage of Bombs behind a paywall")               \
    X(Boomerang, EQ_BOOMERANG,                              \
      "Lock usage of the Boomerang behind a paywall")       \
    X(Bow, EQ_BOW,                                          \
      "Lock usage of the Bow behind a paywall")             \
    X(Clawshot, EQ_CLAWSHOT,                                \
      "Lock usage of the Clawshot behind a paywall")        \
    X(Dominion Rod, EQ_DOMINION_ROD,                        \
      "Lock usage of the Dominion Rod behind a paywall")    \
    X(Double Clawshot, EQ_DOUBLE_CLAWSHOT,                  \
      "Lock usage of the Double Clawshot behind a paywall") \
    X(Fishing Rod, EQ_FISHING_ROD,                          \
      "Lock usage of the Fishing rod behind a paywall")     \
    X(Iron Boots, EQ_IRON_BOOTS,                            \
      "Lock usage of the Iron Boots behind a paywall")      \
    X(Lantern, EQ_LANTERN,                                  \
      "Lock usage of the Lantern behind a paywall")         \
    X(Light Sword, EQ_LIGHT_SWORD,                          \
      "Lock usage of the Light Sword behind a paywall")     \
    X(Master Sword, EQ_MASTER_SWORD,                        \
      "Lock usage of the Master Sword behind a paywall")    \
    X(Ordon Sword, EQ_ORDON_SWORD,                          \
      "Lock usage of the Ordon Sword behind a paywall")     \
    X(Shield, EQ_SHIELD,                                    \
      "Lock usage of the Shield behind a paywall")          \
    X(Slingshot, EQ_SLINGSHOT,                              \
      "Lock usage of the Slingshot behind a paywall")       \
    X(Spinner, EQ_SPINNER,                                  \
      "Lock usage of the Spinner behind a paywall")         \
    X(Wooden Sword, EQ_WOODEN_SWORD,                        \
      "Lock usage of the Wooden Sword behind a paywall")

#define SHOP_ITEMS_SND_UI(X)                                \
    X(Link SFX, SND_LINK_SFX,                               \
      "Lock Link sound effects behind a paywall")           \
    X(Enemy Sfx, SND_ENEMY_SFX,                             \
      "Lock enemy sound effects behind a paywall")          \
    X(Env Sfx, SND_ENV_SFX,                                 \
      "Lock environment sound effects behind a paywall")    \
    X(Npc Sfx, SND_NPC_SFX,                                 \
      "Lock NPC sound effects behind a paywall")            \
    X(BGM, SND_BGM,                                         \
      "Lock Background Music behind a paywall")             \
    X(AST, SND_AST,                                         \
      "Lock audio streams (mostly used in cutscenes) behind a paywall")

class UiHandler {
public:
    ModResult initialize(ConfigHandler* config_handler, CarcoShop* carco_shop);
    ConfigHandler* getConfigHandler() { return configHandler; }
    CarcoShop* getCarcoShop() { return carcoShop; }
    void openCarcoShop();
private:
    ConfigHandler* configHandler;
    CarcoShop* carcoShop;
};