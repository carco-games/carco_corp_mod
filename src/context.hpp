#pragma once

#include "mods/service.hpp"
#include "mods/svc/log.hpp"
#include <string>
#include <algorithm>
#include <vector>

extern "C" {
    extern ModContext* mod_ctx;
}

enum ApplicationStatus {
    /* 0x0 */ STATUS_NOT_STARTED,
    /* 0x1 */ STATUS_ONGOING,
    /* 0x2 */ STATUS_FINISHED,
};

enum FlowUpdateStatus {
    /* 0x0 */ STATUS_NOT_UPDATED,
    /* 0x1 */ STATUS_PROCESSING_UPDATED,
    /* 0x2 */ STATUS_FINISHED_UPDATED,
};

enum ShopItems_e {
    /* 0x00 */ AB_DIG,
    /* 0x01 */ AB_HORSE_RIDING,
    /* 0x02 */ AB_ROLL,
    /* 0x03 */ AB_SENSE,
    /* 0x04 */ AB_TRANSFORM,
    /* 0x05 */ AB_WARP,
    /* 0x06 */ EQ_BALL_AND_CHAIN,
    /* 0x07 */ EQ_BOMBS,
    /* 0x08 */ EQ_BOOMERANG,
    /* 0x09 */ EQ_BOW,
    /* 0x0A */ EQ_CLAWSHOT,
    /* 0x0B */ EQ_DOMINION_ROD,
    /* 0x0C */ EQ_DOUBLE_CLAWSHOT,
    /* 0x0D */ EQ_FISHING_ROD,
    /* 0x0E */ EQ_IRON_BOOTS,
    /* 0x0F */ EQ_LANTERN,
    /* 0x10 */ EQ_LIGHT_SWORD,
    /* 0x11 */ EQ_MASTER_SWORD,
    /* 0x12 */ EQ_ORDON_SWORD,
    /* 0x13 */ EQ_SHIELD,
    /* 0x14 */ EQ_SLINGSHOT,
    /* 0x15 */ EQ_SPINNER,
    /* 0x16 */ EQ_WOODEN_SWORD,
    /* 0x17 */ SND_LINK_SFX,
    /* 0x18 */ SND_ENEMY_SFX,
    /* 0x19 */ SND_ENV_SFX,
    /* 0x1A */ SND_NPC_SFX,
    /* 0x1B */ SND_BGM,
    /* 0x1C */ SND_AST,
    ITEM_COUNT
};