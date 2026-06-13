"""
create_datatables.py
Snowpiercer: Eternal Engine - DataTable Creation Script

Run inside UE5 Editor via: Edit > Execute Python Script
                         or: py "Scripts/create_datatables.py" from the UE5 console

Creates all game DataTables (Items, Dialogue, Quests, Collectibles)
with full row data matching the C++ FTableRowBase structs.

Falls back to JSON export if the unreal Python module is unavailable
(e.g., running outside the editor for data review / CI validation).
"""

import os
import sys
import json
import logging
from pathlib import Path

# ---------------------------------------------------------------------------
# Logging
# ---------------------------------------------------------------------------
logging.basicConfig(
    level=logging.INFO,
    format="[SEE-DataTables] %(levelname)s  %(message)s",
)
log = logging.getLogger("SEE-DataTables")

# ---------------------------------------------------------------------------
# Detect environment
# ---------------------------------------------------------------------------
try:
    import unreal  # type: ignore
    IN_EDITOR = True
    log.info("Running inside UE5 Editor — will create .uasset DataTables.")
except ImportError:
    IN_EDITOR = False
    log.info(
        "unreal module not found — running in standalone mode. "
        "JSON files will be written to DataTables/ for manual import."
    )

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
CONTENT_DT_PATH = "/Game/DataTables"
SCRIPT_DIR = Path(__file__).resolve().parent
JSON_OUTPUT_DIR = SCRIPT_DIR.parent / "DataTables"


# ===================================================================
#  DATA DEFINITIONS
# ===================================================================

# ----- 1. ITEMS  (FSEEItemData, row‑name = ItemID) ----

# Enum ordinals must match C++ declaration order
ITEM_CATEGORY = {
    "Weapon": 0, "Armor": 1, "Consumable": 2,
    "Crafting": 3, "Quest": 4, "Junk": 5,
}
ITEM_RARITY = {"Common": 0, "Uncommon": 1, "Rare": 2, "Legendary": 3}

ITEMS = [
    # --- Consumables ---
    {
        "RowName": "Item_ProteinBlock",
        "ItemID": "Item_ProteinBlock",
        "DisplayName": "Protein Block",
        "Description": "A gelatinous slab of processed protein. It sustains life, barely. The less you know about the ingredients, the better.",
        "Category": ITEM_CATEGORY["Consumable"],
        "Rarity": ITEM_RARITY["Common"],
        "Weight": 0.3,
        "MaxStackSize": 20,
        "Value": 5,
        "HealthRestore": 0.0,
        "HungerRestore": 25.0,
        "StaminaRestore": 5.0,
        "BluntArmor": 0.0, "BladedArmor": 0.0, "PiercingArmor": 0.0, "ColdResistance": 0.0,
    },
    {
        "RowName": "Item_RatMeat",
        "ItemID": "Item_RatMeat",
        "DisplayName": "Rat Meat",
        "Description": "A skinned rodent carcass, surprisingly plump. Cook it first unless you fancy intestinal parasites.",
        "Category": ITEM_CATEGORY["Consumable"],
        "Rarity": ITEM_RARITY["Common"],
        "Weight": 0.2,
        "MaxStackSize": 10,
        "Value": 3,
        "HealthRestore": 5.0,
        "HungerRestore": 15.0,
        "StaminaRestore": 0.0,
        "BluntArmor": 0.0, "BladedArmor": 0.0, "PiercingArmor": 0.0, "ColdResistance": 0.0,
    },
    {
        "RowName": "Item_WaterRation",
        "ItemID": "Item_WaterRation",
        "DisplayName": "Water Ration",
        "Description": "One day's allocation of recycled water. Slightly metallic, wholly indispensable.",
        "Category": ITEM_CATEGORY["Consumable"],
        "Rarity": ITEM_RARITY["Common"],
        "Weight": 0.5,
        "MaxStackSize": 10,
        "Value": 8,
        "HealthRestore": 0.0,
        "HungerRestore": 10.0,
        "StaminaRestore": 20.0,
        "BluntArmor": 0.0, "BladedArmor": 0.0, "PiercingArmor": 0.0, "ColdResistance": 0.0,
    },
    {
        "RowName": "Item_Moonshine",
        "ItemID": "Item_Moonshine",
        "DisplayName": "Tail Moonshine",
        "Description": "Distilled somewhere in the back three cars. Burns going down. Restores courage — and not much else.",
        "Category": ITEM_CATEGORY["Consumable"],
        "Rarity": ITEM_RARITY["Uncommon"],
        "Weight": 0.4,
        "MaxStackSize": 5,
        "Value": 15,
        "HealthRestore": 5.0,
        "HungerRestore": 0.0,
        "StaminaRestore": 30.0,
        "BluntArmor": 0.0, "BladedArmor": 0.0, "PiercingArmor": 0.0, "ColdResistance": 0.0,
    },
    {
        "RowName": "Item_Bandage",
        "ItemID": "Item_Bandage",
        "DisplayName": "Bandage",
        "Description": "Torn cloth strips boiled in recycled water. Basic first aid — stops the bleeding, doesn't fix the problem.",
        "Category": ITEM_CATEGORY["Consumable"],
        "Rarity": ITEM_RARITY["Common"],
        "Weight": 0.1,
        "MaxStackSize": 15,
        "Value": 10,
        "HealthRestore": 15.0,
        "HungerRestore": 0.0,
        "StaminaRestore": 0.0,
        "BluntArmor": 0.0, "BladedArmor": 0.0, "PiercingArmor": 0.0, "ColdResistance": 0.0,
    },
    {
        "RowName": "Item_Painkillers",
        "ItemID": "Item_Painkillers",
        "DisplayName": "Painkillers",
        "Description": "Pre-Freeze pharmaceuticals, rationed by the Bureaucracy. Numbs pain and sharpens focus temporarily.",
        "Category": ITEM_CATEGORY["Consumable"],
        "Rarity": ITEM_RARITY["Uncommon"],
        "Weight": 0.05,
        "MaxStackSize": 10,
        "Value": 25,
        "HealthRestore": 10.0,
        "HungerRestore": 0.0,
        "StaminaRestore": 15.0,
        "BluntArmor": 0.0, "BladedArmor": 0.0, "PiercingArmor": 0.0, "ColdResistance": 0.0,
    },
    {
        "RowName": "Item_Antiseptic",
        "ItemID": "Item_Antiseptic",
        "DisplayName": "Antiseptic",
        "Description": "Medical-grade disinfectant from the Sickbay stores. Prevents infection — a luxury in the Tail.",
        "Category": ITEM_CATEGORY["Consumable"],
        "Rarity": ITEM_RARITY["Rare"],
        "Weight": 0.15,
        "MaxStackSize": 5,
        "Value": 40,
        "HealthRestore": 25.0,
        "HungerRestore": 0.0,
        "StaminaRestore": 0.0,
        "BluntArmor": 0.0, "BladedArmor": 0.0, "PiercingArmor": 0.0, "ColdResistance": 0.0,
    },
    {
        "RowName": "Item_Kronole",
        "ItemID": "Item_Kronole",
        "DisplayName": "Kronole",
        "Description": "A potent narcotic originally used as an industrial waste solvent. Highly addictive. The Kronole Network's currency of choice.",
        "Category": ITEM_CATEGORY["Consumable"],
        "Rarity": ITEM_RARITY["Rare"],
        "Weight": 0.05,
        "MaxStackSize": 5,
        "Value": 50,
        "HealthRestore": -10.0,
        "HungerRestore": 0.0,
        "StaminaRestore": 50.0,
        "BluntArmor": 0.0, "BladedArmor": 0.0, "PiercingArmor": 0.0, "ColdResistance": 0.0,
    },
    # --- Weapons ---
    {
        "RowName": "Item_RustyPipe",
        "ItemID": "Item_RustyPipe",
        "DisplayName": "Rusty Pipe",
        "Description": "A length of corroded plumbing ripped from the wall. Heavy, unwieldy, effective.",
        "Category": ITEM_CATEGORY["Weapon"],
        "Rarity": ITEM_RARITY["Common"],
        "Weight": 2.5,
        "MaxStackSize": 1,
        "Value": 10,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 0.0, "BladedArmor": 0.0, "PiercingArmor": 0.0, "ColdResistance": 0.0,
    },
    {
        "RowName": "Item_Shiv",
        "ItemID": "Item_Shiv",
        "DisplayName": "Shiv",
        "Description": "A sharpened metal shard wrapped in cloth tape. Quick and quiet — the Tail's weapon of choice.",
        "Category": ITEM_CATEGORY["Weapon"],
        "Rarity": ITEM_RARITY["Common"],
        "Weight": 0.3,
        "MaxStackSize": 1,
        "Value": 8,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 0.0, "BladedArmor": 0.0, "PiercingArmor": 0.0, "ColdResistance": 0.0,
    },
    {
        "RowName": "Item_Wrench",
        "ItemID": "Item_Wrench",
        "DisplayName": "Heavy Wrench",
        "Description": "An industrial-grade pipe wrench. Useful for repairs and for cracking skulls. Dual-purpose engineering.",
        "Category": ITEM_CATEGORY["Weapon"],
        "Rarity": ITEM_RARITY["Common"],
        "Weight": 1.8,
        "MaxStackSize": 1,
        "Value": 15,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 0.0, "BladedArmor": 0.0, "PiercingArmor": 0.0, "ColdResistance": 0.0,
    },
    {
        "RowName": "Item_Machete",
        "ItemID": "Item_Machete",
        "DisplayName": "Machete",
        "Description": "Repurposed from the agricultural car. The blade is nicked but still keen. A terrifying sight in close quarters.",
        "Category": ITEM_CATEGORY["Weapon"],
        "Rarity": ITEM_RARITY["Uncommon"],
        "Weight": 1.2,
        "MaxStackSize": 1,
        "Value": 35,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 0.0, "BladedArmor": 0.0, "PiercingArmor": 0.0, "ColdResistance": 0.0,
    },
    {
        "RowName": "Item_ReinforcedBat",
        "ItemID": "Item_ReinforcedBat",
        "DisplayName": "Reinforced Bat",
        "Description": "A wooden bat wrapped in scrap metal and wire. Slow but devastating. One good swing changes the conversation.",
        "Category": ITEM_CATEGORY["Weapon"],
        "Rarity": ITEM_RARITY["Uncommon"],
        "Weight": 3.0,
        "MaxStackSize": 1,
        "Value": 30,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 0.0, "BladedArmor": 0.0, "PiercingArmor": 0.0, "ColdResistance": 0.0,
    },
    {
        "RowName": "Item_JackbootBaton",
        "ItemID": "Item_JackbootBaton",
        "DisplayName": "Jackboot Baton",
        "Description": "Standard-issue crowd-control baton carried by Wilford's enforcers. Lightweight polymer over a steel core.",
        "Category": ITEM_CATEGORY["Weapon"],
        "Rarity": ITEM_RARITY["Rare"],
        "Weight": 1.0,
        "MaxStackSize": 1,
        "Value": 60,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 0.0, "BladedArmor": 0.0, "PiercingArmor": 0.0, "ColdResistance": 0.0,
    },
    # --- Armor ---
    {
        "RowName": "Item_ScrapVest",
        "ItemID": "Item_ScrapVest",
        "DisplayName": "Scrap Vest",
        "Description": "Sheet metal riveted to a leather apron. Crude but functional — stops a shiv, mostly.",
        "Category": ITEM_CATEGORY["Armor"],
        "Rarity": ITEM_RARITY["Common"],
        "Weight": 4.0,
        "MaxStackSize": 1,
        "Value": 20,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 5.0, "BladedArmor": 10.0, "PiercingArmor": 3.0, "ColdResistance": 5.0,
    },
    {
        "RowName": "Item_TailRags",
        "ItemID": "Item_TailRags",
        "DisplayName": "Tail Rags",
        "Description": "Layered cloth and salvaged insulation. Offers almost no protection, but keeps the cold at bay.",
        "Category": ITEM_CATEGORY["Armor"],
        "Rarity": ITEM_RARITY["Common"],
        "Weight": 1.5,
        "MaxStackSize": 1,
        "Value": 5,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 2.0, "BladedArmor": 1.0, "PiercingArmor": 0.0, "ColdResistance": 15.0,
    },
    {
        "RowName": "Item_JackbootArmor",
        "ItemID": "Item_JackbootArmor",
        "DisplayName": "Jackboot Armor",
        "Description": "Composite riot armor issued to Wilford's security forces. Lightweight, durable, and deeply envied by the lower cars.",
        "Category": ITEM_CATEGORY["Armor"],
        "Rarity": ITEM_RARITY["Rare"],
        "Weight": 6.0,
        "MaxStackSize": 1,
        "Value": 120,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 15.0, "BladedArmor": 20.0, "PiercingArmor": 12.0, "ColdResistance": 10.0,
    },
    # --- Armor loot pool (paper-doll equippable: Head / Torso / Shield) ---
    {
        "RowName": "Item_Armor_TailRags", "ItemID": "Item_Armor_TailRags",
        "DisplayName": "Tail Rags", "Description": "Layered scraps of cloth and hide. Better than nothing, which is what most of the Tail has.",
        "Category": ITEM_CATEGORY["Armor"], "Rarity": ITEM_RARITY["Common"], "Weight": 1.2, "MaxStackSize": 1, "Value": 6,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 2.0, "BladedArmor": 2.0, "PiercingArmor": 1.0, "ColdResistance": 4.0,
    },
    {
        "RowName": "Item_Armor_WorkerCoat", "ItemID": "Item_Armor_WorkerCoat",
        "DisplayName": "Worker's Heavy Coat", "Description": "Thick third-class labor coat, padded against the cold of the working spine.",
        "Category": ITEM_CATEGORY["Armor"], "Rarity": ITEM_RARITY["Uncommon"], "Weight": 3.5, "MaxStackSize": 1, "Value": 35,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 5.0, "BladedArmor": 3.0, "PiercingArmor": 2.0, "ColdResistance": 12.0,
    },
    {
        "RowName": "Item_Armor_JackbootHelmet", "ItemID": "Item_Armor_JackbootHelmet",
        "DisplayName": "Jackboot Riot Helmet", "Description": "Visored composite helmet, pried off a guard who won't need it.",
        "Category": ITEM_CATEGORY["Armor"], "Rarity": ITEM_RARITY["Uncommon"], "Weight": 2.1, "MaxStackSize": 1, "Value": 60,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 12.0, "BladedArmor": 8.0, "PiercingArmor": 10.0, "ColdResistance": 2.0,
    },
    {
        "RowName": "Item_Armor_ScrapPlate", "ItemID": "Item_Armor_ScrapPlate",
        "DisplayName": "Scrap-Metal Plate Vest", "Description": "Hammered plate lashed to a harness. Heavy, loud, and it stops a blade.",
        "Category": ITEM_CATEGORY["Armor"], "Rarity": ITEM_RARITY["Rare"], "Weight": 6.8, "MaxStackSize": 1, "Value": 90,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 14.0, "BladedArmor": 6.0, "PiercingArmor": 5.0, "ColdResistance": 1.0,
    },
    {
        "RowName": "Item_Armor_EngineMonkHood", "ItemID": "Item_Armor_EngineMonkHood",
        "DisplayName": "Order Hood", "Description": "A felt hood of the Engine's faithful. Silent, warm, and it draws no second looks up-train.",
        "Category": ITEM_CATEGORY["Armor"], "Rarity": ITEM_RARITY["Uncommon"], "Weight": 0.9, "MaxStackSize": 1, "Value": 40,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 4.0, "BladedArmor": 4.0, "PiercingArmor": 4.0, "ColdResistance": 8.0,
    },
    {
        "RowName": "Item_Armor_FreezerSuit", "ItemID": "Item_Armor_FreezerSuit",
        "DisplayName": "Freezer-Breach Suit", "Description": "Insulated suit for the cold cars. Bulky, but the only thing keeping the breach from killing you.",
        "Category": ITEM_CATEGORY["Armor"], "Rarity": ITEM_RARITY["Rare"], "Weight": 4.0, "MaxStackSize": 1, "Value": 80,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 3.0, "BladedArmor": 3.0, "PiercingArmor": 3.0, "ColdResistance": 30.0,
    },
    {
        "RowName": "Item_Armor_KronoleGuard", "ItemID": "Item_Armor_KronoleGuard",
        "DisplayName": "Runner's Buckler", "Description": "A strapped scrap shield the kronole couriers swear by. Turns a knife, mostly.",
        "Category": ITEM_CATEGORY["Armor"], "Rarity": ITEM_RARITY["Uncommon"], "Weight": 1.4, "MaxStackSize": 1, "Value": 45,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 18.0, "BladedArmor": 18.0, "PiercingArmor": 18.0, "ColdResistance": 0.0,
    },
    {
        "RowName": "Item_Armor_FirstClassVisor", "ItemID": "Item_Armor_FirstClassVisor",
        "DisplayName": "First-Class Visor Mask", "Description": "Lacquered mask from the front cars. Fine work — and a fine disguise.",
        "Category": ITEM_CATEGORY["Armor"], "Rarity": ITEM_RARITY["Rare"], "Weight": 1.0, "MaxStackSize": 1, "Value": 110,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 6.0, "BladedArmor": 10.0, "PiercingArmor": 6.0, "ColdResistance": 5.0,
    },
    {
        "RowName": "Item_Armor_WilfordPlate", "ItemID": "Item_Armor_WilfordPlate",
        "DisplayName": "Engine Plate", "Description": "Ceremonial armor of the Engine's wardens. Nobody this far back should have it.",
        "Category": ITEM_CATEGORY["Armor"], "Rarity": ITEM_RARITY["Legendary"], "Weight": 8.2, "MaxStackSize": 1, "Value": 400,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 20.0, "BladedArmor": 15.0, "PiercingArmor": 18.0, "ColdResistance": 15.0,
    },
    {
        "RowName": "Item_Armor_ThawShield", "ItemID": "Item_Armor_ThawShield",
        "DisplayName": "Thaw Shield", "Description": "A heretic's shield, painted with a melting sun. Heavy enough to matter.",
        "Category": ITEM_CATEGORY["Armor"], "Rarity": ITEM_RARITY["Rare"], "Weight": 2.8, "MaxStackSize": 1, "Value": 95,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 30.0, "BladedArmor": 30.0, "PiercingArmor": 30.0, "ColdResistance": 10.0,
    },
    # --- Crafting Materials ---
    {
        "RowName": "Item_ScrapMetal",
        "ItemID": "Item_ScrapMetal",
        "DisplayName": "Scrap Metal",
        "Description": "Salvaged metal fragments. The backbone of Tail engineering — every weapon and tool started as a handful of scrap.",
        "Category": ITEM_CATEGORY["Crafting"],
        "Rarity": ITEM_RARITY["Common"],
        "Weight": 0.8,
        "MaxStackSize": 30,
        "Value": 2,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 0.0, "BladedArmor": 0.0, "PiercingArmor": 0.0, "ColdResistance": 0.0,
    },
    {
        "RowName": "Item_ClothStrips",
        "ItemID": "Item_ClothStrips",
        "DisplayName": "Cloth Strips",
        "Description": "Torn fabric from old uniforms and blankets. Used for bandages, weapon grips, and insulation.",
        "Category": ITEM_CATEGORY["Crafting"],
        "Rarity": ITEM_RARITY["Common"],
        "Weight": 0.1,
        "MaxStackSize": 30,
        "Value": 1,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 0.0, "BladedArmor": 0.0, "PiercingArmor": 0.0, "ColdResistance": 0.0,
    },
    {
        "RowName": "Item_Chemicals",
        "ItemID": "Item_Chemicals",
        "DisplayName": "Chemical Compound",
        "Description": "A sealed vial of industrial solvent. Useful in crafting, dangerous if mishandled. Handle with extreme care.",
        "Category": ITEM_CATEGORY["Crafting"],
        "Rarity": ITEM_RARITY["Uncommon"],
        "Weight": 0.3,
        "MaxStackSize": 10,
        "Value": 12,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 0.0, "BladedArmor": 0.0, "PiercingArmor": 0.0, "ColdResistance": 0.0,
    },
    {
        "RowName": "Item_Wire",
        "ItemID": "Item_Wire",
        "DisplayName": "Wire Coil",
        "Description": "Copper wiring stripped from defunct systems. Essential for electrical work, traps, and weapon reinforcement.",
        "Category": ITEM_CATEGORY["Crafting"],
        "Rarity": ITEM_RARITY["Common"],
        "Weight": 0.4,
        "MaxStackSize": 20,
        "Value": 4,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 0.0, "BladedArmor": 0.0, "PiercingArmor": 0.0, "ColdResistance": 0.0,
    },
    {
        "RowName": "Item_Tape",
        "ItemID": "Item_Tape",
        "DisplayName": "Adhesive Tape",
        "Description": "Industrial-grade tape salvaged from maintenance stores. Holds anything together — temporarily.",
        "Category": ITEM_CATEGORY["Crafting"],
        "Rarity": ITEM_RARITY["Common"],
        "Weight": 0.1,
        "MaxStackSize": 20,
        "Value": 3,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 0.0, "BladedArmor": 0.0, "PiercingArmor": 0.0, "ColdResistance": 0.0,
    },
    # --- Quest Items ---
    {
        "RowName": "Item_TinStar",
        "ItemID": "Item_TinStar",
        "DisplayName": "Tin Star",
        "Description": "A five-pointed star cut from a tin can, edges sharp enough to draw blood. Timmy made it for the nursery ceiling. It smells faintly of rust and the Tail. Tanya gave it to you — show it to him.",
        "Category": ITEM_CATEGORY["Quest"],
        "Rarity": ITEM_RARITY["Legendary"],
        "Weight": 0.05,
        "MaxStackSize": 1,
        "Value": 0,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 0.0, "BladedArmor": 0.0, "PiercingArmor": 0.0, "ColdResistance": 0.0,
    },
    {
        "RowName": "Item_GateKey",
        "ItemID": "Item_GateKey",
        "DisplayName": "Gate Key",
        "Description": "A heavy magnetic key card that unlocks the bulkhead between the Tail and Third Class. Stolen at great cost.",
        "Category": ITEM_CATEGORY["Quest"],
        "Rarity": ITEM_RARITY["Rare"],
        "Weight": 0.2,
        "MaxStackSize": 1,
        "Value": 0,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 0.0, "BladedArmor": 0.0, "PiercingArmor": 0.0, "ColdResistance": 0.0,
    },
    {
        "RowName": "Item_CommanderOrders",
        "ItemID": "Item_CommanderOrders",
        "DisplayName": "Commander's Orders",
        "Description": "Sealed documents bearing Wilford's insignia. The contents detail security rotations and supply schedules — invaluable intelligence.",
        "Category": ITEM_CATEGORY["Quest"],
        "Rarity": ITEM_RARITY["Rare"],
        "Weight": 0.1,
        "MaxStackSize": 1,
        "Value": 0,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 0.0, "BladedArmor": 0.0, "PiercingArmor": 0.0, "ColdResistance": 0.0,
    },
    {
        "RowName": "Item_RevolutionMap",
        "ItemID": "Item_RevolutionMap",
        "DisplayName": "Revolution Map",
        "Description": "A hand-drawn schematic of the train's first 20 cars, annotated with guard positions, weak points, and rally signals.",
        "Category": ITEM_CATEGORY["Quest"],
        "Rarity": ITEM_RARITY["Legendary"],
        "Weight": 0.1,
        "MaxStackSize": 1,
        "Value": 0,
        "HealthRestore": 0.0, "HungerRestore": 0.0, "StaminaRestore": 0.0,
        "BluntArmor": 0.0, "BladedArmor": 0.0, "PiercingArmor": 0.0, "ColdResistance": 0.0,
    },
]


# ----- 2. DIALOGUE  (FSEEDialogueNode, row‑name = NodeID) ----

# ESEEDialogueNodeType ordinals
NODE_TYPE = {
    "NPCLine": 0, "PlayerChoice": 1, "SkillCheck": 2,
    "Branch": 3, "SetFlag": 4, "End": 5,
}


def _dlg(
    row, node_id, node_type, speaker, text,
    choices=None, next_node="None", flag_to_set="None", flag_value=True,
    branch_flag="None", branch_true="None", branch_false="None",
    timed=0.0,
):
    """Helper to build a dialogue-node dict."""
    return {
        "RowName": row,
        "NodeID": node_id,
        "NodeType": NODE_TYPE[node_type],
        "SpeakerName": speaker,
        "DialogueText": text,
        "Choices": choices or [],
        "NextNodeID": next_node,
        "FlagToSet": flag_to_set,
        "FlagValue": flag_value,
        "BranchFlag": branch_flag,
        "BranchTrueNode": branch_true,
        "BranchFalseNode": branch_false,
        "TimedResponseDuration": timed,
    }


def _choice(text, next_node, req_flag="None", req_stat="None", req_stat_val=0):
    return {
        "ChoiceText": text,
        "NextNodeID": next_node,
        "RequiredFlag": req_flag,
        "RequiredStat": req_stat,
        "RequiredStatValue": req_stat_val,
    }


DIALOGUE_ZONE1 = [
    # =================================================================
    # OLD MAN PIKE — elder of the Tail (Gilliam role). Revolt counsel,
    # the no-bullets suspicion, the Martyrs, and a buried secret.
    # Entry node: Pike_01 (KEEP — C++ maps the NPC here).
    # Hands off to SAMUEL (hothead lieutenant) at Pike_15.
    # =================================================================
    _dlg("Pike_01", "Pike_01", "NPCLine", "Old Man Pike",
         "Sit. You weren't in the riot this morning — I checked. Eighteen years I've counted this car's faces, and tonight the count came up four short. So before anything else: what are you made of?",
         flag_to_set="Flag_MetPike", flag_value=True, next_node="Pike_02"),
    _dlg("Pike_02", "Pike_02", "PlayerChoice", "", "",
         choices=[
             _choice("What happened this morning?", "Pike_03"),
             _choice("Same as everyone. Insect paste and bad sleep.", "Pike_04"),
             _choice("[Perception 3] You've already decided about me. This is theatre.", "Pike_05", req_stat="Perception", req_stat_val=3),
         ]),
    _dlg("Pike_03", "Pike_03", "NPCLine", "Old Man Pike",
         "They cut the ration to half a block. A boy threw his at a guard's visor, and the answer was batons. It is always batons. Eighteen years, and I cannot remember the last time it was bullets.",
         next_node="Pike_06"),
    _dlg("Pike_04", "Pike_04", "NPCLine", "Old Man Pike",
         "Ha. Honest, at least. Insects and patience — that's the whole Tail. Patience is the one thing I'm short of tonight, so listen close.",
         next_node="Pike_06"),
    _dlg("Pike_05", "Pike_05", "NPCLine", "Old Man Pike",
         "Caught me. I count people the way Martha counts blocks — it's how we're both still here. You see clearly. The revolt will need eyes long before it needs fists.",
         flag_to_set="Pike_Impressed", flag_value=True, next_node="Pike_06"),
    _dlg("Pike_06", "Pike_06", "NPCLine", "Old Man Pike",
         "At last week's count, a rifle slipped a guard's shoulder and hit the floor. The muzzle swept the whole front row. Nobody flinched — not even the guards. You learn to read small things back here.",
         next_node="Pike_07"),
    _dlg("Pike_07", "Pike_07", "PlayerChoice", "", "",
         choices=[
             _choice("You think the guns are empty.", "Pike_08"),
             _choice("Small things don't stop bullets.", "Pike_09"),
             _choice("Why tell the new face?", "Pike_10"),
         ]),
    _dlg("Pike_08", "Pike_08", "NPCLine", "Old Man Pike",
         "I think the bullets ran out four years ago, the night they made the Martyrs. I think we have been kneeling to empty steel ever since. Prove it at the Blockade, and the whole Tail stands up.",
         flag_to_set="Flag_NoBulletsHint", flag_value=True, next_node="Pike_11"),
    _dlg("Pike_09", "Pike_09", "NPCLine", "Old Man Pike",
         "No. But when did you last hear a shot on this train? Not a baton on bone — a shot. Count back. I'll wait. That silence is either discipline, or it's a bluff four years long.",
         next_node="Pike_08"),
    _dlg("Pike_10", "Pike_10", "NPCLine", "Old Man Pike",
         "Because Samuel wants to rush the gate tomorrow with pipes and rage, and I have buried enough brave people to pave a car with them. The guards don't watch you yet. Be my proof first.",
         next_node="Pike_11"),
    _dlg("Pike_11", "Pike_11", "PlayerChoice", "", "",
         choices=[
             _choice("Tell me about the Martyrs.", "Pike_12"),
             _choice("What exactly do you need from me?", "Pike_14"),
             _choice("You're carrying something heavier than a theory. I can see it.", "Pike_16", req_flag="Pike_Impressed"),
             _choice("That's enough for tonight.", "Pike_End"),
         ]),
    _dlg("Pike_12", "Pike_12", "NPCLine", "Old Man Pike",
         "Four years ago, sixty-two of us rushed the gate behind Big Marta. The guards fired until the smoke hid the bodies. We scratch their names into the wall in Car 1 so the train can't eat those too.",
         flag_to_set="Flag_MartyrsTold", flag_value=True, next_node="Pike_13"),
    _dlg("Pike_13", "Pike_13", "NPCLine", "Old Man Pike",
         "And here is what nobody says aloud: after that night, the guards never fired again. Not once. Maybe sixty-two names is what the world's last bullets bought. That arithmetic is why I'm still alive.",
         next_node="Pike_11"),
    _dlg("Pike_14", "Pike_14", "NPCLine", "Old Man Pike",
         "Three things. Walk the kitchen and learn what they feed us — a revolt should know its own blood. Keep Samuel from dying early. And at the Blockade, when it comes — count the bullets.",
         next_node="Pike_15"),
    _dlg("Pike_15", "Pike_15", "NPCLine", "Old Man Pike",
         "Start tonight. The Dealer knows the man who ran the gate console — that arrangement is yours to make. I'm too old and too watched. Go on. And eat something first. Even half a block.",
         flag_to_set="Quest_LongWalk_Started", flag_value=True, next_node="Samuel_01"),
    _dlg("Pike_16", "Pike_16", "NPCLine", "Old Man Pike",
         "(He studies his hands for a long time.) There are years back here I don't speak of. The first winter. What we ate before the blocks came. Ask me when the gate is open — if you still want to know.",
         next_node="Pike_17"),
    _dlg("Pike_17", "Pike_17", "NPCLine", "Old Man Pike",
         "Whatever you hear ahead of you — whatever they say I was — remember that a man can spend eighteen years paying for six weeks. Now let an old man keep one door shut.",
         flag_to_set="Flag_PikeSecret", flag_value=True, next_node="Pike_11"),
    _dlg("Pike_End", "Pike_End", "End", "", ""),

    # =================================================================
    # SAMUEL — NEW SPEAKER. Hothead lieutenant (young-Edgar archetype).
    # Reached from Pike_15. Wants the gate tonight; can be held back.
    # =================================================================
    _dlg("Samuel_01", "Samuel_01", "NPCLine", "Samuel",
         "(A young man shoulders in, knuckles still split from the riot.) More counting, old man? While you count, they cut rations. I say we take the gate tonight — every hand, every pipe. You. Can you swing one?",
         flag_to_set="Flag_MetSamuel", flag_value=True, next_node="Samuel_02"),
    _dlg("Samuel_02", "Samuel_02", "PlayerChoice", "", "",
         choices=[
             _choice("Pike's way keeps people breathing.", "Samuel_03"),
             _choice("Tonight is too soon. But soon.", "Samuel_04"),
             _choice("If the rifles are empty, we don't need every hand. We need proof.", "Samuel_05", req_flag="Flag_NoBulletsHint"),
             _choice("[Strength 4] Swing one? Kid, I'd be carrying you home.", "Samuel_06", req_stat="Strength", req_stat_val=4),
         ]),
    _dlg("Samuel_03", "Samuel_03", "NPCLine", "Samuel",
         "Breathing. Half a block a day isn't breathing, it's a slow drawer. My brother went over the heads in the last revolt and the wall got his name. Patience is just a grave with extra steps.",
         next_node="Samuel_07"),
    _dlg("Samuel_04", "Samuel_04", "NPCLine", "Samuel",
         "Soon. That word has eaten more Tailies than the cold has. Fine. I'll lend you my 'soon' — once. Spend it where I can see it.",
         next_node="Samuel_07"),
    _dlg("Samuel_05", "Samuel_05", "NPCLine", "Samuel",
         "...That's the first new idea anyone's had in four years. Get your proof at the Blockade and I'll hold the hotheads back till you do. My word on it — and back here a word is the only thing that doesn't run out.",
         flag_to_set="Flag_SamuelHeld", flag_value=True, next_node="Samuel_07"),
    _dlg("Samuel_06", "Samuel_06", "NPCLine", "Samuel",
         "(A grin cracks through the scowl.) Maybe you would. Fine, heavy hands — when the doors open, you're in the front row next to me. Try not to die before it matters.",
         next_node="Samuel_07"),
    _dlg("Samuel_07", "Samuel_07", "NPCLine", "Samuel",
         "One more thing. The Dealer in the back bunks runs errands for the gate man — kronole goes forward, favours come back. If you ever need that door to blink, start there. And hold your nose.",
         next_node="Samuel_End"),
    _dlg("Samuel_End", "Samuel_End", "End", "", ""),

    # =================================================================
    # KRONOLE DEALER — sells numbness, knows it burns, fears the sweeps.
    # Entry node: Dealer_01 (KEEP). Gate-specialist (Yastrek) connection.
    # =================================================================
    _dlg("Dealer_01", "Dealer_01", "NPCLine", "Kronole Dealer",
         "Easy, friend, easy. I sell the only sunset left on this train. One pinch of kronole and you can't feel the cold, the count, or the year. First taste is nearly free.",
         flag_to_set="Flag_MetDealer", flag_value=True, next_node="Dealer_02"),
    _dlg("Dealer_02", "Dealer_02", "PlayerChoice", "", "",
         choices=[
             _choice("What is it, really?", "Dealer_03"),
             _choice("What does a hit cost?", "Dealer_04"),
             _choice("I need a door opened, not a sunset.", "Dealer_07"),
             _choice("Not interested.", "Dealer_10"),
         ]),
    _dlg("Dealer_03", "Dealer_03", "NPCLine", "Kronole Dealer",
         "Industrial waste, refined with love. The train makes it; I only move it. It eats your teeth and your years and people thank me anyway. That tells you about the train, friend. Not about me.",
         next_node="Dealer_05"),
    _dlg("Dealer_04", "Dealer_04", "NPCLine", "Kronole Dealer",
         "Two protein blocks a pinch. One, if you run an errand. Grief is the only thing back here that comes free.",
         next_node="Dealer_05"),
    _dlg("Dealer_05", "Dealer_05", "PlayerChoice", "", "",
         choices=[
             _choice("[Cunning 3] I saw a spilled pinch scorch a steel pipe. It burns, doesn't it.", "Dealer_06", req_stat="Cunning", req_stat_val=3),
             _choice("Who buys the most?", "Dealer_08"),
             _choice("Maybe later.", "Dealer_11"),
         ]),
    _dlg("Dealer_06", "Dealer_06", "NPCLine", "Kronole Dealer",
         "Voice down. Yes. Dried and packed tight, one spark takes a door off its hinge. Why do you think the Jackboots sweep for it? They are not worried about our teeth, friend.",
         flag_to_set="Flag_KronoleBurns", flag_value=True, next_node="Dealer_09"),
    _dlg("Dealer_07", "Dealer_07", "NPCLine", "Kronole Dealer",
         "Ah. Then you want Yastrek — ran the Tail gate console till he owed the Network more than he's worth. They penned him behind the kitchen after the riot. Spring him, and his debt becomes your favour.",
         flag_to_set="Dealer_Persuaded", flag_value=True, next_node="Dealer_09"),
    _dlg("Dealer_08", "Dealer_08", "NPCLine", "Kronole Dealer",
         "Mothers, mostly. The ones whose little ones went forward. Kronole is the only thing that makes the bunk across from you stop being empty. Judge me after you've watched that for eighteen years.",
         next_node="Dealer_09"),
    _dlg("Dealer_09", "Dealer_09", "NPCLine", "Kronole Dealer",
         "Free advice, one time only. If Pike's war kicks off, the Jackboots sweep these bunks down to the bolts — my stock, my buyers, gone. So whatever you people are planning... buy first.",
         next_node="Dealer_11"),
    _dlg("Dealer_10", "Dealer_10", "NPCLine", "Kronole Dealer",
         "Today, maybe. But the Tail always comes back to me. The cold does my advertising.",
         next_node="Dealer_11"),
    _dlg("Dealer_11", "Dealer_11", "PlayerChoice", "", "",
         choices=[
             _choice("The woman with the broken ribs — Mara. She needs something for the pain.", "Dealer_12"),
             _choice("Tell me about the gate man.", "Dealer_07"),
             _choice("We're done.", "Dealer_End"),
         ]),
    _dlg("Dealer_12", "Dealer_12", "NPCLine", "Kronole Dealer",
         "...One dose. On the house, and you saw nothing — mercy is terrible for my margins. Tell Mara to take half. Half, you hear? I'd rather keep a customer than bury one.",
         flag_to_set="Flag_DealerMercy", flag_value=True, next_node="Dealer_13"),
    _dlg("Dealer_13", "Dealer_13", "NPCLine", "Kronole Dealer",
         "Now get gone. And friend — when the sweeps come, remember which poison kept this car quiet enough to survive eighteen years. It wasn't hope.",
         next_node="Dealer_End"),
    _dlg("Dealer_End", "Dealer_End", "End", "", ""),

    # =================================================================
    # JACKBOOT GUARD — Mason-voice: order rhetoric, condescension,
    # one crack in the facade behind a high Perception check.
    # Entry node: Guard_01 (KEEP).
    # =================================================================
    _dlg("Guard_01", "Guard_01", "NPCLine", "Jackboot Guard",
         "Behind the line, Tailie. See the yellow paint? The paint is the law, the law is the train, and the train is the reason you are breathing. Show some gratitude and step back.",
         flag_to_set="Flag_MetGuard", flag_value=True, next_node="Guard_02"),
    _dlg("Guard_02", "Guard_02", "PlayerChoice", "", "",
         choices=[
             _choice("We're starving. Half a block a day.", "Guard_03"),
             _choice("Open the gate. I have business forward.", "Guard_04"),
             _choice("You were at the gate four years ago, weren't you.", "Guard_07", req_flag="Flag_MartyrsTold"),
             _choice("Forget it.", "Guard_End"),
         ]),
    _dlg("Guard_03", "Guard_03", "NPCLine", "Jackboot Guard",
         "The Engine feeds you on schedule. A foot does not complain that it is not the mouth — every part of this train gets its portion and its place. Yours is behind the paint. Eternal order, eternal engine.",
         next_node="Guard_05"),
    _dlg("Guard_04", "Guard_04", "NPCLine", "Jackboot Guard",
         "Business. *a dry laugh inside the helmet* The Tail has no business, it has allotments. Forward is for ticketed souls. You were freight that learned to talk. Step. Back.",
         next_node="Guard_05"),
    _dlg("Guard_05", "Guard_05", "PlayerChoice", "", "",
         choices=[
             _choice("[Strength 5] Take the helmet off and say 'freight' again.", "Guard_06", req_stat="Strength", req_stat_val=5),
             _choice("[Cunning 3] Allotments. Like the kronole your sergeant allots himself? (Offer bribe)", "Guard_08", req_stat="Cunning", req_stat_val=3),
             _choice("When that gate opens, remember which side of the paint you stood on.", "Guard_09"),
             _choice("[Perception 5] Your magazine's seated crooked and you don't care. When was that rifle last loaded?", "Guard_10", req_stat="Perception", req_stat_val=5),
         ]),
    _dlg("Guard_06", "Guard_06", "NPCLine", "Jackboot Guard",
         "*his hand drops to the baton — and stops* Six men behind this door, Tailie. Six. Keep talking and you'll be a stain on the yellow. *but he has taken one step back, and you both know it*",
         flag_to_set="Guard_Intimidated", flag_value=True, next_node="Guard_11"),
    _dlg("Guard_07", "Guard_07", "NPCLine", "Jackboot Guard",
         "...You don't know what you're asking. The gate held. That's all. The gate held and the train went on, the way it always does. *the slogans are gone from his voice* Step back now.",
         next_node="Guard_11"),
    _dlg("Guard_08", "Guard_08", "NPCLine", "Jackboot Guard",
         "*he checks the camera, then your hand* Shift change is four minutes. What I don't see in four minutes never happened. The Engine forgives what it does not witness. Move.",
         flag_to_set="Guard_Bribed", flag_value=True, next_node="Guard_End"),
    _dlg("Guard_09", "Guard_09", "NPCLine", "Jackboot Guard",
         "It opened once. We mopped for a week. *almost gentle* The train is a body, Tailie. When the foot forgets what it is, the body cuts it off and rides on lighter. Don't be the foot.",
         next_node="Guard_11"),
    _dlg("Guard_10", "Guard_10", "NPCLine", "Jackboot Guard",
         "*half a second too long* Loaded enough for you. *but his thumb moves to check the strap, and his eyes go somewhere else entirely* Inspections are not your job. Behind. The. Line.",
         flag_to_set="Flag_GuardDoubts", flag_value=True, next_node="Guard_11"),
    _dlg("Guard_11", "Guard_11", "PlayerChoice", "", "",
         choices=[
             _choice("There's nothing in that rifle. We both just learned it.", "Guard_12", req_flag="Flag_GuardDoubts"),
             _choice("One day the paint won't save you.", "Guard_13"),
             _choice("Enjoy the yellow line.", "Guard_End"),
         ]),
    _dlg("Guard_12", "Guard_12", "NPCLine", "Jackboot Guard",
         "*quiet, no slogans left* I haven't drawn ammunition in four years. Nobody on this gate has. Get behind the line before someone braver than me hears you. *louder* BEHIND THE LINE, TAILIE.",
         flag_to_set="Flag_NoBulletsConfirmed", flag_value=True, next_node="Guard_End"),
    _dlg("Guard_13", "Guard_13", "NPCLine", "Jackboot Guard",
         "The paint has kept me alive for eighteen years, and it will keep me alive tomorrow. That's more than your side of it can claim. *taps the baton on the line* Eternal is a long time. Get comfortable.",
         next_node="Guard_End"),
    _dlg("Guard_End", "Guard_End", "End", "", ""),

    # =================================================================
    # JOSIE — Workshop Mechanic. Revolt logistics, water-ration anger,
    # weapons from scrap, kronole breaching charge (gated callback).
    # Entry node: Mechanic_01 (KEEP).
    # =================================================================
    _dlg("Mechanic_01", "Mechanic_01", "NPCLine", "Josie",
         "Mind the bench — three square metres holding most of the Tail's hope. I'm Josie. I turn wall panels into spears and bedframes into shields. You here to work, or to watch?",
         flag_to_set="Flag_MetJosie", flag_value=True, next_node="Mechanic_02"),
    _dlg("Mechanic_02", "Mechanic_02", "PlayerChoice", "", "",
         choices=[
             _choice("Work. Teach me to build weapons.", "Mechanic_03"),
             _choice("What's got you hammering this hard?", "Mechanic_04"),
             _choice("Pike thinks the rifles are empty. If he's right, your steel decides this.", "Mechanic_05", req_flag="Flag_NoBulletsHint"),
         ]),
    _dlg("Mechanic_03", "Mechanic_03", "NPCLine", "Josie",
         "Right answer. Bring me 3 Scrap Metal and 2 Cloth Strips and we'll start with a reinforced shiv. After that — spearheads, shield rims, gate hooks. Garbage in, revolution out.",
         flag_to_set="Quest_Workshop_Started", flag_value=True, next_node="Mechanic_06"),
    _dlg("Mechanic_04", "Mechanic_04", "NPCLine", "Josie",
         "They cut the water ration. Again. Half a cup a day to drink, wash, and boil bandages — my nursery kids are drinking rust. So I hammer. It's this or start screaming and never stop.",
         next_node="Mechanic_06"),
    _dlg("Mechanic_05", "Mechanic_05", "NPCLine", "Josie",
         "Then every blade on this bench just doubled in value — steel doesn't run out of bullets. Tell Pike I can arm forty hands if the scrap keeps coming. And that I'm holding him to that 'if.'",
         flag_to_set="Flag_JosieArming", flag_value=True, next_node="Mechanic_06"),
    _dlg("Mechanic_06", "Mechanic_06", "NPCLine", "Josie",
         "Bench rules. Nothing leaves before the signal. Nothing gets sold to the Network. And nothing — ever — gets used on a Tailie. Break those, and I will unmake you like a bad weld.",
         next_node="Mechanic_07"),
    _dlg("Mechanic_07", "Mechanic_07", "PlayerChoice", "", "",
         choices=[
             _choice("What do you need most?", "Mechanic_08"),
             _choice("Kronole burns hot. Could you build a charge that opens the gate?", "Mechanic_09", req_flag="Flag_KronoleBurns"),
             _choice("Why the bench and not the front line?", "Mechanic_10"),
             _choice("I'll get to work.", "Mechanic_11"),
         ]),
    _dlg("Mechanic_08", "Mechanic_08", "NPCLine", "Josie",
         "Scrap, wire, and time. The first two you can carry — strip the dead bunks in Car 3, gently, people slept their whole lives in those frames. The third, the train keeps for itself.",
         next_node="Mechanic_07"),
    _dlg("Mechanic_09", "Mechanic_09", "NPCLine", "Josie",
         "...So you found out what it really is. Dried, packed in pipe, wired to a spark — it would open that gate like a ration tin. I hate that I know that. Bring me the bricks and keep them dry.",
         flag_to_set="Flag_ChargePlanned", flag_value=True, next_node="Mechanic_07"),
    _dlg("Mechanic_10", "Mechanic_10", "NPCLine", "Josie",
         "A fighter stops one Jackboot. This bench arms forty. And I promised the nursery kids I'd come back every night — I keep the small promises. It's how I practise for the big one.",
         next_node="Mechanic_07"),
    _dlg("Mechanic_11", "Mechanic_11", "NPCLine", "Josie",
         "Eyes up, hands steady. And when it starts — stay behind the shields until the spears have done their work. I build these so people come back. Make my work mean something.",
         next_node="Mechanic_End"),
    _dlg("Mechanic_End", "Mechanic_End", "End", "", ""),

    # =================================================================
    # MARA — Injured Tailie. Human cost of the last revolt; despair vs.
    # hope branch; kronole mercy callback. Entry node: Injured_01 (KEEP).
    # Hands off to TANYA (grieving mother) at Injured_13.
    # =================================================================
    _dlg("Injured_01", "Injured_01", "NPCLine", "Injured Tailie",
         "*coughs* Mind the floor — that's mine, I think. A Jackboot put his boot through my ribs at the count, three days back. Something whistles when I breathe now. Funny old train.",
         flag_to_set="Flag_MetInjured", flag_value=True, next_node="Injured_02"),
    _dlg("Injured_02", "Injured_02", "PlayerChoice", "", "",
         choices=[
             _choice("Let me see the wound.", "Injured_03"),
             _choice("Why did they beat you?", "Injured_04"),
             _choice("Rest. I'll come back.", "Injured_End"),
         ]),
    _dlg("Injured_03", "Injured_03", "NPCLine", "Injured Tailie",
         "*she lifts her shirt hem — the skin is the wrong colour* It's going bad. Asha says antiseptic, but the stores are forward, on the guards' shelf in Car 8. You see the problem. The whole Tail IS the problem.",
         flag_to_set="Quest_MedicineRun_Started", flag_value=True, next_node="Injured_05"),
    _dlg("Injured_04", "Injured_04", "NPCLine", "Injured Tailie",
         "I stood between a baton and the boy who threw his ration. *laughs, winces* Worth it. In the last revolt I held a sharpened bed slat at the gate. We get older. The batons don't.",
         next_node="Injured_05"),
    _dlg("Injured_05", "Injured_05", "PlayerChoice", "", "",
         choices=[
             _choice("Tell me about the last revolt.", "Injured_06"),
             _choice("Do you still think we can win?", "Injured_07"),
             _choice("The Dealer sent a dose. For the pain — if you want it.", "Injured_09", req_flag="Flag_DealerMercy"),
             _choice("Hold on. I'll find that medicine.", "Injured_11"),
         ]),
    _dlg("Injured_06", "Injured_06", "NPCLine", "Injured Tailie",
         "Four years ago we made it six cars on rage and bed slats. Then the gate, the guns, the smoke. I crawled back under the bodies of better people. *quiet* Sixty-two names. I can still say them in order.",
         flag_to_set="Flag_HeardLastRevolt", flag_value=True, next_node="Injured_05"),
    _dlg("Injured_07", "Injured_07", "NPCLine", "Injured Tailie",
         "Win? *a long breath* Ask me at dawn and I'll say yes. Ask me at night, when the rib whistles and the bunk across the way is empty — and I'll tell you to buy the kronole and take the quiet.",
         next_node="Injured_08"),
    _dlg("Injured_08", "Injured_08", "PlayerChoice", "", "",
         choices=[
             _choice("Then I'm asking at dawn. We fight.", "Injured_10"),
             _choice("Maybe the quiet is all we're owed.", "Injured_12"),
         ]),
    _dlg("Injured_09", "Injured_09", "NPCLine", "Injured Tailie",
         "*she stares at the fold of paper a long time* ...Half now. Half for when it gets bad. I watched kronole hollow out my brother, so I know exactly what I'm thanking you for. Thank you anyway.",
         flag_to_set="Flag_GaveKronole", flag_value=True, next_node="Injured_05"),
    _dlg("Injured_10", "Injured_10", "NPCLine", "Injured Tailie",
         "*she grips your wrist, surprisingly strong* Dawn, then. When you go forward, I want to be on your list of reasons. Mara. Don't put me on the wall — put me on the list.",
         flag_to_set="Flag_InjuredHope", flag_value=True, next_node="Injured_13"),
    _dlg("Injured_11", "Injured_11", "NPCLine", "Injured Tailie",
         "Car 8, the old sickbay shelf — they keep what they took from us. *coughs* And listen. Three bunks down there's a woman called Tanya. Her boy was taken forward last month. Her hurt is worse than mine.",
         next_node="Injured_13"),
    _dlg("Injured_12", "Injured_12", "NPCLine", "Injured Tailie",
         "*she looks away* Then sit a while and say nothing. That's its own medicine back here. *coughs* You'll fight anyway. Your kind always does. Just come and say goodbye first.",
         next_node="Injured_13"),
    _dlg("Injured_13", "Injured_13", "PlayerChoice", "", "",
         choices=[
             _choice("The woman three bunks down — the one who never sleeps. Who is she?", "Tanya_01"),
             _choice("Rest now.", "Injured_End"),
         ]),
    _dlg("Injured_End", "Injured_End", "End", "", ""),

    # =================================================================
    # TANYA — NEW SPEAKER. Grieving mother; her son Timmy was taken
    # forward. Foreshadow only — the answer waits in later zones.
    # Reached from Injured_13.
    # =================================================================
    _dlg("Tanya_01", "Tanya_01", "NPCLine", "Tanya",
         "*she doesn't look up from a folded square of cloth* Mara sent you. Tell her I ate. Tell her whatever makes her worry about her own ribs instead of me.",
         flag_to_set="Flag_MetTanya", flag_value=True, next_node="Tanya_02"),
    _dlg("Tanya_02", "Tanya_02", "PlayerChoice", "", "",
         choices=[
             _choice("What's the cloth?", "Tanya_03"),
             _choice("Mara said your son was taken forward.", "Tanya_04"),
             _choice("I'll leave you be.", "Tanya_End"),
         ]),
    _dlg("Tanya_03", "Tanya_03", "NPCLine", "Tanya",
         "Timmy's shirt. He's five. They measured his arms — his arms — and the woman in yellow said 'selected,' like a prize. He waved at me going through the gate. He thought he'd won something.",
         next_node="Tanya_05"),
    _dlg("Tanya_04", "Tanya_04", "NPCLine", "Tanya",
         "Taken. *she finally looks up* A month ago. Eleven little ones in two years, all measured first, all marched forward smiling. Small ones. It's always the small ones.",
         next_node="Tanya_05"),
    _dlg("Tanya_05", "Tanya_05", "PlayerChoice", "", "",
         choices=[
             _choice("Where do they take them?", "Tanya_06"),
             _choice("Why the small ones?", "Tanya_07"),
             _choice("[Charisma 4] Look at me. I'm going forward. I will find where the children go.", "Tanya_09", req_stat="Charisma", req_stat_val=4),
         ]),
    _dlg("Tanya_06", "Tanya_06", "NPCLine", "Tanya",
         "Forward. That's the whole answer anyone has. 'Apprenticeships,' says the woman in yellow. But no letter has ever come back. No child has ever come back. The train swallows them and keeps running.",
         next_node="Tanya_08"),
    _dlg("Tanya_07", "Tanya_07", "NPCLine", "Tanya",
         "*her voice drops* I asked that too. All under five. All measured — arms, shoulders, hands. Whatever the front wants them for, it wants them small. I stopped guessing. The guesses are worse.",
         next_node="Tanya_08"),
    _dlg("Tanya_08", "Tanya_08", "PlayerChoice", "", "",
         choices=[
             _choice("I'm going forward. I'll look for him.", "Tanya_09"),
             _choice("I'm sorry, Tanya.", "Tanya_10"),
         ]),
    _dlg("Tanya_09", "Tanya_09", "NPCLine", "Tanya",
         "*she grabs your sleeve* Timmy. Brown eyes, a chin scar from the bunk ladder. If you find where they go — even if it's bad — you come back and tell me. Not knowing is the thing that's killing me.",
         flag_to_set="Flag_TanyaPromise", flag_value=True, next_node="Tanya_11"),
    _dlg("Tanya_10", "Tanya_10", "NPCLine", "Tanya",
         "Everyone's sorry. Sorry is the Tail's other ration. *she folds the shirt smaller* If you ever do go forward... look at the small ones' faces for me. Somebody has to.",
         next_node="Tanya_End"),
    _dlg("Tanya_11", "Tanya_11", "NPCLine", "Tanya",
         "Take this. *a tin star, cut from a can, sharp at the edges* He made it for the nursery ceiling. Show him, so he knows you came from me. And don't make a mother a promise you don't mean to keep.",
         flag_to_set="Reward_TinStar", flag_value=True, next_node="Tanya_End"),
    # NOTE — DIALOGUE-DIRECT ITEM GRANT: Tanya_11 sets "Reward_TinStar".
    # Wire a one-shot C++ / Blueprint OnDialogueFlagSet handler that calls
    # InventoryComponent->AddItem("Item_TinStar", 1) when this flag is first
    # set. Item_TinStar must be added to DT_Items (Quest category, Legendary,
    # "A five-pointed star cut from a tin can, sharp at the edges. Timmy made
    # it for the nursery ceiling. It smells faintly of rust and the Tail.").
    # Guard against re-granting: check flag before AddItem or use a one-shot
    # flag clear after the grant.
    _dlg("Tanya_End", "Tanya_End", "End", "", ""),

    # =================================================================
    # GENERIC TAILIE POOL — 10 short trees, 3-5 nodes each, 1-2 branches.
    # Entry node IDs must match GENERIC_POOL_NODES in SEENPCBrainSubsystem.cpp.
    # C++ routes unnamed civilians here via GetTypeHash(Label) % 10.
    # ALL existing named trees above are UNCHANGED.
    # =================================================================

    # --- POOL 0: BUNKMATE (rumour trader) ---
    # Entry: Gen_Bunkmate_01
    _dlg("Gen_Bunkmate_01", "Gen_Bunkmate_01", "NPCLine", "Bunkmate",
         "Keep your voice down. I swap rumours for protein blocks. Heard something last night that's worth at least half.",
         flag_to_set="Flag_MetBunkmate", flag_value=True, next_node="Gen_Bunkmate_02"),
    _dlg("Gen_Bunkmate_02", "Gen_Bunkmate_02", "PlayerChoice", "", "",
         choices=[
             _choice("What did you hear?", "Gen_Bunkmate_03"),
             _choice("I don't trade in rumours.", "Gen_Bunkmate_05"),
         ]),
    _dlg("Gen_Bunkmate_03", "Gen_Bunkmate_03", "NPCLine", "Bunkmate",
         "Guard rotation in Car 3 changed last night. Different face on the door. Either a promotion or they're hiding something. Might be a window if you need one.",
         flag_to_set="Flag_Bunkmate_Rumour", flag_value=True, next_node="Gen_Bunkmate_04"),
    _dlg("Gen_Bunkmate_04", "Gen_Bunkmate_04", "NPCLine", "Bunkmate",
         "Half block. Fair price for a gap in the schedule, yes? Information keeps you alive back here — almost as well as food.",
         next_node="Gen_Bunkmate_End"),
    _dlg("Gen_Bunkmate_05", "Gen_Bunkmate_05", "NPCLine", "Bunkmate",
         "Your loss. Rumours and rats — the only things that breed freely in the Tail. I'll find another buyer.",
         next_node="Gen_Bunkmate_End"),
    _dlg("Gen_Bunkmate_End", "Gen_Bunkmate_End", "End", "", ""),

    # --- POOL 1: STARVING FATHER ---
    # Entry: Gen_Father_01
    _dlg("Gen_Father_01", "Gen_Father_01", "NPCLine", "Starving Father",
         "My boy ate his whole block in one bite. Twelve years old and you can count his ribs. I used to give him half of mine but I can't stand anymore if I do.",
         flag_to_set="Flag_MetFather", flag_value=True, next_node="Gen_Father_02"),
    _dlg("Gen_Father_02", "Gen_Father_02", "PlayerChoice", "", "",
         choices=[
             _choice("How old is your son?", "Gen_Father_03"),
             _choice("The revolt will fix the ration.", "Gen_Father_04"),
         ]),
    _dlg("Gen_Father_03", "Gen_Father_03", "NPCLine", "Starving Father",
         "Twelve. Born on the train — never knew grass, never knew a full belly. He thinks this is just how the world is. I don't have the heart to tell him it wasn't always.",
         next_node="Gen_Father_End"),
    _dlg("Gen_Father_04", "Gen_Father_04", "NPCLine", "Starving Father",
         "I'll believe that when he eats a whole block AND the one after. Until then, wake me when the gate opens — I'll be right behind you.",
         next_node="Gen_Father_End"),
    _dlg("Gen_Father_End", "Gen_Father_End", "End", "", ""),

    # --- POOL 2: CARD PLAYER ---
    # Entry: Gen_CardPlayer_01
    _dlg("Gen_CardPlayer_01", "Gen_CardPlayer_01", "NPCLine", "Card Player",
         "One hand before you go. Cards made from meal tickets — it's the closest thing to money we've got back here.",
         flag_to_set="Flag_MetCardPlayer", flag_value=True, next_node="Gen_CardPlayer_02"),
    _dlg("Gen_CardPlayer_02", "Gen_CardPlayer_02", "PlayerChoice", "", "",
         choices=[
             _choice("What are you betting?", "Gen_CardPlayer_03"),
             _choice("No time for cards.", "Gen_CardPlayer_05"),
         ]),
    _dlg("Gen_CardPlayer_03", "Gen_CardPlayer_03", "NPCLine", "Card Player",
         "Information, mostly. A lost card might mean you owe someone a chore. A won hand might mean you learn which bunk holds what. The whole economy of the Tail runs on this table.",
         next_node="Gen_CardPlayer_04"),
    _dlg("Gen_CardPlayer_04", "Gen_CardPlayer_04", "NPCLine", "Card Player",
         "Pike doesn't approve, of course. Says it creates resentment. I say it creates the only entertainment left. We can't all sit and count bullets.",
         next_node="Gen_CardPlayer_End"),
    _dlg("Gen_CardPlayer_05", "Gen_CardPlayer_05", "NPCLine", "Card Player",
         "Always time for cards. You just have to decide if you'd rather think or sit still. Back here, the cards are how people think.",
         next_node="Gen_CardPlayer_End"),
    _dlg("Gen_CardPlayer_End", "Gen_CardPlayer_End", "End", "", ""),

    # --- POOL 3: OLD WOMAN (remembers boarding day) ---
    # Entry: Gen_OldWoman_01
    _dlg("Gen_OldWoman_01", "Gen_OldWoman_01", "NPCLine", "Old Woman",
         "I boarded at Rotterdam. Three hours before they sealed the doors. I remember sunshine on the platform and thinking — just a moment, just a moment of ordinary life.",
         flag_to_set="Flag_MetOldWoman", flag_value=True, next_node="Gen_OldWoman_02"),
    _dlg("Gen_OldWoman_02", "Gen_OldWoman_02", "PlayerChoice", "", "",
         choices=[
             _choice("What was it like, boarding day?", "Gen_OldWoman_03"),
             _choice("Do you regret getting on?", "Gen_OldWoman_04"),
         ]),
    _dlg("Gen_OldWoman_03", "Gen_OldWoman_03", "NPCLine", "Old Woman",
         "Chaos. Families separated at the gangway. A man shot for pushing. The Wilford men with lists, checking names. Those of us near the back — the Tail — were waved on without a check. That should have told us something.",
         next_node="Gen_OldWoman_End"),
    _dlg("Gen_OldWoman_04", "Gen_OldWoman_04", "NPCLine", "Old Woman",
         "Every day for eighteen years. And then I remember the cold outside and I stop regretting. This train is a prison but the world outside is a grave. At least in prison you can still talk.",
         next_node="Gen_OldWoman_End"),
    _dlg("Gen_OldWoman_End", "Gen_OldWoman_End", "End", "", ""),

    # --- POOL 4: KID RUNNER ---
    # Entry: Gen_KidRunner_01
    _dlg("Gen_KidRunner_01", "Gen_KidRunner_01", "NPCLine", "Kid Runner",
         "I run messages for Pike. Three blocks an hour, both ways, no stopping. I know every bolt and blind corner in the Tail better than the guards do.",
         flag_to_set="Flag_MetKidRunner", flag_value=True, next_node="Gen_KidRunner_02"),
    _dlg("Gen_KidRunner_02", "Gen_KidRunner_02", "PlayerChoice", "", "",
         choices=[
             _choice("That sounds dangerous.", "Gen_KidRunner_03"),
             _choice("Can you pass a message for me?", "Gen_KidRunner_04"),
         ]),
    _dlg("Gen_KidRunner_03", "Gen_KidRunner_03", "NPCLine", "Kid Runner",
         "The guards grab slower kids. I'm not slow. Besides — if you're twelve in the Tail you're either useful or you're scared. I decided to be useful.",
         next_node="Gen_KidRunner_End"),
    _dlg("Gen_KidRunner_04", "Gen_KidRunner_04", "NPCLine", "Kid Runner",
         "Two protein blocks upfront. I don't do favours — Pike says a runner who does favours becomes a leaky pipe. The message gets there, or I eat the blocks and the message never existed.",
         next_node="Gen_KidRunner_End"),
    _dlg("Gen_KidRunner_End", "Gen_KidRunner_End", "End", "", ""),

    # --- POOL 5: CYNIC ---
    # Entry: Gen_Cynic_01
    _dlg("Gen_Cynic_01", "Gen_Cynic_01", "NPCLine", "Cynic",
         "Don't give me that look. I've seen three revolts fail and I'll probably see a fourth. The Tail eats hope for breakfast and spits out the pits.",
         flag_to_set="Flag_MetCynic", flag_value=True, next_node="Gen_Cynic_02"),
    _dlg("Gen_Cynic_02", "Gen_Cynic_02", "PlayerChoice", "", "",
         choices=[
             _choice("This time is different. The rifles may be empty.", "Gen_Cynic_03"),
             _choice("Then why are you still here?", "Gen_Cynic_04"),
         ]),
    _dlg("Gen_Cynic_03", "Gen_Cynic_03", "NPCLine", "Cynic",
         "That's what they said about Marta's run. *Empty rifles, empty promises.* Fine. Prove it. I'll be the last one through the gate — right behind the first wave of bodies.",
         next_node="Gen_Cynic_End"),
    _dlg("Gen_Cynic_04", "Gen_Cynic_04", "NPCLine", "Cynic",
         "Where would I go? I was born in Car 1. This bunk is the only address I've ever had. I'll die here too, most likely. But I'll die calling it what it is.",
         next_node="Gen_Cynic_End"),
    _dlg("Gen_Cynic_End", "Gen_Cynic_End", "End", "", ""),

    # --- POOL 6: BELIEVER IN WILFORD ---
    # Entry: Gen_Believer_01
    _dlg("Gen_Believer_01", "Gen_Believer_01", "NPCLine", "Believer",
         "The Engine provides. I know how that sounds back here. But we're alive. We're still moving. Eighteen years and the train hasn't stopped once. That's Wilford.",
         flag_to_set="Flag_MetBeliever", flag_value=True, next_node="Gen_Believer_02"),
    _dlg("Gen_Believer_02", "Gen_Believer_02", "PlayerChoice", "", "",
         choices=[
             _choice("Wilford put us in the Tail.", "Gen_Believer_03"),
             _choice("You really believe that?", "Gen_Believer_04"),
         ]),
    _dlg("Gen_Believer_03", "Gen_Believer_03", "NPCLine", "Believer",
         "Wilford saved us. The Tail is the price of that salvation. Every engine needs a stoker — the front cars can't keep moving without the weight at the back. That's balance. That's the train.",
         next_node="Gen_Believer_End"),
    _dlg("Gen_Believer_04", "Gen_Believer_04", "NPCLine", "Believer",
         "I have to. My wife is in the drawers. My daughter is forward somewhere — selected three years ago. If Wilford is a lie, then everything I've endured is for nothing. I can't afford for it to be a lie.",
         next_node="Gen_Believer_End"),
    _dlg("Gen_Believer_End", "Gen_Believer_End", "End", "", ""),

    # --- POOL 7: GRIEVING WIDOW ---
    # Entry: Gen_Widow_01
    _dlg("Gen_Widow_01", "Gen_Widow_01", "NPCLine", "Grieving Widow",
         "He went up in the last revolt. I used to hate the memory — the waste of it. Now I think: at least he chose. Most of us just endure.",
         flag_to_set="Flag_MetWidow", flag_value=True, next_node="Gen_Widow_02"),
    _dlg("Gen_Widow_02", "Gen_Widow_02", "PlayerChoice", "", "",
         choices=[
             _choice("What was his name?", "Gen_Widow_03"),
             _choice("This revolt won't waste anyone.", "Gen_Widow_04"),
         ]),
    _dlg("Gen_Widow_03", "Gen_Widow_03", "NPCLine", "Grieving Widow",
         "Piotr. He carried a section of bed frame as a shield. The Jackboots thought it was funny until it wasn't. His name is on the wall. I add a mark below it every year we keep going without him.",
         next_node="Gen_Widow_End"),
    _dlg("Gen_Widow_04", "Gen_Widow_04", "NPCLine", "Grieving Widow",
         "That's what he said. *No waste this time.* You can't promise that. But try anyway. The alternative is doing nothing, and I've had eighteen years of that already.",
         next_node="Gen_Widow_End"),
    _dlg("Gen_Widow_End", "Gen_Widow_End", "End", "", ""),

    # --- POOL 8: BRAWLER (spoiling for revolt) ---
    # Entry: Gen_Brawler_01
    _dlg("Gen_Brawler_01", "Gen_Brawler_01", "NPCLine", "Brawler",
         "Pike keeps saying wait. Samuel keeps saying soon. Meanwhile my knuckles heal between beatings and I still don't have breakfast. I'm done waiting.",
         flag_to_set="Flag_MetBrawler", flag_value=True, next_node="Gen_Brawler_02"),
    _dlg("Gen_Brawler_02", "Gen_Brawler_02", "PlayerChoice", "", "",
         choices=[
             _choice("Charging blind gets people killed.", "Gen_Brawler_03"),
             _choice("Save that anger for the gate.", "Gen_Brawler_04"),
         ]),
    _dlg("Gen_Brawler_03", "Gen_Brawler_03", "NPCLine", "Brawler",
         "Starving slow also gets people killed — just quieter, so nobody writes a song about it. I'll take a fast death that means something over a slow one that doesn't.",
         next_node="Gen_Brawler_End"),
    _dlg("Gen_Brawler_04", "Gen_Brawler_04", "NPCLine", "Brawler",
         "That I can do. You just tell me when and which gate. I'll be at the front row and I won't need a weapon — just a big enough problem and enough room to swing.",
         next_node="Gen_Brawler_End"),
    _dlg("Gen_Brawler_End", "Gen_Brawler_End", "End", "", ""),

    # --- POOL 9: SICK MAN ---
    # Entry: Gen_SickMan_01
    # NOTE: Gen_SickMan_04 sets FlagToSet="Reward_Bandage" as a direct-grant
    # signal. C++ will need a one-shot item grant wired to this flag
    # (see implementation notes at bottom of file).
    _dlg("Gen_SickMan_01", "Gen_SickMan_01", "NPCLine", "Sick Man",
         "*his cough rattles the bunk boards* Been like this three weeks. Asha says it's the ventilation — same recycled air moving through the same sick lungs. Half the car sounds like me now.",
         flag_to_set="Flag_MetSickMan", flag_value=True, next_node="Gen_SickMan_02"),
    _dlg("Gen_SickMan_02", "Gen_SickMan_02", "PlayerChoice", "", "",
         choices=[
             _choice("Let me find you a bandage at least.", "Gen_SickMan_03"),
             _choice("Is it getting worse?", "Gen_SickMan_05"),
         ]),
    _dlg("Gen_SickMan_03", "Gen_SickMan_03", "NPCLine", "Sick Man",
         "Bandage won't fix lungs. But — yes. Thank you. Tying one around the face keeps some of the particulate out. Here, take this scrap in return — I'm done pretending I'll ever finish that shiv.",
         next_node="Gen_SickMan_04"),
    _dlg("Gen_SickMan_04", "Gen_SickMan_04", "SetFlag", "Sick Man",
         "",
         flag_to_set="Reward_Bandage", flag_value=True, next_node="Gen_SickMan_End"),
    _dlg("Gen_SickMan_05", "Gen_SickMan_05", "NPCLine", "Sick Man",
         "Every week. The blocks keep us alive but they don't keep us well. I figure I've got until the revolt or until spring, whichever's further away. I'm betting on the revolt.",
         next_node="Gen_SickMan_End"),
    _dlg("Gen_SickMan_End", "Gen_SickMan_End", "End", "", ""),

    # NOTE — DIALOGUE-DIRECT ITEM GRANTS:
    # Gen_SickMan_04 sets flag "Reward_Bandage" (NodeType=SetFlag, no text).
    # This is a one-shot grant signal: wire a BlueprintCallable or C++
    # OnDialogueFlagSet handler that calls InventoryComponent->AddItem("Item_Bandage", 1)
    # when "Reward_Bandage" is set, then immediately clears or checks the flag
    # so it doesn't fire again on a second conversation. The QuestManager's
    # HandleDialogueFlagSet already propagates SetFlag nodes, but AddItem must
    # be called by a separate grant handler — the QuestManager does NOT do
    # dialogue-direct grants (only quest completion grants in GrantRewards).
]


# ----- 3. QUESTS  (FSEEQuest — NOT FTableRowBase, stored as JSON array) ----
# FSEEQuest does not extend FTableRowBase so we provide it as structured data
# that can be loaded via a Blueprint-callable init function or DataAsset.
# We also provide it as a DataTable-compatible format in case the team wraps
# it in a FTableRowBase wrapper later.

OBJECTIVE_TYPE = {
    "GoTo": 0, "Interact": 1, "Kill": 2,
    "Collect": 3, "Escort": 4, "Dialogue": 5, "Custom": 6,
}

QUESTS = [
    # --- MAIN QUEST, PART 1: the riot, the kitchen, the count -------------
    {
        "RowName": "Quest_LongWalkForward",
        "QuestID": "Quest_LongWalkForward",  # ID kept for save compatibility
        "QuestName": "The Count of Bullets",
        "Description": "The ration was cut, the riot was crushed, and Old Man Pike has a theory: the guards' rifles have been empty for four years. Prove it — and the whole Tail stands up.",
        "State": 0,  # Available
        "bIsMainQuest": True,
        "Objectives": [
            {"ObjectiveID": "CB_01", "Description": "Walk the wreckage of the ration riot in Car 2", "Type": OBJECTIVE_TYPE["GoTo"], "TargetID": "Car2_RiotAftermath", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "CB_02", "Description": "Hear Old Man Pike's count", "Type": OBJECTIVE_TYPE["Dialogue"], "TargetID": "Pike_01", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "CB_03", "Description": "See for yourself what goes into the protein blocks", "Type": OBJECTIVE_TYPE["Interact"], "TargetID": "ProteinGrinder", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "CB_04", "Description": "Ask the Kronole Dealer about the gate man", "Type": OBJECTIVE_TYPE["Dialogue"], "TargetID": "Dealer_07", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "CB_05", "Description": "Free Yastrek, the gate specialist, from the pen behind the kitchen", "Type": OBJECTIVE_TYPE["Interact"], "TargetID": "HoldingPen_Yastrek", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "CB_06", "Description": "Rush the Blockade guards — and count the bullets", "Type": OBJECTIVE_TYPE["Kill"], "TargetID": "Jackboot_Blockade", "RequiredCount": 3, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "CB_07", "Description": "Bring Pike his proof", "Type": OBJECTIVE_TYPE["Dialogue"], "TargetID": "Pike_BulletReport", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
        ],
        "XPReward": 600,
        "ItemRewards": ["Item_Machete"],
        "FactionRepRewards": {"Tailies": 50, "Jackboots": -25},
        "PrerequisiteQuests": [],
    },
    # --- MAIN QUEST, PART 2: the assault on Martyr's Gate -----------------
    {
        "RowName": "Quest_MartyrsGate",
        "QuestID": "Quest_MartyrsGate",
        "QuestName": "Martyr's Gate",
        "Description": "The bullets were a bluff. Now the Tail moves — past the kill map of the last revolt and through the gate where sixty-two names were made. This time the names go forward.",
        "State": 0,
        "bIsMainQuest": True,
        "Objectives": [
            {"ObjectiveID": "MG_01", "Description": "Walk the kill map of the last revolt", "Type": OBJECTIVE_TYPE["GoTo"], "TargetID": "Car14_KillMap", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "MG_02", "Description": "Read the sixty-two names before asking anyone to add more", "Type": OBJECTIVE_TYPE["Interact"], "TargetID": "Memorial_Monument", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "MG_03", "Description": "(Optional) Set Josie's kronole charge on the gate hinge", "Type": OBJECTIVE_TYPE["Interact"], "TargetID": "GateHinge_BreachingCharge", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": True},
            {"ObjectiveID": "MG_04", "Description": "Break the garrison at Martyr's Gate", "Type": OBJECTIVE_TYPE["Kill"], "TargetID": "Jackboot_GateGarrison", "RequiredCount": 8, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "MG_05", "Description": "Bring down the Gate Captain", "Type": OBJECTIVE_TYPE["Kill"], "TargetID": "Boss_GateCaptain", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "MG_06", "Description": "Open the gate. Leave the Tail.", "Type": OBJECTIVE_TYPE["GoTo"], "TargetID": "Zone2_Threshold", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
        ],
        "XPReward": 1000,
        "ItemRewards": ["Item_JackbootArmor"],
        "FactionRepRewards": {"Tailies": 75, "Jackboots": -50, "ThirdClassUnion": 25},
        "PrerequisiteQuests": ["Quest_LongWalkForward"],
    },
    # --- SIDE: the injured woman — medicine, and a mercy choice -----------
    {
        "RowName": "Quest_MedicineRun",
        "QuestID": "Quest_MedicineRun",  # ID kept for save compatibility
        "QuestName": "What Mercy Costs",
        "Description": "Mara held the line at the riot and the Jackboots broke her ribs for it. The infection needs antiseptic from the guards' shelf in Car 8. The pain needs something else — and the Dealer sells it.",
        "State": 0,
        "bIsMainQuest": False,
        "Objectives": [
            {"ObjectiveID": "MR_01", "Description": "Hear Mara out", "Type": OBJECTIVE_TYPE["Dialogue"], "TargetID": "Injured_01", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "MR_02", "Description": "Take back antiseptic from the Car 8 confiscation shelf", "Type": OBJECTIVE_TYPE["Collect"], "TargetID": "Item_Antiseptic", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "MR_03", "Description": "Bring Mara the antiseptic", "Type": OBJECTIVE_TYPE["Interact"], "TargetID": "NPC_InjuredTailie", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "MR_04", "Description": "(Optional) Ask the Dealer for a dose to dull her pain", "Type": OBJECTIVE_TYPE["Dialogue"], "TargetID": "Dealer_12", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": True},
        ],
        "XPReward": 200,
        "ItemRewards": ["Item_Bandage", "Item_Bandage", "Item_Bandage"],
        "FactionRepRewards": {"Tailies": 25},
        "PrerequisiteQuests": [],
    },
    # --- SIDE: kronole as contact explosive — the revolt's ugly bargain ---
    {
        "RowName": "Quest_KronoleCleanup",
        "QuestID": "Quest_KronoleCleanup",  # ID kept for save compatibility
        "QuestName": "Burn Rate",
        "Description": "Kronole numbs the cold and the grief — and dried and packed, it takes doors off hinges. Every brick you pull for the revolt is a night's quiet taken from somebody else. Gather it anyway.",
        "State": 0,
        "bIsMainQuest": False,
        "Objectives": [
            {"ObjectiveID": "KC_01", "Description": "Learn from the Dealer what kronole really does", "Type": OBJECTIVE_TYPE["Dialogue"], "TargetID": "Dealer_06", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "KC_02", "Description": "Gather three bricks of kronole", "Type": OBJECTIVE_TYPE["Collect"], "TargetID": "Item_Kronole", "RequiredCount": 3, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "KC_03", "Description": "Scavenge chemical compound to stabilise the charge", "Type": OBJECTIVE_TYPE["Collect"], "TargetID": "Item_Chemicals", "RequiredCount": 2, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "KC_04", "Description": "Deliver everything to Josie's bench", "Type": OBJECTIVE_TYPE["Interact"], "TargetID": "NPC_Mechanic", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
        ],
        "XPReward": 300,
        "ItemRewards": [],
        "FactionRepRewards": {"Tailies": 20, "KronoleNetwork": -25},
        "PrerequisiteQuests": [],
    },
    # --- SIDE: weapons from scrap at Josie's bench -------------------------
    {
        "RowName": "Quest_TheWorkshop",
        "QuestID": "Quest_TheWorkshop",  # ID kept for save compatibility
        "QuestName": "Steel Doesn't Run Out",
        "Description": "Josie can turn wall panels into spears, but the bench runs on scrap and the scrap runs on you. Arm yourself first. Then arm the Tail.",
        "State": 0,
        "bIsMainQuest": False,
        "Objectives": [
            {"ObjectiveID": "WS_01", "Description": "Report to Josie's bench", "Type": OBJECTIVE_TYPE["Dialogue"], "TargetID": "Mechanic_01", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "WS_02", "Description": "Collect 3 Scrap Metal", "Type": OBJECTIVE_TYPE["Collect"], "TargetID": "Item_ScrapMetal", "RequiredCount": 3, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "WS_03", "Description": "Collect 2 Cloth Strips", "Type": OBJECTIVE_TYPE["Collect"], "TargetID": "Item_ClothStrips", "RequiredCount": 2, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "WS_04", "Description": "Forge a Reinforced Shiv at the workbench", "Type": OBJECTIVE_TYPE["Custom"], "TargetID": "Craft_ReinforcedShiv", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "WS_05", "Description": "(Optional) Strip 2 Wire Coils so Josie can arm forty hands", "Type": OBJECTIVE_TYPE["Collect"], "TargetID": "Item_Wire", "RequiredCount": 2, "CurrentCount": 0, "bCompleted": False, "bOptional": True},
        ],
        "XPReward": 200,
        "ItemRewards": ["Item_Shiv"],
        "FactionRepRewards": {"Tailies": 10},
        "PrerequisiteQuests": [],
    },
    # --- SIDE: the paper trail that backs Pike's arithmetic ---------------
    {
        "RowName": "Quest_ListeningPost",
        "QuestID": "Quest_ListeningPost",  # ID kept for save compatibility
        "QuestName": "Count the Shots",
        "Description": "Paper doesn't lie the way mouths do. Somewhere in the guards' own paperwork is the proof of Pike's arithmetic: four years of requisitions, and not one crate of ammunition.",
        "State": 0,
        "bIsMainQuest": False,
        "Objectives": [
            {"ObjectiveID": "LP_01", "Description": "Find the guard rotation roster", "Type": OBJECTIVE_TYPE["Collect"], "TargetID": "Collect_Intel_Z1_01", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "LP_02", "Description": "Find the supply requisition forms — note what's missing", "Type": OBJECTIVE_TYPE["Collect"], "TargetID": "Collect_Intel_Z1_02", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "LP_03", "Description": "Find the encrypted transmission transcript", "Type": OBJECTIVE_TYPE["Collect"], "TargetID": "Collect_Intel_Z1_03", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "LP_04", "Description": "Bring the paper trail to Pike", "Type": OBJECTIVE_TYPE["Dialogue"], "TargetID": "Pike_Intel", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
        ],
        "XPReward": 300,
        "ItemRewards": ["Item_CommanderOrders"],
        "FactionRepRewards": {"Tailies": 30, "Jackboots": -20},
        "PrerequisiteQuests": [],
    },
    # --- SIDE: the taken children — investigation foreshadow only ---------
    {
        "RowName": "Quest_TakenChildren",
        "QuestID": "Quest_TakenChildren",
        "QuestName": "The Small Ones",
        "Description": "Eleven children taken forward in two years. All under five. All measured first. No letters come back. Tanya wants the one thing the Tail can't ration: an answer.",
        "State": 0,
        "bIsMainQuest": False,
        "Objectives": [
            {"ObjectiveID": "TC_01", "Description": "Listen to Tanya", "Type": OBJECTIVE_TYPE["Dialogue"], "TargetID": "Tanya_01", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "TC_02", "Description": "Check the nursery ledger — who was measured, who was taken", "Type": OBJECTIVE_TYPE["Interact"], "TargetID": "Nursery_Ledger", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "TC_03", "Description": "Find the engine maintenance schedule fragment", "Type": OBJECTIVE_TYPE["Collect"], "TargetID": "Collect_Manifest_Z1_05", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "TC_04", "Description": "Decide what to tell Tanya", "Type": OBJECTIVE_TYPE["Dialogue"], "TargetID": "Tanya_Report", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
        ],
        "XPReward": 250,
        "ItemRewards": ["Item_TinStar"],
        "FactionRepRewards": {"Tailies": 20},
        "PrerequisiteQuests": [],
    },
    # --- SIDE: the memorial names — collect / lore -------------------------
    {
        "RowName": "Quest_MemorialNames",
        "QuestID": "Quest_MemorialNames",
        "QuestName": "Sixty-Two Names",
        "Description": "The wall in Car 1 holds the names of the Martyrs — but the train shakes, the scratches fade, and the dead left things behind. Recover what they carried. Cut the names back in deep.",
        "State": 0,
        "bIsMainQuest": False,
        "Objectives": [
            {"ObjectiveID": "MN_01", "Description": "Ask Pike about the Martyrs", "Type": OBJECTIVE_TYPE["Dialogue"], "TargetID": "Pike_12", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "MN_02", "Description": "Recover the wedding ring", "Type": OBJECTIVE_TYPE["Collect"], "TargetID": "Collect_Personal_Z1_02", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "MN_03", "Description": "Recover the hand-carved figurine", "Type": OBJECTIVE_TYPE["Collect"], "TargetID": "Collect_Personal_Z1_04", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "MN_04", "Description": "Recover the diary of the first revolt", "Type": OBJECTIVE_TYPE["Collect"], "TargetID": "Collect_Personal_Z1_05", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "MN_05", "Description": "Cut the fading names back into the memorial wall", "Type": OBJECTIVE_TYPE["Interact"], "TargetID": "MemorialWall_Car1", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
        ],
        "XPReward": 300,
        "ItemRewards": [],
        "FactionRepRewards": {"Tailies": 30},
        "PrerequisiteQuests": [],
    },
]


# ----- 4. COLLECTIBLES  (FCollectibleData, row‑name = CollectibleID) ----

COLLECTIBLE_TYPE = {
    "TrainLog": 0, "AudioRecording": 1, "Artifact": 2,
    "Blueprint": 3, "FactionIntel": 4, "ManifestPage": 5,
}
COLLECTIBLE_RARITY = {"Common": 0, "Uncommon": 1, "Rare": 2, "Legendary": 3}
TRAIN_ZONE = {
    "Zone1_Tail": 0, "Zone2_ThirdClass": 1, "Zone3_SecondClass": 2,
    "Zone4_WorkingSpine": 3, "Zone5_FirstClass": 4, "Zone6_Sanctum": 5,
    "Zone7_Engine": 6, "Exterior": 7,
}
ACCESS_TYPE = {
    "Visible": 0, "EnvironmentPuzzle": 1, "LockedContainer": 2,
    "FalseWall": 3, "VentAccess": 4, "CompanionTriggered": 5,
    "ExteriorOnly": 6, "TimedEvent": 7,
}
PERSISTENCE = {"Persistent": 0, "Timed": 1, "ChoiceDependent": 2}

COLLECTIBLES = [
    # --- Manifest Pages (5) ---
    {
        "RowName": "Collect_Manifest_Z1_01",
        "CollectibleID": "Collect_Manifest_Z1_01",
        "DisplayName": "Manifest Page — Original Passenger List (Tail)",
        "Description": "A yellowed ledger page listing the original Tail passengers by name. Many have been crossed out. The handwriting grows increasingly erratic toward the bottom.",
        "Type": COLLECTIBLE_TYPE["ManifestPage"],
        "Rarity": COLLECTIBLE_RARITY["Uncommon"],
        "Zone": TRAIN_ZONE["Zone1_Tail"],
        "AccessType": ACCESS_TYPE["Visible"],
        "Persistence": PERSISTENCE["Persistent"],
        "CarNumber": 1,
        "FactionID": "Tailies",
    },
    {
        "RowName": "Collect_Manifest_Z1_02",
        "CollectibleID": "Collect_Manifest_Z1_02",
        "DisplayName": "Manifest Page — Supply Allocation Table",
        "Description": "A logistics table showing protein block allocation per car. The Tail receives less than a tenth of Third Class. Someone has circled the figures in red.",
        "Type": COLLECTIBLE_TYPE["ManifestPage"],
        "Rarity": COLLECTIBLE_RARITY["Uncommon"],
        "Zone": TRAIN_ZONE["Zone1_Tail"],
        "AccessType": ACCESS_TYPE["LockedContainer"],
        "Persistence": PERSISTENCE["Persistent"],
        "CarNumber": 2,
        "FactionID": "Bureaucracy",
    },
    {
        "RowName": "Collect_Manifest_Z1_03",
        "CollectibleID": "Collect_Manifest_Z1_03",
        "DisplayName": "Manifest Page — Drawer Inventory (Classified)",
        "Description": "A sealed document cataloguing the cryogenic drawers. Names, dates of freezing, and a column labelled 'Retrieval Priority.' Most entries say 'NONE.'",
        "Type": COLLECTIBLE_TYPE["ManifestPage"],
        "Rarity": COLLECTIBLE_RARITY["Rare"],
        "Zone": TRAIN_ZONE["Zone1_Tail"],
        "AccessType": ACCESS_TYPE["FalseWall"],
        "Persistence": PERSISTENCE["Persistent"],
        "CarNumber": 3,
        "FactionID": "Jackboots",
    },
    {
        "RowName": "Collect_Manifest_Z1_04",
        "CollectibleID": "Collect_Manifest_Z1_04",
        "DisplayName": "Manifest Page — Wilford Industries Pre-Departure Memo",
        "Description": "An internal memo from before the Freeze. It discusses 'acceptable population density' and 'managed attrition rates.' The corporate language barely conceals the cruelty.",
        "Type": COLLECTIBLE_TYPE["ManifestPage"],
        "Rarity": COLLECTIBLE_RARITY["Rare"],
        "Zone": TRAIN_ZONE["Zone1_Tail"],
        "AccessType": ACCESS_TYPE["VentAccess"],
        "Persistence": PERSISTENCE["Persistent"],
        "CarNumber": 2,
        "FactionID": "Bureaucracy",
    },
    {
        "RowName": "Collect_Manifest_Z1_05",
        "CollectibleID": "Collect_Manifest_Z1_05",
        "DisplayName": "Manifest Page — Engine Maintenance Schedule (Fragment)",
        "Description": "A torn corner of a maintenance log. References to 'child-sized components' and 'replacement cycles.' The implications are horrifying.",
        "Type": COLLECTIBLE_TYPE["ManifestPage"],
        "Rarity": COLLECTIBLE_RARITY["Legendary"],
        "Zone": TRAIN_ZONE["Zone1_Tail"],
        "AccessType": ACCESS_TYPE["EnvironmentPuzzle"],
        "Persistence": PERSISTENCE["Persistent"],
        "CarNumber": 1,
        "FactionID": "OrderOfTheEngine",
    },
    # --- Intel Documents (5) ---
    {
        "RowName": "Collect_Intel_Z1_01",
        "CollectibleID": "Collect_Intel_Z1_01",
        "DisplayName": "Intel — Guard Rotation Roster",
        "Description": "A crumpled schedule showing Jackboot patrol times for Cars 1 through 5. The night shift has a 12-minute gap between 0200 and 0214.",
        "Type": COLLECTIBLE_TYPE["FactionIntel"],
        "Rarity": COLLECTIBLE_RARITY["Uncommon"],
        "Zone": TRAIN_ZONE["Zone1_Tail"],
        "AccessType": ACCESS_TYPE["Visible"],
        "Persistence": PERSISTENCE["Persistent"],
        "CarNumber": 3,
        "FactionID": "Jackboots",
    },
    {
        "RowName": "Collect_Intel_Z1_02",
        "CollectibleID": "Collect_Intel_Z1_02",
        "DisplayName": "Intel — Supply Requisition Form",
        "Description": "A carbon-copy form showing what the Jackboots order from forward cars. Medical supplies, real food, alcohol. Everything the Tail is denied.",
        "Type": COLLECTIBLE_TYPE["FactionIntel"],
        "Rarity": COLLECTIBLE_RARITY["Common"],
        "Zone": TRAIN_ZONE["Zone1_Tail"],
        "AccessType": ACCESS_TYPE["LockedContainer"],
        "Persistence": PERSISTENCE["Persistent"],
        "CarNumber": 4,
        "FactionID": "Jackboots",
    },
    {
        "RowName": "Collect_Intel_Z1_03",
        "CollectibleID": "Collect_Intel_Z1_03",
        "DisplayName": "Intel — Encrypted Transmission Transcript",
        "Description": "A decoded radio intercept between the Tail gate commander and someone in First Class. They discuss 'the next culling' with chilling detachment.",
        "Type": COLLECTIBLE_TYPE["FactionIntel"],
        "Rarity": COLLECTIBLE_RARITY["Rare"],
        "Zone": TRAIN_ZONE["Zone1_Tail"],
        "AccessType": ACCESS_TYPE["EnvironmentPuzzle"],
        "Persistence": PERSISTENCE["Persistent"],
        "CarNumber": 5,
        "FactionID": "FirstClassElite",
    },
    {
        "RowName": "Collect_Intel_Z1_04",
        "CollectibleID": "Collect_Intel_Z1_04",
        "DisplayName": "Intel — Kronole Distribution Map",
        "Description": "A hand-drawn map showing drop points for Kronole throughout the lower cars. The Network reaches further than anyone realised.",
        "Type": COLLECTIBLE_TYPE["FactionIntel"],
        "Rarity": COLLECTIBLE_RARITY["Uncommon"],
        "Zone": TRAIN_ZONE["Zone1_Tail"],
        "AccessType": ACCESS_TYPE["VentAccess"],
        "Persistence": PERSISTENCE["Persistent"],
        "CarNumber": 2,
        "FactionID": "KronoleNetwork",
    },
    {
        "RowName": "Collect_Intel_Z1_05",
        "CollectibleID": "Collect_Intel_Z1_05",
        "DisplayName": "Intel — Dissenter Watch List",
        "Description": "A list of names marked for 'enhanced observation.' Your name has been added recently, in fresh ink.",
        "Type": COLLECTIBLE_TYPE["FactionIntel"],
        "Rarity": COLLECTIBLE_RARITY["Rare"],
        "Zone": TRAIN_ZONE["Zone1_Tail"],
        "AccessType": ACCESS_TYPE["FalseWall"],
        "Persistence": PERSISTENCE["Persistent"],
        "CarNumber": 3,
        "FactionID": "Jackboots",
    },
    # --- Personal Items (5) ---
    {
        "RowName": "Collect_Personal_Z1_01",
        "CollectibleID": "Collect_Personal_Z1_01",
        "DisplayName": "Child's Drawing — 'The Sun'",
        "Description": "A crayon drawing of a yellow circle over green hills. The child who drew it has never seen the sun. Below the picture: 'When the ice goes away.'",
        "Type": COLLECTIBLE_TYPE["Artifact"],
        "Rarity": COLLECTIBLE_RARITY["Common"],
        "Zone": TRAIN_ZONE["Zone1_Tail"],
        "AccessType": ACCESS_TYPE["Visible"],
        "Persistence": PERSISTENCE["Persistent"],
        "CarNumber": 1,
        "FactionID": "Tailies",
    },
    {
        "RowName": "Collect_Personal_Z1_02",
        "CollectibleID": "Collect_Personal_Z1_02",
        "DisplayName": "Wedding Ring",
        "Description": "A thin gold band, scratched and worn. The inscription inside reads 'Until the world thaws — M & J.' One of them is in the drawers now.",
        "Type": COLLECTIBLE_TYPE["Artifact"],
        "Rarity": COLLECTIBLE_RARITY["Uncommon"],
        "Zone": TRAIN_ZONE["Zone1_Tail"],
        "AccessType": ACCESS_TYPE["LockedContainer"],
        "Persistence": PERSISTENCE["Persistent"],
        "CarNumber": 2,
        "FactionID": "Tailies",
    },
    {
        "RowName": "Collect_Personal_Z1_03",
        "CollectibleID": "Collect_Personal_Z1_03",
        "DisplayName": "Faded Photograph",
        "Description": "A photograph of a family standing outside a house, summer sun bright overhead. The faces are nearly worn away from years of handling. Someone carried this through the boarding.",
        "Type": COLLECTIBLE_TYPE["Artifact"],
        "Rarity": COLLECTIBLE_RARITY["Common"],
        "Zone": TRAIN_ZONE["Zone1_Tail"],
        "AccessType": ACCESS_TYPE["Visible"],
        "Persistence": PERSISTENCE["Persistent"],
        "CarNumber": 1,
        "FactionID": "Neutral",
    },
    {
        "RowName": "Collect_Personal_Z1_04",
        "CollectibleID": "Collect_Personal_Z1_04",
        "DisplayName": "Hand-Carved Figurine",
        "Description": "A small wooden figure of a woman, carved with surprising skill from a chair leg. The artist's initials — 'EP' — are scratched into the base.",
        "Type": COLLECTIBLE_TYPE["Artifact"],
        "Rarity": COLLECTIBLE_RARITY["Uncommon"],
        "Zone": TRAIN_ZONE["Zone1_Tail"],
        "AccessType": ACCESS_TYPE["VentAccess"],
        "Persistence": PERSISTENCE["Persistent"],
        "CarNumber": 3,
        "FactionID": "Tailies",
    },
    {
        "RowName": "Collect_Personal_Z1_05",
        "CollectibleID": "Collect_Personal_Z1_05",
        "DisplayName": "Diary of the First Revolt",
        "Description": "A battered journal recounting the first uprising, seven years into the journey. The author survived. The revolt did not. The final entry reads: 'We will try again.'",
        "Type": COLLECTIBLE_TYPE["Artifact"],
        "Rarity": COLLECTIBLE_RARITY["Rare"],
        "Zone": TRAIN_ZONE["Zone1_Tail"],
        "AccessType": ACCESS_TYPE["FalseWall"],
        "Persistence": PERSISTENCE["Persistent"],
        "CarNumber": 1,
        "FactionID": "Tailies",
    },
    # --- Lore Objects (5) ---
    {
        "RowName": "Collect_Lore_Z1_01",
        "CollectibleID": "Collect_Lore_Z1_01",
        "DisplayName": "Train Log — Day One",
        "Description": "An audio recording from Snowpiercer's first day of operation. The announcer's voice is calm, professional. 'Welcome aboard. Your new life begins now.' The screaming in the background tells a different story.",
        "Type": COLLECTIBLE_TYPE["TrainLog"],
        "Rarity": COLLECTIBLE_RARITY["Uncommon"],
        "Zone": TRAIN_ZONE["Zone1_Tail"],
        "AccessType": ACCESS_TYPE["Visible"],
        "Persistence": PERSISTENCE["Persistent"],
        "CarNumber": 1,
        "FactionID": "Neutral",
    },
    {
        "RowName": "Collect_Lore_Z1_02",
        "CollectibleID": "Collect_Lore_Z1_02",
        "DisplayName": "Train Log — The First Winter",
        "Description": "A recording from the engineer's log, six months in. Reports of external temperature dropping below minus 120. The train's skin creaks and groans. 'She's holding. Barely.'",
        "Type": COLLECTIBLE_TYPE["TrainLog"],
        "Rarity": COLLECTIBLE_RARITY["Common"],
        "Zone": TRAIN_ZONE["Zone1_Tail"],
        "AccessType": ACCESS_TYPE["LockedContainer"],
        "Persistence": PERSISTENCE["Persistent"],
        "CarNumber": 2,
        "FactionID": "OrderOfTheEngine",
    },
    {
        "RowName": "Collect_Lore_Z1_03",
        "CollectibleID": "Collect_Lore_Z1_03",
        "DisplayName": "Audio Recording — Wilford's Address",
        "Description": "A public address from Mr. Wilford himself. 'Every passenger has their place. Every place has its purpose. The Engine provides.' The crowd chants back. The Tail section is silent.",
        "Type": COLLECTIBLE_TYPE["AudioRecording"],
        "Rarity": COLLECTIBLE_RARITY["Uncommon"],
        "Zone": TRAIN_ZONE["Zone1_Tail"],
        "AccessType": ACCESS_TYPE["EnvironmentPuzzle"],
        "Persistence": PERSISTENCE["Persistent"],
        "CarNumber": 3,
        "FactionID": "OrderOfTheEngine",
    },
    {
        "RowName": "Collect_Lore_Z1_04",
        "CollectibleID": "Collect_Lore_Z1_04",
        "DisplayName": "Blueprint Fragment — Tail Car Modifications",
        "Description": "Technical drawings showing how the Tail cars were modified after departure. Comfort features removed, bunks tripled, ventilation reduced to minimum. Dated three months after launch.",
        "Type": COLLECTIBLE_TYPE["Blueprint"],
        "Rarity": COLLECTIBLE_RARITY["Rare"],
        "Zone": TRAIN_ZONE["Zone1_Tail"],
        "AccessType": ACCESS_TYPE["VentAccess"],
        "Persistence": PERSISTENCE["Persistent"],
        "CarNumber": 1,
        "FactionID": "Bureaucracy",
    },
    {
        "RowName": "Collect_Lore_Z1_05",
        "CollectibleID": "Collect_Lore_Z1_05",
        "DisplayName": "Audio Recording — Last Outside Broadcast",
        "Description": "The final radio transmission received from the outside world, recorded eight months after the Freeze. Static, then a voice: 'If anyone can hear this... we are the last. God help us.' Then silence. Forever.",
        "Type": COLLECTIBLE_TYPE["AudioRecording"],
        "Rarity": COLLECTIBLE_RARITY["Legendary"],
        "Zone": TRAIN_ZONE["Zone1_Tail"],
        "AccessType": ACCESS_TYPE["CompanionTriggered"],
        "Persistence": PERSISTENCE["ChoiceDependent"],
        "CarNumber": 4,
        "FactionID": "Neutral",
    },
]


# ===================================================================
#  UE5 EDITOR CREATION (unreal Python API)
# ===================================================================

def _asset_exists(asset_path):
    """Check whether an asset already exists in the Content Browser."""
    return unreal.EditorAssetLibrary.does_asset_exist(asset_path)


def _ensure_directory(dir_path):
    """Create a Content Browser directory if it does not exist."""
    if not unreal.EditorAssetLibrary.does_directory_exist(dir_path):
        unreal.EditorAssetLibrary.make_directory(dir_path)
        log.info("Created directory: %s", dir_path)


def _save_asset(asset_path):
    """Save a single asset to disk."""
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.EditorAssetLibrary.save_asset(asset_path, only_if_is_dirty=False)
        log.info("Saved: %s", asset_path)
    else:
        log.warning("Cannot save — asset not found: %s", asset_path)


# ---- Item DataTable ----

def _set_item_row(dt, row_name, data):
    """Add a single item row to the DataTable using row-handle property editing."""
    # We use the DataTableFunctionLibrary to retrieve a row handle, then set
    # properties.  UE5's Python does not expose a direct "add row" on UDataTable
    # for struct-based tables, so we use the JSON import approach instead.
    pass  # Handled via JSON import below


def _build_item_json_string():
    """Build a JSON string compatible with UDataTable::CreateTableFromJSONString / reimport."""
    rows = []
    for item in ITEMS:
        rows.append({
            "Name": item["RowName"],
            "ItemID": item["ItemID"],
            "DisplayName": item["DisplayName"],
            "Description": item["Description"],
            "Category": _enum_str("ESEEItemCategory", item["Category"]),
            "Rarity": _enum_str("ESEEItemRarity", item["Rarity"]),
            "Weight": item["Weight"],
            "MaxStackSize": item["MaxStackSize"],
            "Value": item["Value"],
            "Icon": "",
            "HealthRestore": item["HealthRestore"],
            "HungerRestore": item["HungerRestore"],
            "StaminaRestore": item["StaminaRestore"],
            "WeaponClass": "",
            "BluntArmor": item["BluntArmor"],
            "BladedArmor": item["BladedArmor"],
            "PiercingArmor": item["PiercingArmor"],
            "ColdResistance": item["ColdResistance"],
        })
    return rows


def _enum_str(enum_name, ordinal):
    """Convert an ordinal to the 'EnumName::Value' string UE expects in JSON imports."""
    # Map ordinal back to name based on our dictionaries
    maps = {
        "ESEEItemCategory": {v: k for k, v in ITEM_CATEGORY.items()},
        "ESEEItemRarity": {v: k for k, v in ITEM_RARITY.items()},
        "ESEEDialogueNodeType": {v: k for k, v in NODE_TYPE.items()},
        "ECollectibleType": {v: k for k, v in COLLECTIBLE_TYPE.items()},
        "ECollectibleRarity": {v: k for k, v in COLLECTIBLE_RARITY.items()},
        "ECollectibleZone": {v: k for k, v in TRAIN_ZONE.items()},
        "ECollectibleAccessType": {v: k for k, v in ACCESS_TYPE.items()},
        "ECollectiblePersistence": {v: k for k, v in PERSISTENCE.items()},
        "ESEEObjectiveType": {v: k for k, v in OBJECTIVE_TYPE.items()},
    }
    name_map = maps.get(enum_name, {})
    return f"{enum_name}::{name_map.get(ordinal, str(ordinal))}"


def create_datatable_via_json_import(asset_name, struct_name, rows_json, asset_dir=CONTENT_DT_PATH):
    """
    Create a DataTable asset by:
      1. Writing a temporary JSON file
      2. Using unreal.DataTableFunctionLibrary or AssetTools to import it
      3. Cleaning up the temp file

    This is the most reliable cross-version approach for UE5 Python DataTable creation.
    """
    asset_path = f"{asset_dir}/{asset_name}"
    if _asset_exists(asset_path):
        log.info("Asset already exists, skipping: %s", asset_path)
        return asset_path

    _ensure_directory(asset_dir)

    # Write temp JSON
    import tempfile
    tmp = tempfile.NamedTemporaryFile(
        mode="w", suffix=".json", delete=False, prefix=f"see_{asset_name}_"
    )
    json.dump(rows_json, tmp, indent=2)
    tmp.close()
    tmp_path = tmp.name.replace("\\", "/")

    try:
        # Attempt 1: Use AssetTools factory import
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

        # Create the DataTable asset first
        factory = unreal.DataTableFactory()
        # Resolve the row struct. UScriptStruct objects are registered WITHOUT
        # the C++ 'F' prefix (FSEEItemData -> /Script/SnowpiercerEE.SEEItemData),
        # so try both spellings across the lookup methods.
        bare_name = struct_name[1:] if struct_name.startswith("F") else struct_name
        resolved = None
        for candidate in (bare_name, struct_name):
            try:
                resolved = unreal.find_object(None, f"/Script/SnowpiercerEE.{candidate}")
            except Exception:
                resolved = None
            if resolved:
                break
            try:
                resolved = unreal.load_object(None, f"/Script/SnowpiercerEE.{candidate}")
            except Exception:
                resolved = None
            if resolved:
                break
        if not resolved:
            log.error("Could not resolve struct %s", struct_name)
            raise RuntimeError(f"Row struct {struct_name} not found in /Script/SnowpiercerEE")
        factory.struct = resolved

        # Create the asset
        dt = asset_tools.create_asset(
            asset_name=asset_name,
            package_path=asset_dir,
            asset_class=unreal.DataTable,
            factory=factory,
        )

        if dt is None:
            log.error("Failed to create DataTable asset: %s", asset_path)
            return None

        # Fill from JSON string
        json_string = json.dumps(rows_json)
        result = unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(dt, json_string)
        if not result:
            log.warning("fill_data_table_from_json_string returned False for %s — rows may need manual import.", asset_name)

        _save_asset(asset_path)
        log.info("Created DataTable: %s  (%d rows)", asset_path, len(rows_json))
        return asset_path

    except Exception as e:
        log.error("Editor DataTable creation failed for %s: %s", asset_name, e)
        log.info("Falling back to JSON export for %s", asset_name)
        return None
    finally:
        try:
            os.unlink(tmp_path)
        except OSError:
            pass


# ---- Build UE‑compatible JSON for each table ----

def _build_dialogue_json():
    rows = []
    for node in DIALOGUE_ZONE1:
        choices = []
        for c in node.get("Choices", []):
            choices.append({
                "ChoiceText": c["ChoiceText"],
                "NextNodeID": c["NextNodeID"],
                "RequiredFlag": c["RequiredFlag"],
                "RequiredStat": c["RequiredStat"],
                "RequiredStatValue": c["RequiredStatValue"],
            })
        rows.append({
            "Name": node["RowName"],
            "NodeID": node["NodeID"],
            "NodeType": _enum_str("ESEEDialogueNodeType", node["NodeType"]),
            "SpeakerName": node["SpeakerName"],
            "DialogueText": node["DialogueText"],
            "Choices": choices,
            "NextNodeID": node["NextNodeID"],
            "FlagToSet": node["FlagToSet"],
            "FlagValue": node["FlagValue"],
            "BranchFlag": node["BranchFlag"],
            "BranchTrueNode": node["BranchTrueNode"],
            "BranchFalseNode": node["BranchFalseNode"],
            "TimedResponseDuration": node["TimedResponseDuration"],
            "VoiceAudio": "",
        })
    return rows


def _build_quest_json():
    rows = []
    for q in QUESTS:
        objectives = []
        for obj in q["Objectives"]:
            objectives.append({
                "ObjectiveID": obj["ObjectiveID"],
                "Description": obj["Description"],
                "Type": _enum_str("ESEEObjectiveType", obj["Type"]),
                "TargetID": obj["TargetID"],
                "RequiredCount": obj["RequiredCount"],
                "CurrentCount": obj["CurrentCount"],
                "bCompleted": obj["bCompleted"],
                "bOptional": obj["bOptional"],
            })
        rows.append({
            "Name": q["RowName"],
            "QuestID": q["QuestID"],
            "QuestName": q["QuestName"],
            "Description": q["Description"],
            "State": "ESEEQuestState::Available",
            "bIsMainQuest": q["bIsMainQuest"],
            "Objectives": objectives,
            "XPReward": q["XPReward"],
            "ItemRewards": q["ItemRewards"],
            "FactionRepRewards": q.get("FactionRepRewards", {}),
            "PrerequisiteQuests": q.get("PrerequisiteQuests", []),
        })
    return rows


def _build_collectible_json():
    rows = []
    for c in COLLECTIBLES:
        rows.append({
            "Name": c["RowName"],
            "CollectibleID": c["CollectibleID"],
            "DisplayName": c["DisplayName"],
            "Description": c["Description"],
            "Type": _enum_str("ECollectibleType", c["Type"]),
            "Rarity": _enum_str("ECollectibleRarity", c["Rarity"]),
            "Zone": _enum_str("ECollectibleZone", c["Zone"]),
            "AccessType": _enum_str("ECollectibleAccessType", c["AccessType"]),
            "Persistence": _enum_str("ECollectiblePersistence", c["Persistence"]),
            "CarNumber": c["CarNumber"],
            "StatRequirements": {},
            "LinkedCollectibles": [],
            "ResonanceCompanion": "",
            "ResonanceLoyaltyBonus": 0,
            "CodexEntries": [],
            "AudioAsset": "",
            "InspectMesh": "",
            "UnlockedRecipeID": "",
            "FactionID": c.get("FactionID", ""),
        })
    return rows


# ===================================================================
#  JSON FALLBACK (standalone / CI mode)
# ===================================================================

def write_json_fallback():
    """Write all DataTable data as JSON files for manual import into UE5."""
    os.makedirs(JSON_OUTPUT_DIR, exist_ok=True)

    tables = {
        "DT_Items": _build_item_json_string(),
        "DT_Dialogue_Zone1": _build_dialogue_json(),
        "DT_Quests": _build_quest_json(),
        "DT_Collectibles": _build_collectible_json(),
    }

    for name, rows in tables.items():
        path = JSON_OUTPUT_DIR / f"{name}.json"
        with open(path, "w", encoding="utf-8") as f:
            json.dump(rows, f, indent=2, ensure_ascii=False)
        log.info("Wrote JSON: %s  (%d rows)", path, len(rows))

    # Write import instructions
    instructions_path = JSON_OUTPUT_DIR / "IMPORT_INSTRUCTIONS.txt"
    with open(instructions_path, "w", encoding="utf-8") as f:
        f.write(IMPORT_INSTRUCTIONS)
    log.info("Wrote import instructions: %s", instructions_path)

    return tables


IMPORT_INSTRUCTIONS = """\
=============================================================================
  Snowpiercer: Eternal Engine — DataTable JSON Import Instructions
=============================================================================

These JSON files contain all game data for DataTables. To import them into
Unreal Engine 5:

METHOD 1 — Reimport via Editor (Recommended)
---------------------------------------------
1. Open UE5 Editor with the SnowpiercerEE project.
2. In Content Browser, right-click > Miscellaneous > DataTable.
3. Select the Row Structure:
     DT_Items.json          -> FSEEItemData
     DT_Dialogue_Zone1.json -> FSEEDialogueNode
     DT_Quests.json         -> FSEEQuest  (requires FTableRowBase wrapper)
     DT_Collectibles.json   -> FCollectibleData
4. Name the DataTable (e.g., DT_Items).
5. Right-click the new DataTable > Reimport > select the JSON file.
6. Save the asset.

METHOD 2 — Python Script (Inside Editor)
-----------------------------------------
1. Open UE5 Editor.
2. Edit > Execute Python Script...
3. Browse to: Scripts/create_datatables.py
4. The script auto-detects the editor environment and creates all tables.

METHOD 3 — Console Command
---------------------------
In the UE5 Output Log, type:
    py "Scripts/create_datatables.py"

STRUCT MAPPING
--------------
  DT_Items          -> FSEEItemData        (SEEItemBase.h)
  DT_Dialogue_Zone1 -> FSEEDialogueNode    (SEEDialogueManager.h)
  DT_Quests         -> FSEEQuest            (SEEQuestManager.h)
  DT_Collectibles   -> FCollectibleData     (Exploration/CollectibleTypes.h)

NOTE: FSEEQuest does not extend FTableRowBase in the current C++ code.
      To use it as a DataTable row struct, either:
        a) Add ": public FTableRowBase" to FSEEQuest in SEEQuestManager.h, or
        b) Create a wrapper struct FSEEQuestRow : public FTableRowBase that
           contains an FSEEQuest member, or
        c) Load quest data at runtime via UDataAsset or JSON deserialization.
      The JSON format provided here supports all three approaches.
=============================================================================
"""


# ===================================================================
#  MAIN ENTRY POINT
# ===================================================================

def main():
    log.info("=" * 60)
    log.info("  Snowpiercer: Eternal Engine — DataTable Creator")
    log.info("=" * 60)

    if IN_EDITOR:
        log.info("Creating DataTables via UE5 Editor Python API...")
        _ensure_directory(CONTENT_DT_PATH)

        tables_created = 0
        tables_failed = 0

        # Items
        try:
            result = create_datatable_via_json_import(
                "DT_Items", "FSEEItemData", _build_item_json_string()
            )
            if result:
                tables_created += 1
            else:
                tables_failed += 1
        except Exception as e:
            log.error("DT_Items creation error: %s", e)
            tables_failed += 1

        # Dialogue
        try:
            result = create_datatable_via_json_import(
                "DT_Dialogue_Zone1", "FSEEDialogueNode", _build_dialogue_json()
            )
            if result:
                tables_created += 1
            else:
                tables_failed += 1
        except Exception as e:
            log.error("DT_Dialogue_Zone1 creation error: %s", e)
            tables_failed += 1

        # Quests — FSEEQuest may not extend FTableRowBase; attempt anyway
        try:
            result = create_datatable_via_json_import(
                "DT_Quests", "FSEEQuest", _build_quest_json()
            )
            if result:
                tables_created += 1
            else:
                tables_failed += 1
        except Exception as e:
            log.warning(
                "DT_Quests: FSEEQuest may not be a FTableRowBase. "
                "Quest data will be available as JSON fallback. Error: %s", e
            )
            tables_failed += 1

        # Collectibles
        try:
            result = create_datatable_via_json_import(
                "DT_Collectibles", "FCollectibleData", _build_collectible_json()
            )
            if result:
                tables_created += 1
            else:
                tables_failed += 1
        except Exception as e:
            log.error("DT_Collectibles creation error: %s", e)
            tables_failed += 1

        log.info("-" * 60)
        log.info("Editor results: %d created, %d failed", tables_created, tables_failed)

        if tables_failed > 0:
            log.info("Writing JSON fallback for failed tables...")
            write_json_fallback()
    else:
        # Standalone mode — always write JSON
        write_json_fallback()

    log.info("=" * 60)
    log.info("  DataTable creation complete.")
    log.info("=" * 60)

    # Summary
    log.info("Items:        %d entries", len(ITEMS))
    log.info("Dialogue:     %d nodes (7 named + 10 generic-pool trees)", len(DIALOGUE_ZONE1))
    log.info("Quests:       %d quests", len(QUESTS))
    log.info("Collectibles: %d entries", len(COLLECTIBLES))
    log.info("Total:        %d data rows",
             len(ITEMS) + len(DIALOGUE_ZONE1) + len(QUESTS) + len(COLLECTIBLES))


if __name__ == "__main__":
    main()
else:
    # When executed via UE5's "Execute Python Script", __name__ is not "__main__"
    main()
