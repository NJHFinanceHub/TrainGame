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
    # ====== Old Man Pike (elder / quest giver) ======
    _dlg("Pike_01", "Pike_01", "NPCLine", "Old Man Pike",
         "You're new blood, or new enough. I've watched seventeen years of faces come and go in this car. Most don't last.",
         next_node="Pike_02"),
    _dlg("Pike_02", "Pike_02", "PlayerChoice", "", "",
         choices=[
             _choice("What happened to the ones who didn't last?", "Pike_03"),
             _choice("I'm not here for a history lesson, old man.", "Pike_04"),
             _choice("[Perception 3] You're testing me. Why?", "Pike_05", req_stat="Perception", req_stat_val=3),
         ]),
    _dlg("Pike_03", "Pike_03", "NPCLine", "Old Man Pike",
         "Taken for the drawers. Frozen alive and stacked like cordwood. Wilford calls it 'population management.' I call it murder in the cold.",
         next_node="Pike_06"),
    _dlg("Pike_04", "Pike_04", "NPCLine", "Old Man Pike",
         "Sharp tongue. Good. You'll need that. But listen anyway — ignorance kills faster than the cold out there.",
         next_node="Pike_06"),
    _dlg("Pike_05", "Pike_05", "NPCLine", "Old Man Pike",
         "Ha! Sharp eyes on you. Yes, I'm measuring your spine. We need people who see — really see. The revolution needs eyes as much as fists.",
         flag_to_set="Pike_Impressed", flag_value=True, next_node="Pike_06"),
    _dlg("Pike_06", "Pike_06", "NPCLine", "Old Man Pike",
         "There's a map. Hand-drawn by those who came before. It shows the first twenty cars — guard routes, supply caches, the works. But it's in pieces.",
         next_node="Pike_07"),
    _dlg("Pike_07", "Pike_07", "PlayerChoice", "", "",
         choices=[
             _choice("Where are the pieces?", "Pike_08"),
             _choice("Why should I risk my neck for a map?", "Pike_09"),
             _choice("[Charisma 4] I'll find it — if you share everything you know.", "Pike_10", req_stat="Charisma", req_stat_val=4),
         ]),
    _dlg("Pike_08", "Pike_08", "NPCLine", "Old Man Pike",
         "Scattered. Hidden in the walls, under bunks, sewn into clothes. Collect what you can. Bring the pieces to me and I'll make it whole. Now go — the Jackboots patrol every twenty minutes.",
         flag_to_set="Quest_LongWalk_Started", flag_value=True, next_node="Pike_End"),
    _dlg("Pike_09", "Pike_09", "NPCLine", "Old Man Pike",
         "Because without it, you're walking blind into a meat grinder. Every car forward is a new kind of danger. Your choice — but I thought you were smarter than that.",
         next_node="Pike_08"),
    _dlg("Pike_10", "Pike_10", "NPCLine", "Old Man Pike",
         "Deal. You bring me that map, and I'll tell you about the Engine. Things only I remember. Things Wilford would kill to keep buried.",
         flag_to_set="Pike_FullDeal", flag_value=True, next_node="Pike_08"),
    _dlg("Pike_End", "Pike_End", "End", "", ""),

    # ====== Kronole Dealer ======
    _dlg("Dealer_01", "Dealer_01", "NPCLine", "Kronole Dealer",
         "Psst. You look like you could use something to take the edge off. Kronole — pure stuff, not the garbage they cut with engine grease.",
         next_node="Dealer_02"),
    _dlg("Dealer_02", "Dealer_02", "PlayerChoice", "", "",
         choices=[
             _choice("How much?", "Dealer_03"),
             _choice("[Cunning 4] I know what Kronole really is. Who's your supplier?", "Dealer_04", req_stat="Cunning", req_stat_val=4),
             _choice("Get that poison away from me.", "Dealer_05"),
         ]),
    _dlg("Dealer_03", "Dealer_03", "NPCLine", "Kronole Dealer",
         "Two protein blocks per hit, or one favour. I always need runners — people who can slip through the gate when the guards change shift.",
         next_node="Dealer_End"),
    _dlg("Dealer_04", "Dealer_04", "NPCLine", "Kronole Dealer",
         "You... don't repeat that question. Not here. Fine — I'll tell you what I know, but you owe me. The Network remembers debts.",
         flag_to_set="Dealer_Persuaded", flag_value=True, next_node="Dealer_End"),
    _dlg("Dealer_05", "Dealer_05", "NPCLine", "Kronole Dealer",
         "Your loss. When the nightmares come — and they will — you know where to find me.",
         next_node="Dealer_End"),
    _dlg("Dealer_End", "Dealer_End", "End", "", ""),

    # ====== Jackboot Guard ======
    _dlg("Guard_01", "Guard_01", "NPCLine", "Jackboot Guard",
         "Back in line, Tailie. This gate stays shut. Mr. Wilford's orders.",
         next_node="Guard_02"),
    _dlg("Guard_02", "Guard_02", "PlayerChoice", "", "",
         choices=[
             _choice("[Strength 5] I wasn't asking permission.", "Guard_03A", req_stat="Strength", req_stat_val=5),
             _choice("[Cunning 3] I have something that might change your mind. (Offer bribe)", "Guard_03B", req_stat="Cunning", req_stat_val=3),
             _choice("Understood. I'll go.", "Guard_End"),
         ]),
    _dlg("Guard_03A", "Guard_03A", "NPCLine", "Jackboot Guard",
         "You want to try me? Fine. But there are six more behind that door, and they won't be as polite.",
         flag_to_set="Guard_Intimidated", flag_value=True, next_node="Guard_End"),
    _dlg("Guard_03B", "Guard_03B", "NPCLine", "Jackboot Guard",
         "...Make it quick. You've got thirty seconds on the other side. And if anyone asks, I never saw you.",
         flag_to_set="Guard_Bribed", flag_value=True, next_node="Guard_End"),
    _dlg("Guard_End", "Guard_End", "End", "", ""),

    # ====== Workshop Mechanic ======
    _dlg("Mechanic_01", "Mechanic_01", "NPCLine", "Workshop Mechanic",
         "Welcome to the Workshop — well, what we call a workshop. It's three square metres of bench space and a dream. Name's Josie.",
         next_node="Mechanic_02"),
    _dlg("Mechanic_02", "Mechanic_02", "PlayerChoice", "", "",
         choices=[
             _choice("Can you teach me to make weapons?", "Mechanic_03"),
             _choice("What kind of things can you build here?", "Mechanic_04"),
         ]),
    _dlg("Mechanic_03", "Mechanic_03", "NPCLine", "Workshop Mechanic",
         "Weapons, armour, tools — anything the Tail needs. Bring me scrap metal and cloth, I'll show you the basics. We start with a reinforced shiv.",
         next_node="Mechanic_05"),
    _dlg("Mechanic_04", "Mechanic_04", "NPCLine", "Workshop Mechanic",
         "Anything, if you've got the materials. Scrap metal, cloth, wire, tape — I can turn garbage into gear. It's what we do back here.",
         next_node="Mechanic_05"),
    _dlg("Mechanic_05", "Mechanic_05", "NPCLine", "Workshop Mechanic",
         "Here, take this workbench key. Bring me 3 Scrap Metal and 2 Cloth Strips, and we'll craft your first weapon together.",
         flag_to_set="Quest_Workshop_Started", flag_value=True, next_node="Mechanic_06"),
    _dlg("Mechanic_06", "Mechanic_06", "NPCLine", "Workshop Mechanic",
         "One more thing — the better materials you find as you move forward, the better gear we can make. Keep your eyes open in every car.",
         next_node="Mechanic_End"),
    _dlg("Mechanic_End", "Mechanic_End", "End", "", ""),

    # ====== Injured Tailie ======
    _dlg("Injured_01", "Injured_01", "NPCLine", "Injured Tailie",
         "*coughs* The Jackboots... they broke my ribs during the last count. Something's wrong inside. I can feel it.",
         next_node="Injured_02"),
    _dlg("Injured_02", "Injured_02", "PlayerChoice", "", "",
         choices=[
             _choice("What do you need?", "Injured_03"),
             _choice("I can't help you. I'm sorry.", "Injured_End"),
         ]),
    _dlg("Injured_03", "Injured_03", "NPCLine", "Injured Tailie",
         "Antiseptic. Real medicine, not the bark tea they give us. The Sickbay in Car 8 has stores — if you could get even one bottle...",
         flag_to_set="Quest_MedicineRun_Started", flag_value=True, next_node="Injured_04"),
    _dlg("Injured_04", "Injured_04", "NPCLine", "Injured Tailie",
         "Please... I've got a daughter in the children's section. She can't lose both parents. Not to this train.",
         next_node="Injured_End"),
    _dlg("Injured_End", "Injured_End", "End", "", ""),
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
    {
        "RowName": "Quest_LongWalkForward",
        "QuestID": "Quest_LongWalkForward",
        "QuestName": "The Long Walk Forward",
        "Description": "Reach Car 14 to establish contact with the Third Class Union. The revolution begins with a single step — through a hundred metres of hostile territory.",
        "State": 0,  # Available
        "bIsMainQuest": True,
        "Objectives": [
            {"ObjectiveID": "LWF_01", "Description": "Speak to Old Man Pike", "Type": OBJECTIVE_TYPE["Dialogue"], "TargetID": "Pike_01", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "LWF_02", "Description": "Collect 3 Revolution Map fragments", "Type": OBJECTIVE_TYPE["Collect"], "TargetID": "Item_RevolutionMap", "RequiredCount": 3, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "LWF_03", "Description": "Obtain the Gate Key", "Type": OBJECTIVE_TYPE["Collect"], "TargetID": "Item_GateKey", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "LWF_04", "Description": "Pass through the Tail Gate", "Type": OBJECTIVE_TYPE["GoTo"], "TargetID": "TailGate", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "LWF_05", "Description": "Reach Car 14 — Third Class Union Hall", "Type": OBJECTIVE_TYPE["GoTo"], "TargetID": "Car14_UnionHall", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
        ],
        "XPReward": 500,
        "ItemRewards": ["Item_Machete"],
        "FactionRepRewards": {"Tailies": 50, "ThirdClassUnion": 25},
        "PrerequisiteQuests": [],
    },
    {
        "RowName": "Quest_RatCatcher",
        "QuestID": "Quest_RatCatcher",
        "QuestName": "Rat Catcher",
        "Description": "The cook needs 5 pieces of rat meat to keep the Tail fed. The rats nest in the walls between Cars 1 and 3 — bring back what you can.",
        "State": 0,
        "bIsMainQuest": False,
        "Objectives": [
            {"ObjectiveID": "RC_01", "Description": "Collect 5 Rat Meat", "Type": OBJECTIVE_TYPE["Collect"], "TargetID": "Item_RatMeat", "RequiredCount": 5, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "RC_02", "Description": "Deliver the meat to the Cook", "Type": OBJECTIVE_TYPE["Interact"], "TargetID": "NPC_Cook", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
        ],
        "XPReward": 150,
        "ItemRewards": ["Item_ProteinBlock", "Item_ProteinBlock"],
        "FactionRepRewards": {"Tailies": 15},
        "PrerequisiteQuests": [],
    },
    {
        "RowName": "Quest_MedicineRun",
        "QuestID": "Quest_MedicineRun",
        "QuestName": "Medicine Run",
        "Description": "An injured Tailie is dying of infection. Find antiseptic in the Sickbay stores of Car 8 and bring it back before it's too late.",
        "State": 0,
        "bIsMainQuest": False,
        "Objectives": [
            {"ObjectiveID": "MR_01", "Description": "Speak to the Injured Tailie", "Type": OBJECTIVE_TYPE["Dialogue"], "TargetID": "Injured_01", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "MR_02", "Description": "Find Antiseptic in Car 8 Sickbay", "Type": OBJECTIVE_TYPE["Collect"], "TargetID": "Item_Antiseptic", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "MR_03", "Description": "Return the Antiseptic to the Injured Tailie", "Type": OBJECTIVE_TYPE["Interact"], "TargetID": "NPC_InjuredTailie", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
        ],
        "XPReward": 200,
        "ItemRewards": ["Item_Bandage", "Item_Bandage", "Item_Bandage"],
        "FactionRepRewards": {"Tailies": 25},
        "PrerequisiteQuests": [],
    },
    {
        "RowName": "Quest_KronoleCleanup",
        "QuestID": "Quest_KronoleCleanup",
        "QuestName": "Kronole Cleanup",
        "Description": "The Kronole dealer is poisoning the Tail. Deal with them — by force, persuasion, or by turning the Network against itself.",
        "State": 0,
        "bIsMainQuest": False,
        "Objectives": [
            {"ObjectiveID": "KC_01", "Description": "Confront the Kronole Dealer", "Type": OBJECTIVE_TYPE["Dialogue"], "TargetID": "Dealer_01", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "KC_02A", "Description": "(Optional) Defeat the Kronole Dealer in combat", "Type": OBJECTIVE_TYPE["Kill"], "TargetID": "NPC_KronoleDealer", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": True},
            {"ObjectiveID": "KC_02B", "Description": "(Optional) Persuade the Dealer to stop selling", "Type": OBJECTIVE_TYPE["Dialogue"], "TargetID": "Dealer_Persuade", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": True},
            {"ObjectiveID": "KC_03", "Description": "Report back to Old Man Pike", "Type": OBJECTIVE_TYPE["Dialogue"], "TargetID": "Pike_KronoleReport", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
        ],
        "XPReward": 250,
        "ItemRewards": ["Item_Kronole", "Item_Kronole"],
        "FactionRepRewards": {"Tailies": 20, "KronoleNetwork": -30},
        "PrerequisiteQuests": [],
    },
    {
        "RowName": "Quest_TheWorkshop",
        "QuestID": "Quest_TheWorkshop",
        "QuestName": "The Workshop",
        "Description": "The Tail's mechanic can teach you to craft weapons and gear. Gather materials and forge your first weapon.",
        "State": 0,
        "bIsMainQuest": False,
        "Objectives": [
            {"ObjectiveID": "WS_01", "Description": "Speak to the Workshop Mechanic", "Type": OBJECTIVE_TYPE["Dialogue"], "TargetID": "Mechanic_01", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "WS_02", "Description": "Collect 3 Scrap Metal", "Type": OBJECTIVE_TYPE["Collect"], "TargetID": "Item_ScrapMetal", "RequiredCount": 3, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "WS_03", "Description": "Collect 2 Cloth Strips", "Type": OBJECTIVE_TYPE["Collect"], "TargetID": "Item_ClothStrips", "RequiredCount": 2, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "WS_04", "Description": "Craft a Reinforced Shiv at the workbench", "Type": OBJECTIVE_TYPE["Custom"], "TargetID": "Craft_ReinforcedShiv", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
        ],
        "XPReward": 200,
        "ItemRewards": ["Item_Shiv"],
        "FactionRepRewards": {"Tailies": 10},
        "PrerequisiteQuests": [],
    },
    {
        "RowName": "Quest_ListeningPost",
        "QuestID": "Quest_ListeningPost",
        "QuestName": "Listening Post",
        "Description": "Scattered throughout Zone 1 are intel documents that reveal the Jackboots' patrol schedules and weak points. Find them all.",
        "State": 0,
        "bIsMainQuest": False,
        "Objectives": [
            {"ObjectiveID": "LP_01", "Description": "Find Intel Document 1 — Guard Roster", "Type": OBJECTIVE_TYPE["Collect"], "TargetID": "Collect_Intel_Z1_01", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "LP_02", "Description": "Find Intel Document 2 — Supply Manifest", "Type": OBJECTIVE_TYPE["Collect"], "TargetID": "Collect_Intel_Z1_02", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
            {"ObjectiveID": "LP_03", "Description": "Find Intel Document 3 — Encrypted Orders", "Type": OBJECTIVE_TYPE["Collect"], "TargetID": "Collect_Intel_Z1_03", "RequiredCount": 1, "CurrentCount": 0, "bCompleted": False, "bOptional": False},
        ],
        "XPReward": 300,
        "ItemRewards": ["Item_CommanderOrders"],
        "FactionRepRewards": {"Tailies": 30, "Jackboots": -20},
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
        "ETrainZone": {v: k for k, v in TRAIN_ZONE.items()},
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
        # Set the row struct — this requires the struct to be registered
        # We use unreal.find_struct() or the factory's Struct property
        try:
            factory.struct = unreal.ScriptStruct.find(struct_name)
        except Exception:
            # Fallback: Try setting by path
            log.warning("Could not find struct '%s' directly; trying path-based lookup.", struct_name)
            try:
                factory.struct = unreal.load_object(None, f"/Script/SnowpiercerEE.{struct_name}")
            except Exception as e2:
                log.error("Could not resolve struct %s: %s", struct_name, e2)
                raise

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
            "Zone": _enum_str("ETrainZone", c["Zone"]),
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
    log.info("Dialogue:     %d nodes (5 NPCs)", len(DIALOGUE_ZONE1))
    log.info("Quests:       %d quests", len(QUESTS))
    log.info("Collectibles: %d entries", len(COLLECTIBLES))
    log.info("Total:        %d data rows",
             len(ITEMS) + len(DIALOGUE_ZONE1) + len(QUESTS) + len(COLLECTIBLES))


if __name__ == "__main__":
    main()
else:
    # When executed via UE5's "Execute Python Script", __name__ is not "__main__"
    main()
