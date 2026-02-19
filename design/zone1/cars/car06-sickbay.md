# Car 06: The Sickbay

**Type:** Resource Management Introduction
**Difficulty:** None (no combat, resource puzzle)
**Estimated Play Time:** 10-15 minutes

## Narrative Context

The Tail's only medical facility — a grotesque understatement. Dr. Asha works with almost nothing: torn cloth for bandages, boiled water, a handful of salvaged pills, and sheer willpower. Disease is spreading. A respiratory illness has taken hold, and without real medicine, people will die. Asha knows things — she was Third Class before being demoted. She has intelligence the revolution needs.

## Layout

```
[DOOR ← Car 05]
|                                    |
|  [Triage area — floor mats]      |  Section A: Triage (12m)
|  [Patients lying on mats]         |
|  [IV drip stands — empty bags]    |
|  [Partition curtains on wire]     |
|                                    |
|-------- Curtain partition ---------|
|                                    |
|  [Examination table — steel slab] |  Section B: Treatment (12m)
|  [Dr. Asha's station]            |
|  [Medicine cabinet — padlocked]   |
|  [Boiling water station]          |
|  [Surgical tools — improvised]    |
|                                    |
|-------- Solid partition -----------|
|                                    |
|  [Quarantine — sealed area]       |  Section C: Quarantine (14m)
|  [3 patients, badly ill]          |
|  [Air vent — stuffed with cloth]  |
|  [Warning signs — crude biohazard]|
|                                    |
|-------- Locked cage door ----------|
|                                    |
|  [Asha's private storage/bunk]    |  Section D: Private (12m)
|  [Hidden intel documents]         |
|  [Locked chest — medicine reserve]|
|                                    |
[DOOR → Car 07]
```

## Environmental Details

### Section A: Triage
- 6-8 patients on floor mats. Various ailments: respiratory illness, infected wounds, malnutrition.
- Empty IV stands — a reminder of what medicine used to look like. Now decorative/aspirational.
- Partition curtains provide minimal privacy. They're stained.
- The air smells of antiseptic (Kronole-derived) and sickness.

### Section B: Treatment
- A steel slab serves as the exam table. Cold, unforgiving.
- Asha's station: a desk covered in notes, symptoms logs, patient records on scrap paper.
- **Medicine cabinet** — Padlocked. Contains the Tail's entire medicine supply: 3 doses of antibiotics (critical), bandages, fever reducers. Asha controls access absolutely.
- Boiling water station: a pot on a salvaged heating element. Sterilization.
- Surgical tools: sharpened scrap metal, repurposed kitchen implements. Functional but terrifying.

### Section C: Quarantine
- Sealed with plastic sheeting and tape. Three patients with the worst of the respiratory illness.
- Air vent stuffed with cloth to prevent spread (ineffective, but all they have).
- Crude biohazard warnings painted on the partition.
- One patient is delirious, muttering about "the cold coming in." Another is silent. The third calls for water.

### Section D: Private
- Asha's personal space behind a locked cage door (she has the only key).
- **Hidden documents** — Intelligence about Jackboot patrol patterns, stolen from a guard she treated. This is what Gilliam wants.
- **Locked chest** — A small reserve of medicine Asha keeps for emergencies. The player can see it but not access it (yet).

## Gameplay

### Resource Management Tutorial

The player learns that medicine is the most valuable currency in the Tail.

**Quest: "The Fever"** (from Gilliam, or discovered organically)
1. **Talk to Dr. Asha** — She's overwhelmed. Explains the respiratory illness. "I have three doses of antibiotics. Ten patients who need them. Do the math."
2. **The Triage Decision** — Asha asks the player to help decide who gets medicine:
   - **Patient A:** A child from the Nursery with high fever (emotional appeal)
   - **Patient B:** A fighter being prepped for the Blockade assault (strategic value)
   - **Patient C:** An elder with knowledge of the train's old ventilation systems (intelligence value)
   - Only 2 can be treated (3 doses, but one patient needs 2). The player recommends. Asha follows the recommendation.
3. **Deliver Intelligence** — Asha gives the player the Jackboot patrol documents for Gilliam in exchange for future medicine supply promises. Quest item: "Patrol Routes — Cars 9-15."

### Medicine Economy Introduction
- Player learns: medicine is scarce, has strategic value, and forces ugly choices.
- The UI introduces the "Medicine" resource counter.
- If the player has medicine (from Car 4 or elsewhere): can donate it. Asha remembers.

### Discoverable Content
- **Patient conversations** — Each conscious patient has a story. Worldbuilding.
- **Asha's notes** — Her desk papers reveal she was a real doctor in Third Class. Demoted for treating Tailies without authorization.
- **Quarantine observation** — Looking through the plastic sheeting. The delirious patient mentions "the drawers" — foreshadowing the cryogenics bay in Zone 6.

## Items

| Item | Location | Type |
|---|---|---|
| Patrol Routes (Cars 9-15) | Asha (quest reward) | Quest item — intel |
| Cloth Bandage (x2) | Triage area, loose | Medicine (minor heal) |
| Boiled Water Flask | Boiling station | Consumable (minor cold resist) |
| Asha's Medical Log | Desk, Section B | Lore |

## NPCs

| Name | Location | Role |
|---|---|---|
| **Dr. Asha** | Section B | The Tail's doctor. Intel contact. Resource management quest. |
| Patient A (child) | Section A | Triage decision target. |
| Patient B (fighter) | Section A | Triage decision target. Named: Rem. |
| Patient C (elder) | Section A | Triage decision target. Named: Volta. Has ventilation knowledge. |
| 3 Quarantine patients | Section C | Ambient. One delirious with dialogue. |
| ~3 ambulatory patients | Throughout | Ambient, barks. |

## Lighting

- **Section A:** Harsh overhead fluorescent. Clinical, unflattering. Every wound is visible.
- **Section B:** Focused. A salvaged desk lamp over the treatment table. Asha needs to see.
- **Section C:** Dim. Quarantine is deliberately kept dark (patients sleep more, conserves energy).
- **Section D:** Low. Private. A single LED.

## Audio

- **Ambient:** Coughing (constant, layered), labored breathing, water boiling, Asha's murmured instructions
- **Section C:** The delirious patient's muttering. Haunting.
- **Treatment:** Clinking of improvised instruments. Water pouring.
- **Emotional design:** The Sickbay should sound exhausting. Every sound is illness or futile effort.

## Revisit Behavior

- Asha remains available for healing (limited — uses medicine from player's inventory)
- The triage decision has visible consequences: healed patients recover and appear elsewhere; untreated patients worsen
- If Player C (Volta) was treated: she provides ventilation maps useful in later zones
- If Player B (Rem) was treated: he joins the Blockade assault as a capable fighter
- During revolution: Sickbay becomes a field hospital. Casualties arrive from forward cars.
