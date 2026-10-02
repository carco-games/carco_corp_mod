#include "shop.hpp"
#include "d/d_com_inf_game.h"

#include <random>
#include <algorithm>

void CarcoShop::initialize(ConfigHandler* config_handler) {
    configHandler = config_handler;
}

bool CarcoShop::checkFunds(uint16_t price) {
    uint16_t rupees = dComIfGs_getRupee();
    if ((rupees - price) < 0) {
        mDoAud_seStart(Z2SE_SY_FILE_ERROR, NULL, 0, 0);
        UiToastDesc desc = UI_TOAST_DESC_INIT;
        desc.title_rml = "CarcoCorp";
        desc.duration_ms = 1500;
        desc.body_rml = "You don't have enough Rupees to buy this";
        svc_ui->push_toast(mod_ctx, &desc);
        return false;
    }

    return true;
}

void CarcoShop::checkReturnItem(int64_t itemID) {
    auto& ids = configHandler->getReturnedItemIds();
    ids.erase(
        std::remove(ids.begin(), ids.end(), itemID),
        ids.end()
    );
}

void CarcoShop::unlockItem(ConfigVarHandle cvar, uint16_t price) {
    configHandler->setOption<bool>(cvar, true);
    mDoAud_seStart(Z2SE_SY_FILE_SAVE_OK, NULL, 0, 0);
    dComIfGs_setRupee(dComIfGs_getRupee() - price);
    UiToastDesc desc = UI_TOAST_DESC_INIT;
    desc.title_rml = "Carco Shop";
    desc.duration_ms = 1500;
    desc.body_rml = "Successfully Bought Item!";
    svc_ui->push_toast(mod_ctx, &desc);
    configHandler->setChangePrices(true);
}

void CarcoShop::lockItem(ConfigVarHandle cvar, int64_t item_id) {
    if ((configHandler->*shopChecks[item_id].unlockedFunc)()) {
        mDoAud_seStart(Z2SE_SY_FILE_DELETE_OK, NULL, 0, 0);
        configHandler->setOption<bool>(cvar, false);
        UiToastDesc desc = UI_TOAST_DESC_INIT;
        desc.title_rml = "Carco Shop";
        desc.duration_ms = 2500;
        std::string body_msg = "Oops! It seems there was a mistake on our end!\n";
        body_msg = body_msg + "But we will have to take your " + configHandler->getShopLockItemMsg(item_id) + "!";
        desc.body_rml = body_msg.c_str();
        svc_ui->push_toast(mod_ctx, &desc);
    }
}

void CarcoShop::changePrices() {
    if (configHandler->getChangePrices()) {
        std::array<uint16_t, ITEM_COUNT>& prices = configHandler->getPrices();
        prices = configHandler->getBasePrices();
        std::random_device random;
        std::mt19937 gen(random());
        std::uniform_int_distribution<int> rnd_percent(-50, 100);
        for (int64_t i = 0; i < prices.size(); i++) {
            int64_t percentChange = rnd_percent(gen);
            prices[i] = prices[i] * (100 + percentChange) / 100;
        }
        configHandler->setChangePrices(false);
    }
}

void CarcoShop::randomReturn() {
    std::random_device random;
    std::mt19937 gen(random());
    std::uniform_int_distribution<int> dist(1, 100);

    if (dist(gen) <= configHandler->getRndReturnChance()) {
        // Execute random return
        std::uniform_int_distribution<int> rnd_item(0, ITEM_COUNT - 1);
        int64_t item = rnd_item(gen);

        // Check if item has already been returned
        if (std::find(
                configHandler->getReturnedItemIds().begin(),
                configHandler->getReturnedItemIds().end(),
                item
            ) != configHandler->getReturnedItemIds().end()) {
            // Item has already been returned
            return;
        }

        // If player has double clawshots and random item is the clawshot,
        // lock the double clawshots instead of the regular clawshot
        if (dComIfGs_getItem(SLOT_10, false) == dItemNo_W_HOOKSHOT_e && item == EQ_CLAWSHOT) {
            item = EQ_DOUBLE_CLAWSHOT;
        }


        ConfigVarHandle cvar = (configHandler->*unlockedCvarFuncs[item])();
        lockItem(cvar, item);

        configHandler->getReturnedItemIds().emplace_back(item);
    }
}