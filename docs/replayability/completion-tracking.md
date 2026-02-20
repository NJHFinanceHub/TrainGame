# Completion Tracking, Chapter Select & Road-Not-Taken Indicators

## Completion Percentage System

### Overview

Completion percentage tracks cumulative progress across all playthroughs. It measures what the player has *ever* done, not what they did in a single run. This rewards replay — reaching 100% requires multiple playthroughs with different builds, choices, and companion configurations.

### Completion Categories

| Category | Weight | Max Points | Details |
|----------|--------|------------|---------|
| **Main Story** | 15% | 150 | Complete all 7 zones (always 100% in a finished run) |
| **Moral Choices — Coverage** | 15% | 150 | Experience all options across 24 choices (requires multiple runs) |
| **Companions** | 15% | 150 | Recruit, bond, and complete personal quests for all 12 |
| **Exploration** | 15% | 150 | Fully explore all 103 cars |
| **Collectibles** | 10% | 100 | Find all journal pages, memory fragments, lore documents, Thaw symbols, revolution evidence |
| **Combat & Stealth** | 10% | 100 | Defeat all bosses in combat AND resolve all non-violently (requires 2+ runs minimum) |
| **Crafting & Economy** | 5% | 50 | Learn all recipes, craft all items, find all blueprints, earn all merchant favors |
| **Endings** | 10% | 100 | Achieve all 6 endings (including 2 secret endings) |
| **Skill Mastery** | 5% | 50 | Unlock all 75 base skill nodes + 5 NG+ T4 nodes |

**Total: 1000 points = 100%**

### Completion Milestones

| Percentage | Title Awarded | Unlock |
|------------|---------------|--------|
| 25% | *Passenger* | Gallery mode (concept art viewer) |
| 50% | *Conductor* | Soundtrack player (in-game jukebox at rest points) |
| 75% | *Veteran* | Commentary mode (developer notes at key locations) |
| 90% | *Historian* | The full Hidden Ledger calculator (experiment with hypothetical choice combinations) |
| 100% | *True Conductor* | Golden train skin for main menu, NG+ title, all "Road Not Taken" content fully revealed |

### What Counts Per Category

**Moral Choices — Coverage (150 points):**
- Each of the 24 moral choices has 3–4 options
- Each option experienced (across any playthrough) earns points
- Formula: (unique options experienced / total options available) × 150
- Estimated total unique options: ~88 across all 24 choices
- A single playthrough experiences 24 options → ~27% of this category
- Full coverage requires ~4 playthroughs with deliberately different choices

**Companions (150 points):**
- Recruit each companion: 5 pts × 12 = 60
- Complete full personal quest: 5 pts × 12 = 60
- Reach Bonded loyalty: 2.5 pts × 12 = 30
- Points are cumulative across playthroughs (recruit Marta in run 1, Edgar in run 2 — both count)

**Exploration (150 points):**
- Each car has a "fully explored" checklist: all loot containers opened, all NPCs interacted with, all environmental events triggered
- 150 / 103 ≈ 1.46 points per car
- Some cars have NG+-exclusive content (e.g., Car 52 hidden room) — these are required for 100%

**Combat & Stealth (100 points):**
- Defeat each boss in combat: ~7 pts × 7 = 49
- Resolve each boss non-violently: ~7 pts × 7 = 49
- Ghost run (zero kills, zero detections): 2 bonus pts
- These are tracked across playthroughs — defeat Grey in combat in run 1, resolve him non-violently in run 2, both count

---

## Chapter / Zone Select

### Unlock Condition

Chapter Select unlocks after completing the main story once (any ending). Available from the main menu.

### How It Works

- The player selects a starting zone (1–7)
- A pre-built character state is generated based on the player's most recent full playthrough at the appropriate level range for that zone
- Alternatively, the player can import their exact character state from any completed save at the zone's starting checkpoint

### Zone Select Options

| Zone | Starting Level | Starting Car | Companions Available |
|------|---------------|--------------|---------------------|
| **Zone 1: The Tail** | 1 | Car 1 | None (Marta + Edgar recruited during zone) |
| **Zone 2: Third Class** | 6 | Car 16 | Up to 2 from Zone 1 roster |
| **Zone 3: Second Class** | 11 | Car 31 | Up to 4 from Zones 1–2 roster |
| **Zone 4: Working Spine** | 16 | Car 46 | Up to 6 from Zones 1–3 roster |
| **Zone 5: First Class** | 21 | Car 63 | Up to 8 from Zones 1–4 roster |
| **Zone 6: The Sanctum** | 26 | Car 83 | Up to 10 from Zones 1–5 roster |
| **Zone 7: The Engine** | 30 | Car 103 | Narrative only (no combat) |

### Character Import vs. Pre-Built

**Import Mode:**
- Uses the exact character state (stats, skills, perks, inventory, companions, faction rep, Ledger scores) from the selected save at the zone transition checkpoint
- Allows replaying specific zones to try different approaches while preserving prior context
- Moral choice consequences from prior zones are preserved

**Pre-Built Mode:**
- Generates a character with balanced stats appropriate for the zone's level range
- Default neutral faction reputations
- Default companion roster (all available companions at Neutral loyalty)
- Default Ledger scores (all axes at 0)
- Useful for quickly experimenting with a zone's content without prior context

### Chapter Select Limitations

- Progress in Chapter Select does NOT contribute to main save completion percentage
- Chapter Select has its own separate save slots (3 slots)
- Moral choices made in Chapter Select do not affect the main Ledger
- Chapter Select is for experimentation and content access, not progression

---

## Road-Not-Taken Indicators

### In-Game Indicators (During Play)

Subtle visual cues during gameplay that hint at paths the player didn't take. These appear only in NG+ or Chapter Select replays — never on a first playthrough.

#### Types of Indicators

**1. Ghost Echoes (Visual)**
At locations where a major moral choice was made differently in a previous playthrough, a faint visual shimmer appears (like heat distortion). Approaching it triggers a 2-second audio whisper of the unchosen option's key dialogue line.

- Example: At the ration distribution point in Car 8, if the player previously chose to feed the children but is now hoarding supplies, they hear a ghostly echo of Timmy's voice: "Thank you... I was so hungry."
- Only appears for choices made differently from the previous playthrough
- Can be disabled in settings under "NG+ Features"

**2. Scrawled Notes (Environmental)**
In NG+, new environmental details appear near moral choice locations — scratched notes, chalk marks, or graffiti that reference the unchosen paths:

- A chalk tally near the medicine allocation point showing the count of patients who could have been saved
- A scratched name on a wall where a companion died — visible only if that companion survived in the current run (or vice versa)
- Wilford's handwriting in the margins of documents the player didn't read or chose to destroy

These are collectible and count toward the "Revolution Evidence" collectible category in NG+.

**3. NPC Awareness (Dialogue)**
In NG+, the 6 "echo NPCs" (one per zone) deliver lines referencing roads not taken:

| Zone | NPC | Example Line |
|------|-----|-------------|
| 1 | Blind Yuri | "In the version where you saved that child, she grew up to lead a whole section of the revolution." |
| 2 | A Third Class Elder | "There's a world where the strike succeeded without violence. Can you imagine?" |
| 3 | A Second Class Clerk | "The trial had another outcome, once. The law held. People believed in it." |
| 4 | A Spine Worker | "Those children in the Engine... in another life, you got them out sooner." |
| 5 | A First Class Servant | "Beaumont kept her word, in one version. Surprising what mercy earns." |
| 6 | An Order Acolyte | "Ezra listened, once. Really listened. It changed everything." |

These NPCs are clearly marked with a subtle icon (a forking path symbol) so the player can find them intentionally.

**4. The Codex — Roads Not Taken Tab**

After first completion, the Codex (in-game encyclopedia) gains a "Roads Not Taken" tab:

- Lists all 24 moral choices
- Shows which option was chosen in each playthrough
- For unchosen options: reveals a brief hint about consequences (vague on first completion, detailed after NG+)
- Progress bar showing what percentage of total unique options have been experienced
- Highlights which options unlock specific endings or companion outcomes

### Post-Game Road-Not-Taken (Statistics Screen)

See `post-game-statistics.md`, Page 8 for the full post-game "Road Not Taken" breakdown.

---

## Design Principles

1. **Cumulative, not repetitive.** Completion percentage rewards breadth across playthroughs, not grinding a single run. Each new playthrough should meaningfully increase the percentage.
2. **Chapter Select is a sandbox.** It exists for experimentation and content access. It doesn't affect main progression to prevent exploitation.
3. **Road-Not-Taken respects first impressions.** No indicators appear on a first playthrough. The initial experience is pure and unspoiled. Only after completion do the hints emerge.
4. **Reveal scales with investment.** Players who replay more get more detailed information about unchosen paths. Casual players get vague hints; dedicated players get full breakdowns.
5. **100% is aspirational, not expected.** Reaching 100% requires 4+ playthroughs with deliberately varied approaches. It should feel like a significant achievement, not a checklist chore.
