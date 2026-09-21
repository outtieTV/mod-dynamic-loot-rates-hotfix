#include "ScriptMgr.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Config.h"
#include "Map.h"

struct DynamicLootRatesConfig
{
    bool enabled = true;

    uint32 dungeonLootGroupRate = 1;
    uint32 dungeonLootReferenceRate = 1;

    uint32 raidLootGroupRate = 1;
    uint32 raidLootReferenceRate = 1;
};

// Module-specific name.
// Do not use a generic global name such as "config" here because
// AzerothCore modules are linked together into libmodules.a.
DynamicLootRatesConfig dynamicLootRatesConfig;

class DynamicLootRates_WorldScript : public WorldScript
{
public:
    DynamicLootRates_WorldScript()
        : WorldScript("DynamicLootRates_WorldScript")
    {
    }

    void OnBeforeConfigLoad(bool /*reload*/) override
    {
        dynamicLootRatesConfig.enabled =
            sConfigMgr->GetOption<bool>(
                "DynamicLootRates.Enable",
                true);

        dynamicLootRatesConfig.dungeonLootGroupRate =
            sConfigMgr->GetOption<uint32>(
                "DynamicLootRates.Dungeon.Rate.GroupAmount",
                1);

        dynamicLootRatesConfig.dungeonLootReferenceRate =
            sConfigMgr->GetOption<uint32>(
                "DynamicLootRates.Dungeon.Rate.ReferencedAmount",
                1);

        dynamicLootRatesConfig.raidLootGroupRate =
            sConfigMgr->GetOption<uint32>(
                "DynamicLootRates.Raid.Rate.GroupAmount",
                1);

        dynamicLootRatesConfig.raidLootReferenceRate =
            sConfigMgr->GetOption<uint32>(
                "DynamicLootRates.Raid.Rate.ReferencedAmount",
                1);

        LOG_INFO(
            "module",
            "mod_dynamic_loot_rates: Enabled: {}, "
            "Dungeon Group: {}, Dungeon Reference: {}, "
            "Raid Group: {}, Raid Reference: {}",
            dynamicLootRatesConfig.enabled,
            dynamicLootRatesConfig.dungeonLootGroupRate,
            dynamicLootRatesConfig.dungeonLootReferenceRate,
            dynamicLootRatesConfig.raidLootGroupRate,
            dynamicLootRatesConfig.raidLootReferenceRate);
    }
};

class DynamicLootRates_GlobalScript : public GlobalScript
{
public:
    DynamicLootRates_GlobalScript()
        : GlobalScript("DynamicLootRates_GlobalScript")
    {
    }

    void OnAfterCalculateLootGroupAmount(
        Player const* player,
        Loot& /*loot*/,
        uint16 /*lootMode*/,
        uint32& groupAmount,
        LootStore const& /*store*/) override
    {
        if (!dynamicLootRatesConfig.enabled || !player)
            return;

        Map* map = player->GetMap();

        if (!map)
            return;

        if (IsDungeon(map))
        {
            groupAmount = dynamicLootRatesConfig.dungeonLootGroupRate;

            LOG_DEBUG(
                "module",
                "mod_dynamic_loot_rates: "
                "In dungeon: Applying loot group rate of {}",
                groupAmount);

            return;
        }

        if (IsRaid(map))
        {
            groupAmount = dynamicLootRatesConfig.raidLootGroupRate;

            LOG_DEBUG(
                "module",
                "mod_dynamic_loot_rates: "
                "In raid: Applying loot group rate of {}",
                groupAmount);

            return;
        }
    }

    void OnAfterRefCount(
        Player const* player,
        LootStoreItem* /*lootStoreItem*/,
        Loot& /*loot*/,
        bool /*canRate*/,
        uint16 /*lootMode*/,
        uint32& maxcount,
        LootStore const& /*store*/) override
    {
        if (!dynamicLootRatesConfig.enabled || !player)
            return;

        Map* map = player->GetMap();

        if (!map)
            return;

        if (IsDungeon(map))
        {
            // This must use the reference rate, not the group rate.
            maxcount = dynamicLootRatesConfig.dungeonLootReferenceRate;

            LOG_DEBUG(
                "module",
                "mod_dynamic_loot_rates: "
                "In dungeon: Applying loot reference rate of {}",
                maxcount);

            return;
        }

        if (IsRaid(map))
        {
            // This must use the reference rate, not the group rate.
            maxcount = dynamicLootRatesConfig.raidLootReferenceRate;

            LOG_DEBUG(
                "module",
                "mod_dynamic_loot_rates: "
                "In raid: Applying loot reference rate of {}",
                maxcount);

            return;
        }
    }

private:
    static bool IsDungeon(Map* map)
    {
        return map && map->IsDungeon() && !map->IsRaid();
    }

    static bool IsRaid(Map* map)
    {
        return map && map->IsRaid();
    }
};

// Add all scripts in one.
void AddDynamicLootRateScripts()
{
    new DynamicLootRates_WorldScript();
    new DynamicLootRates_GlobalScript();
}
