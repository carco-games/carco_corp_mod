#include "sound_hooks.hpp"
#include "d/actor/d_a_alink.h"
#include "Z2AudioLib/Z2AudioMgr.h"
#include "Z2AudioLib/Z2SeMgr.h"
#include "Z2AudioLib/Z2Creature.h"
#include "Z2AudioLib/Z2SeqMgr.h"
#include "Z2AudioLib/Z2SceneMgr.h"

SoundHooksHandler* g_self;

// All Sounds
#ifdef _MSVC_LANG
#define Z2SoundStarter_startSound_sig_1 "?startSound@Z2SoundStarter@@UEAA_NVJAISoundID@@PEAVJAISoundHandle@@PEBU?$TVec3@M@JGeometry@@IMMMMMI@Z"
#else
#define Z2SoundStarter_startSound_sig_1 "_ZN14Z2SoundStarter10startSoundE10JAISoundIDP14JAISoundHandlePKN9JGeometry5TVec3IfEEjfffffj"
#endif
DEFINE_HOOK_SYMBOL(Z2SoundStarter_startSound_sig_1, bool(Z2SoundStarter*, JAISoundID, JAISoundHandle*, const JGeometry::TVec3<f32>*,
                   u32, f32, f32, f32, f32, f32, u32), StartSound);

#ifdef _MSVC_LANG
#define Z2SoundStarter_startSound_sig_2 "?startSound@Z2SoundStarter@@UEAA_NVJAISoundID@@PEAVJAISoundHandle@@PEBU?$TVec3@M@JGeometry@@@Z"
#else
#define Z2SoundStarter_startSound_sig_2 "_ZN14Z2SoundStarter10startSoundE10JAISoundIDP14JAISoundHandlePKN9JGeometry5TVec3IfEE"
#endif
DEFINE_HOOK_SYMBOL(Z2SoundStarter_startSound_sig_2, bool(Z2SoundStarter*, JAISoundID, JAISoundHandle*, const JGeometry::TVec3<f32>*), StartSound2);

// BGM Hooks
DEFINE_HOOK(&Z2SeqMgr::bgmStart, BgmStart);
DEFINE_HOOK(&Z2SeqMgr::subBgmStart, SubBgmStart);
DEFINE_HOOK(&Z2SeqMgr::startBattleBgm, StartBattleBgm);
DEFINE_HOOK(&Z2SeqMgr::changeBgmStatus, ChangeBgmStatus);
DEFINE_HOOK(&Z2SeqMgr::fieldBgmStart, FieldBgmStart);
DEFINE_HOOK(&Z2SceneMgr::sceneBgmStart, SceneBgmStart);

// AST Hooks
DEFINE_HOOK(&Z2SeqMgr::bgmStreamPrepare, BgmStreamPrepare);
DEFINE_HOOK(&Z2SeqMgr::bgmStreamPlay, BgmStreamPlay);
DEFINE_HOOK_SYMBOL("JAIStreamMgr::startSound", bool(JAISoundID, JAISoundHandle*, const JGeometry::TVec3<f32>*), JAIStreamMgrStartSound);

HookAction onStartSound(ModContext*, void* args, void*, void*) {
    JAISoundID soundID = mods::arg<JAISoundID>(args, 1);

    // Link SFX
    if ((soundID >= Z2SE_AL_V_ATTACK_S && soundID <= Z2SE_AL_KICK_HORSE) ||
        (soundID >= Z2SE_AL_SET_CHAIN && soundID <= Z2SE_WOLFATTACK_WIND_RUSH) ||
        (soundID >= Z2SE_AL_SWING_BOTTLE && soundID <= Z2SE_AL_PADDLING_BACKWARD) ||
        (soundID >= Z2SE_AL_SWORD_SHIELD_ADD && soundID <= Z2SE_AL_MAGNETIZED) ||
        soundID == Z2SE_AL_ITEM_TAKEOUT || (soundID >= JA_SE_LK_BOOM_FLY && soundID <= Z2SE_D06_WLF_SWING_BD) ||
        (soundID >= Z2SE_AL_M_ARMER_TURNOFF && soundID <= Z2SE_AL_BACKTEN_WIND) ||
        (soundID >= Z2SE_AL_SWIM && soundID <= Z2SE_WL_DIG_GROUND) ||
        (soundID >= Z2SE_AL_SWIM_DASH && soundID <= Z2SE_WL_SWIM_DASH) ||
        (soundID >= Z2SE_WLF_V_DEMO_BREATH_RUN && soundID <= Z2SE_D07V_WLF_UNARI_TRIG) ||
        (soundID >= Z2SE_WLF_V_DEMO_DAMAGE_S && soundID <= Z2SE_D20V_WLF_WAIT_DAMAGE) ||
        soundID == Z2SE_D01V_LINK_SMILE_KLN || soundID == Z2SE_D01V_LINK_SMILE_YLA ||
        soundID == Z2SE_D16V_LINK_NOD || soundID == Z2SE_D35V_LINK_MIHIRAKI || soundID == Z2SE_D35V_LINK_LOOK_THE ||
        (soundID >= Z2SE_D35V_LINK_NOTICE_1 && soundID <= Z2SE_D37V_LINK_SMILE) || soundID == Z2SE_D17V_LINK_SURPRISE ||
        (soundID >= Z2SE_D17V_LINK_RELIEF_KLN && soundID <= Z2SE_D19V_LINK_SHOULDER_HIT) || soundID == Z2SE_D19V_LINK_CANNOT_SAY ||
        soundID == Z2SE_D19V_LINK_KIDUKU || soundID == Z2SE_D19V_LINK_YLA_BACK || soundID == Z2SE_D19V_LINK_M_LOST_YLA ||
        soundID == Z2SE_D17V_LINK_NOTICE || (soundID >= Z2SE_D18V_LINK_IN_THE_DARK && soundID <= Z2SE_D18V_LINK_SMILING) ||
        (soundID >= Z2SE_D18V_LINK_B_SMILE && soundID <= Z2SE_D18V_LINK_B_SHOUT) || (soundID >= Z2SE_D18V_LINK_WAKEUP && soundID <= Z2SE_D18V_LINK_BREATH) ||
        (soundID >= Z2SE_D20V_LINK_LOOKBACK && soundID <= Z2SE_D20V_LINK_SHOUT) || (soundID >= Z2SE_D22V_LINK_PULLING && soundID <= Z2SE_D23V_LINK_NOD) ||
        (soundID >= Z2SE_D22V_LINK_SWING_1 && soundID <= Z2SE_D22V_LINK_SWING_3) || (soundID >= Z2SE_D27V_LINK_LOOK_ZELDA && soundID <= Z2SE_D27V_LINK_STOP_BY_MDN) ||
        soundID == Z2SE_D27V_LINK_AH_MDN_FLY || (soundID >= Z2SE_D27V_LINK_FOLLOW_MDN && soundID <= Z2SE_D27V_LINK_LOOKBACK) || soundID == Z2SE_D28V_LINK_LOOK_GNN ||
        (soundID >= Z2SE_D28V_LINK_BACK && soundID <= Z2SE_D28V_LINK_RUMBLE) || (soundID >= Z2SE_D28V_LINK_LOOK_MASK && soundID <= Z2SE_D28V_LINK_COME_HORSE) ||
        (soundID >= Z2SE_D30V_LINK_NOTICE_MDN && soundID <= Z2SE_D30V_LINK_SURPRISE) || soundID == Z2SE_D30V_LINK_LOOKBACK || soundID == Z2SE_LINK_COVER_WATER) {
        return CHECK_LOCK(SND_LINK_SFX);
    }

    // Enemy SFX
    if ((soundID >= Z2SE_EN_WB_L_SLIP && soundID <= Z2SE_EN_WB_KICK_GROUND) || (soundID >= Z2SE_EN_BB_KICK_GROUND && soundID <= Z2SE_EN_BB_FN_WALK_R) ||
        (soundID >= Z2SE_EN_WB_V_INANAKI && soundID <= Z2SE_EN_WB_V_RIDE) || (soundID >= Z2SE_EN_SINEWAVE && soundID <= Z2SE_EN_PH_APPEAR) ||
        (soundID == Z2SE_EN_GND_V_DAMAGE_DOWN) || (soundID == Z2SE_EN_HZE_V_ATK_C_RETURN) ||
        (soundID >= Z2SE_EN_MGN_V_TURN && soundID <= Z2SE_EN_KC_V_NAKU)) {
        return CHECK_LOCK(SND_ENEMY_SFX);
    }

    // NPC/Creature SFX
    if ((soundID >= Z2SE_HS_V_CRY && soundID <= Z2SE_KN_V_JUMP_ATTACK_L) || (soundID >= Z2SE_M053_KOLIN_03 && soundID <= Z2SE_M058_TARO_05) ||
        (soundID >= Z2SE_POST_V_RUN_HIGH && soundID <= Z2SE_KOSARU_V_DELIGHT) || (soundID >= Z2SE_MK_V_COME_BACK && soundID <= Z2SE_GRN_V_SUMO_FALL_LOSE) ||
        (soundID >= Z2SE_YELIA_V_KYAAA_TRIG && soundID <= Z2SE_D22V_WLF_LONG_BARK) || soundID == Z2SE_GORON_RECOVER || soundID == Z2SE_MIDNA_JUMP ||
        soundID == Z2SE_G_WLF_UNARU) {
        return CHECK_LOCK(SND_NPC_SFX);
    }

    // For all other SFX, just check Environment SFX Lock
    return CHECK_LOCK(SND_ENV_SFX);
}

HookAction bgmCheck(ModContext*, void*, void*, void*) {
    return CHECK_LOCK(SND_BGM);
}

HookAction astCheck(ModContext*, void*, void*, void*) {
    return CHECK_LOCK(SND_AST);
}

HookAction onZ2SoundMgrStartSound(ModContext* mod_ctx, void* args, void* retval, void* user_data) {
    JAISoundID soundID = mods::arg<JAISoundID>(args, 1);
    if (soundID >= 0x2000000) {
        return CHECK_LOCK(SND_AST);
    }

    return HOOK_CONTINUE;
}

SoundHooksHandler::SoundHooksHandler(ConfigHandler* config_handler) {
    configHandler = config_handler;
    g_self = this;
    soundLockedToastStatus = true;
}

ModResult SoundHooksHandler::initialize(ModError* error) {
    ModResult result;

    // Common Sound Starter
    PRE_HOOK(StartSound, onStartSound, "Failed to hook startSound");
    PRE_HOOK(StartSound2, onStartSound, "Failed to hook startSound");

    // BGM
    PRE_HOOK(BgmStart, bgmCheck, "Failed to hook bgmStart");
    PRE_HOOK(SubBgmStart, bgmCheck, "Failed to hook subBgmStart");
    PRE_HOOK(StartBattleBgm, bgmCheck, "Failed to hook startBattleBgm");
    PRE_HOOK(ChangeBgmStatus, bgmCheck, "Failed to hook changeBgmStatus");
    PRE_HOOK(FieldBgmStart, bgmCheck, "Failed to hook fieldBgmStart");
    PRE_HOOK(SceneBgmStart, bgmCheck, "Failed to hook sceneBgmStart");

    // AST
    PRE_HOOK(BgmStreamPrepare, astCheck, "Failed to hook bgmStreamPrepare");
    PRE_HOOK(BgmStreamPlay, astCheck, "Failed to hook bgmStreamPlay");
    PRE_HOOK(JAIStreamMgrStartSound, onZ2SoundMgrStartSound, "Failed to hook JAIStreamMgr::startSound");
    return result;
}