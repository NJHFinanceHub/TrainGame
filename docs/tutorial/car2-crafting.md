# Car 2: Sickbay - Crafting Introduction

## Setting Context

Car 2 is the Tail's Sickbay, run by Asha. It's a cramped medical station where the injured and sick are tended with almost nothing. The space is organized around necessity: bandages made from rags, splints from pipe fragments, antiseptic from distilled Kronole.

This environment teaches crafting because crafting IS medicine here. The player doesn't learn crafting as an abstract system - they learn it because someone is hurt and the only way to help is to make something.

## The Crafting Hook: "Asha Needs Help"

### Beat 1: Arriving in the Sickbay (0-1 minute)

**Context:** The player enters Car 2 looking for information about the missing child. Asha is treating a burn victim and can't talk yet.

**What happens:**
- Asha: "Not now - hands full. If you want to help, I need bandages. The workbench is in the back. There's cloth and alcohol somewhere."
- This gives the player a clear objective (make bandages) while keeping the main quest active (Asha has info but can't share it yet)
- The player is directed to the workbench without a waypoint - "in the back" is enough in a 3.2m-wide car

**Whisper's role:** If present, Whisper says: "I've seen her make those before. Cloth strips and a bit of alcohol. The workbench has what you need." This provides ingredient hints without a recipe popup.

### Beat 2: Scavenging for Materials (1-3 minutes)

**Context:** The Sickbay has materials scattered around it - cloth rags on shelves, a bottle of distilled alcohol under a cot, scrap metal in a corner.

**What the player learns:** Scavenging, material identification, inventory management.

**How they learn it:**
- Interactive materials have the same subtle highlight as the tin cup in Car 1 (consistent visual language)
- Picking up materials adds them to inventory with a brief material-type indicator: "Cloth Scraps (x2)", "Distilled Alcohol (x1)"
- The Sickbay has more materials than needed for the bandages - extra cloth, a shard of metal, a chemical residue. This teaches that scavenging yields varied materials, some useful now, some useful later
- Whisper comments on specific items: "That metal shard could be useful. Hold onto it." (foreshadows weapon crafting in Car 3)

**Material placement:**
- **Obvious:** Cloth rags hanging from a shelf near the workbench (hard to miss)
- **Moderate:** Alcohol bottle under a cot (requires looking around)
- **Hidden:** Chemical residue in a cracked jar on a high shelf (PER 4 to notice) - not needed for bandages but useful later

### Beat 3: The Workbench (3-5 minutes)

**Context:** The Improvised Workbench is a plank balanced on crates with basic tools scattered around it.

**What the player learns:** Workbench interaction, recipe discovery, crafting execution.

**How they learn it:**

**Step 1 - Approach and interact:**
- Interacting with the workbench opens the crafting interface
- The interface is minimal: a work surface showing available materials on the left, a central area for combining, and discovered recipes on the right (currently empty)

**Step 2 - Recipe discovery:**
- With cloth and alcohol in inventory, the "Improvised Bandage" recipe auto-discovers
- The recipe appears with required materials highlighted: Cloth Scraps (1) + Distilled Alcohol (1)
- Whisper (if present): "Tear the cloth, soak it. That's all there is to it."
- On Easy difficulty, a brief tooltip explains: "Recipes are discovered when you have the right materials at a workbench"

**Step 3 - Crafting execution:**
- Selecting the recipe and confirming begins a brief crafting animation (2-3 seconds)
- The player's CUN stat affects quality: CUN 5+ produces "Clean Bandage" (better healing), CUN < 5 produces "Rough Bandage" (still works, less effective)
- This stat-quality relationship is shown subtly: the item description mentions "carefully prepared" or "hastily wrapped"
- On Easy/Normal, a tooltip notes: "Higher Cunning improves crafting quality"

**Step 4 - Delivery:**
- Bringing the bandage to Asha completes the task
- She applies it to the patient and then talks: provides information about the missing child, mentions hearing movement toward Car 3
- If the player crafted a high-quality bandage, Asha gives them an extra ration as thanks (teaches that quality matters for rewards)

### Beat 4: Optional Crafting Exploration (variable)

**Context:** The workbench remains available. Extra materials enable additional recipes.

**What the player can discover:**
- **Scrap Shiv** (Metal Shard + Cloth Scraps): A basic weapon, foreshadowing combat in Car 3. Recipe auto-discovers if player has both materials. Whisper: "Wouldn't want to face The Pit without something sharp."
- **Antiseptic Solution** (Distilled Alcohol + Chemical Residue): A healing consumable. Only available if player found the hidden chemical residue (PER 4 check in Beat 2)

**Design intent:** These optional crafts reward exploration and scavenging. The Shiv directly prepares the player for Car 3's combat tutorial. Neither is required to progress.

## Companion as Crafting Guide

Whisper's dialogue throughout Car 2 serves as an organic crafting tutorial:

| Player Action | Whisper Says | Mechanic Taught |
|---------------|-------------|-----------------|
| Picks up first material | "Good. Scavenge everything - nothing's worthless down here." | Scavenging habit |
| Approaches workbench | "Asha's bench. She makes medicine from scraps. You can too." | Workbench purpose |
| Has materials but doesn't craft | "You've got what you need. Try the bench." | Gentle nudge |
| Crafts first item | "Not bad. Better materials, better results." | Quality system |
| Examines extra materials | "That metal shard could be useful. Hold onto it." | Material value |
| Tries to leave without helping | "Asha still needs those bandages. Wouldn't take long." | Quest reminder |

If Whisper was not recruited, Asha herself provides briefer versions of these hints, or the player must figure it out from environmental context alone.

## Environment Design Notes

### The Workbench as Diegetic UI

The workbench is a physical object in the world. The crafting interface is presented as the player looking down at the bench surface. Materials are physically placed. This maintains immersion - the player is making something, not navigating a menu.

### Sickbay Atmosphere

- Moaning patients provide urgency (help Asha quickly)
- Medical supplies are visibly scarce (one shelf, half-empty) - reinforces resource scarcity
- A child with a bandaged arm watches the player craft - this is one of the missing child's friends, providing a quest clue if spoken to: "They took her toward the loud place" (The Pit, Car 3)

### Teaching by Contrast

- Asha's existing work (crude splints, reused bandages) shows what crafting produces
- The before/after of the burn patient (suffering → treated) demonstrates crafting's tangible impact
- This emotional framing makes crafting feel important rather than mechanical

## Stat Check Summary (Car 2)

| Check | Stat | Value | Context | Fail State |
|-------|------|-------|---------|------------|
| Hidden chemical | PER | 4 | Find extra material | Missed optional material |
| Bandage quality | CUN | 5 | Better crafting result | Lower quality item (still works) |

## Timing

Car 2 should take 5-8 minutes for a thorough player (scavenging everything, crafting extras), 3-4 minutes if focused on the main objective (bandage for Asha, get info, move on).
