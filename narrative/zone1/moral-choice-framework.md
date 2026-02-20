# Moral Choice System Framework

> No good choices. Only human ones.

---

## Design Philosophy

The moral choice system in Snowpiercer: Eternal Engine rejects the binary good/evil paradigm. There are no paragon/renegade meters. No karma score. Instead, the system tracks **consequences** — every choice ripples through the train, affecting factions, companions, resources, and ultimately which endings are available.

**Core Principles:**
1. **No choice is universally "right."** Every option costs something.
2. **Consequences are delayed.** The worst (or best) effects of a choice may not appear for zones.
3. **Context changes morality.** Stealing medicine is different when a child is dying.
4. **Inaction is a choice.** Refusing to decide is itself a decision with consequences.
5. **The player should feel the weight.** Choices should cause genuine hesitation.

---

## Choice Architecture

### Choice Tiers

**Tier 1: Ambient Choices (Every 1-2 Cars)**
Small decisions woven into exploration. Low narrative weight, cumulative effect.
- Share a ration with a hungry NPC? (+1 rep, -1 food)
- Report a broken rule? (+authority trust, -peer trust)
- Loot a corpse in front of witnesses? (+resources, -morale)

These don't trigger cutscenes or special UI. They're natural consequences of player behavior that the game tracks silently.

**Tier 2: Encounter Choices (Every 2-3 Cars)**
Structured decisions within encounters. Medium narrative weight, visible consequences.
- Pike's fate in Car 2
- Brakeman's arm in Car 3
- The Spy's Sword in Car 7
- Yuri's request in Car 8
- Nix's fate in Car 9
- The Forgotten in Car 10
- Frost rescue in Car 11
- Nix Two's radio in Car 13
- Volkov's fate in Car 15

These trigger a brief decision UI (options displayed, companion reactions shown) but don't pause exploration for long.

**Tier 3: Major Moral Choices (Every 3-5 Cars)**
Defining decisions that reshape the narrative. High narrative weight, far-reaching consequences.
- The Ration Dilemma (Car 4)
- Medicine Allocation (Car 6)
- The Kronole Question (Car 12)
- Gilliam's Secret (Car 14)

These trigger full cinematic presentation: close-up camera, companion reactions, a moment of silence before the choice appears. The game communicates: this matters.

**Tier 4: Zone-Defining Choices (Once per Zone)**
The final choice of a zone that sets the trajectory for the next. In Zone 1:
- The Revolt Strategy (Car 5: mass assault vs. small team)
- The Threshold Crossing (Car 15: final companion selection, point of no return)

---

## Consequence Tracking

### The Ledger System (Internal)

The game maintains a hidden "Ledger" — a weighted record of every meaningful choice. The Ledger doesn't track "good" vs. "evil" — it tracks patterns:

| Pattern | Tracked By | Affects |
|---------|-----------|---------|
| **Mercy vs. Pragmatism** | How the player treats defeated enemies | Which factions negotiate vs. fight |
| **Individual vs. Collective** | Whether the player sacrifices few for many | Companion loyalty and final ending options |
| **Truth vs. Stability** | Whether the player reveals or hides truths | NPC trust levels and narrative revelations |
| **Force vs. Cunning** | How the player solves problems | Which paths through cars are available |
| **Present vs. Future** | Whether the player sacrifices now for later | Resource availability and long-term faction standing |

### Consequence Timing

**Immediate (Same Car):** NPC reactions, companion approval/disapproval, resource gain/loss.

**Short-term (2-5 Cars):** NPC behavior changes, new encounters unlocked/locked, reputation shifts.

**Medium-term (Next Zone):** Faction relationships solidified, companion loyalty events trigger, alternative paths open/close.

**Long-term (Endgame):** Ending availability, final confrontation options, who stands with the player at the Engine.

---

## Zone 1 Major Choices — Detailed Consequence Maps

### Choice #1: The Ration Dilemma (Car 4)

**Context:** Children's rations are being cut. Josie asks for help.

| Option | Immediate | Short-term | Medium-term | Long-term |
|--------|-----------|------------|-------------|-----------|
| **Steal from Elders** | -Elder trust, children fed | Tension with Gilliam in Car 5, Council divided | Elder faction withholds information in Zone 2 | Player seen as someone who discards the old — affects Wilford confrontation |
| **Raid Jackboots** | Combat/stealth challenge, +all respect if success | Jackboot patrols increase (Car 9 harder), but Tailie morale soars | Third Class hears about the raid — some workers impressed | Player established as direct-action leader |
| **Share Own Rations** | -Health debuff (3 cars), +Josie trust, +Gilliam notices | Gilliam shares extra information, Josie's recruitment is easier | The self-sacrifice story spreads — NPCs in Zone 2 have heard of you | Player seen as Christ-figure — opens specific dialogue in the Cathedral (Zone 6) |
| **Do Nothing** | Full resources, -Josie trust, -morale | Josie harder to recruit, children visibly weaker | Word spreads you wouldn't help children — some NPCs hostile | Player seen as pragmatist — opens specific ruthless options later |

### Choice #2: Medicine Allocation (Car 6)

**Context:** 3 doses of antibiotics, 5 critical patients.

| Option | Immediate | Short-term | Medium-term | Long-term |
|--------|-----------|------------|-------------|-----------|
| **Save Children** | +Josie, +morale, Torres dies | -1 fighter for Blockade, children's parents grateful | Third Class medical car (Car 26) NPCs sympathetic | Player's "save the children" pattern noted — affects Child Labor Car (Zone 7) |
| **Save Fighters** | Children die, -Josie, +fighters | +3 fighters for Blockade, Josie may leave party | Third Class workers respect pragmatism | Player seen as utilitarian — Wilford uses this against them |
| **Triage (Asha's call)** | Neutral/balanced | Mixed results: some live, some die unpredictably | Asha trusts player to let experts decide — she shares more intel | Player established as delegator — faction leaders respect this |
| **Prevent Epidemic** | Cough patients cured, acute patients suffer | No epidemic subplot (saves time/resources), but visible suffering | Tail healthier overall, but those who died are mourned | Long-term survival of the Tail improved — affects how many Tailies survive to endgame |

### Choice #3: The Kronole Question (Car 12)

**Context:** Gilliam wants the den shut down. Zhora defends her people's choice.

| Option | Immediate | Short-term | Medium-term | Long-term |
|--------|-----------|------------|-------------|-----------|
| **Shut Down Den** | +Gilliam, -Zhora hostile, users in withdrawal | Fighters more alert for Threshold, but withdrawal deaths | No Kronole access, Zhora may sabotage efforts | Kronole Network faction hostile throughout game |
| **Keep Open** | -Gilliam, +Zhora allied, +Kronole access | Some fighters impaired, but Zhora provides explosives for Car 15 | Kronole Network friendly, smuggling routes available | Player accepted by the Network — opens alternate paths through Zones 3-5 |
| **Compromise** | Both sides grudging acceptance | Functional balance, no extreme gains or losses | Zhora cautiously allied, Gilliam cautiously disapproves | Moderate Kronole Network relationship — some benefits, not all |
| **Expand** | -Gilliam furious, +Zhora major ally, +users loyal | Fighters dulled, but player has massive Kronole supply | Kronole Network considers player a major partner | Deep Network alliance — unlocks the Network ending path |

### Choice #4: Gilliam's Secret (Car 14)

**Context:** The recording reveals Gilliam designed the class system and staged revolts.

| Option | Immediate | Short-term | Medium-term | Long-term |
|--------|-----------|------------|-------------|-----------|
| **Forgive** | +Gilliam's complete intel package | Gilliam becomes the player's most committed ally | Full train intelligence available — huge strategic advantage | Gilliam sacrifices himself in Zone 5/6 for the player — most impactful death in the game |
| **Condemn** | -Gilliam deposed, Council fractures | New Council leadership (player or Samuel), Tail morale unstable | Player must unify the Tail through personal authority | Player enters endgame as sole leader — lonely, powerful, parallels Wilford |
| **Keep Secret** | Gilliam guilt-driven loyalty | Council remains stable, Gilliam overcompensates with support | Gilliam's hidden guilt creates dramatic tension with Wilford later | At the Engine, Wilford reveals the truth anyway — player must decide again, with witnesses |
| **Tell Everyone** | Chaos, near-riot, Gilliam beaten | Player must personally prevent Gilliam's murder and hold Tail together | If player succeeds, massive trust boost — "they told us the truth and saved us" | Player enters endgame as a truth-teller — unlocks the Eternal Loop ending |
| **Destroy Recording** | Clean break, no one knows | Gilliam grateful but uneasy, player carries the weight | Information advantage: player knows Wilford's methods | At the Engine, Wilford has no leverage — player already knows the worst |

---

## Companion Loyalty System

Companions track trust independently of factions. Trust affects:
- **Dialogue depth** — Higher trust unlocks personal quests and backstory.
- **Combat effectiveness** — Trusted companions fight harder, use better tactics.
- **Critical moments** — At key points, companions may override the player's orders based on their own values. High trust means they'll follow even if they disagree. Low trust means they act independently.
- **Departure threshold** — If trust drops below a critical level, companions leave. This is permanent.

### Companion Moral Alignment (Hidden)

| Companion | Core Value | Approves | Disapproves |
|-----------|-----------|----------|-------------|
| **Josie** | Protection of the innocent | Saving children, mercy, truth | Sacrificing innocents, pragmatic cruelty, secrets |
| **Pike** | Survival and self-interest | Clever solutions, looting, avoiding fights | Reckless heroism, unnecessary sacrifice, blind idealism |
| **Brakeman** | Loyalty and honor | Mercy to opponents, protecting allies, directness | Betrayal, manipulation, abandoning people |
| **Frost** | Self-reliance and endurance | Tough calls, survival focus, respecting strength | Weakness, dependency, sentimentality |

**Conflict Points:** Companions will clash with each other when choices align with one's values and oppose another's. The player cannot please everyone.

---

## Ending Pathway Tracking

Each ending requires a pattern of choices to unlock. No single choice locks the player into an ending — it's the accumulation.

| Ending | Required Patterns | Key Choices |
|--------|------------------|-------------|
| **The New Wilford** | High pragmatism, willingness to sacrifice, accumulation of power | Condemned Gilliam, took the sword, expanded Kronole, turned Volkov |
| **The Revolution** | Balanced mercy and force, popular support, truth-telling | Forgave Gilliam, shared rations, compromised on Kronole, spared enemies |
| **The Derailment** | Desperation, loss of faith in the system, nihilism | Lost companions, destroyed the recording, shut down the Den, killed Volkov |
| **The Exodus** | Hope, evidence of thawing world, trust in the outside | Saved Frost, studied the outside in Car 11, listened to Glass's prophecy, kept Yuri's quest |
| **The Eternal Loop** | Pursuit of truth, discovery of the train's circular route | Told everyone about Gilliam, read all lore entries, questioned every system |
| **The Bridge (Secret)** | Maximum exploration, found all hidden intel, contacted outside | Used Nix's radio + Nix Two's comm + found all Listening Post intel |

**No ending is locked before Zone 4.** Early choices weight the probability but never eliminate options. The player should always feel they have agency, even if the consequences of their Zone 1 choices constrain their Zone 7 options.

---

## Anti-Patterns to Avoid

1. **The Obvious Good Choice** — If one option is clearly "right," redesign the choice. Every option must cost something real.
2. **The Info Dump Reward** — Don't use "moral" choices where one option gives gameplay information and the other doesn't. Both paths should be informationally valid.
3. **The Fake Choice** — If the outcome is the same regardless, it's not a choice. Every path must diverge.
4. **The Restart Trap** — Choices should be interesting on a first playthrough, not just on replay. The player shouldn't need to save-scum to make informed decisions.
5. **Moral Tourism** — Don't let the player be cruel without cost or kind without sacrifice. Every position has a price tag.
