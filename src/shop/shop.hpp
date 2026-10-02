#pragma once

#include "../context.hpp"
#include "../config/config.hpp"
#include <mods/svc/ui.hpp>

class CarcoShop {
public:
    void initialize(ConfigHandler* config_handler);
    bool checkFunds(uint16_t price);
    void checkReturnItem(int64_t itemID);
    void unlockItem(ConfigVarHandle cvar, uint16_t price);
    void lockItem(ConfigVarHandle cvar, int64_t item_id);
    void changePrices();
    void randomReturn();
private:
    ConfigHandler* configHandler;
};