# Car 1: Nursery - Movement & Interaction

## Setting Context

The game opens in the Nursery (Car 1), the rearmost car of the Tail. The player wakes to Gilliam's voice. The space is cramped, dark, and filled with sleeping bodies. This is home - the only home the player has ever known.

The Nursery teaches movement, camera control, object interaction, dialogue, and stat checks through a single morning routine that feels like narrative, not tutorial.

## Opening Sequence: "Wake Up"

### Beat 1: First Movement (0-30 seconds)

**Context:** The player's character is lying on a sleeping shelf. Gilliam speaks from nearby: "Get up. Council's called. Something's happening."

**What the player learns:** Basic movement.

**How they learn it:**
- The camera starts locked on the player's face, then pulls back to show the cramped shelf
- The only possible direction is "down" (off the shelf) - this constrains the first input to a single success state
- The narrow aisle between sleeping shelves channels movement forward toward Gilliam
- No prompt appears unless the player is idle for 8+ seconds (Easy) or 15+ seconds (Normal). On Hard, no idle prompt at all

**Environmental design:**
- Sleeping bodies block lateral movement, creating a natural corridor
- Dim blue emergency lighting draws the eye forward
- Gilliam's lantern provides a visible objective without a waypoint marker

### Beat 2: Camera and Look (30-60 seconds)

**Context:** As the player moves down the aisle, a child tugs their sleeve from a lower shelf: "Hey... is it ration day?"

**What the player learns:** Camera control, looking at objects/NPCs.

**How they learn it:**
- The child's voice comes from below and to the side, requiring the player to look down/turn
- Looking at the child highlights them subtly (slight outline glow, not a UI marker)
- The child's face is the first clear detail the player sees - establishing that looking at things reveals information

**Companion setup:** If the player looks around the Nursery instead of going straight to Gilliam, they spot Whisper watching from a dark corner - foreshadowing the companion recruitment later in this car.

### Beat 3: Object Interaction (1-2 minutes)

**Context:** Moving toward Gilliam, the player passes a water pipe with a slow drip and a tin cup beneath it. Gilliam: "Grab water if there's any. Long day ahead."

**What the player learns:** Interacting with world objects, inventory basics.

**How they learn it:**
- The tin cup glints in the lantern light (environmental attention cue)
- Approaching it reveals the interaction prompt (contextual, not permanent)
- Picking up the cup adds it to inventory - the inventory UI appears briefly showing the item, then fades
- The water pipe itself is a separate interaction: filling the cup requires holding the interact button (teaches hold-to-interact vs tap-to-interact)

**Scavenging seed:** A loose floor panel near the water pipe can be pried up (STR 3 check, trivially easy) to find a scrap of cloth. This teaches: objects in the environment can be searched, and some require stat checks.

### Beat 4: Dialogue System (2-4 minutes)

**Context:** The player reaches Gilliam at the front of Car 1. He briefs them on the situation: a child has gone missing, possibly taken forward.

**What the player learns:** Dialogue trees, stat-gated dialogue options, tone selection.

**How they learn it:**
- Gilliam's briefing plays as a short conversation with 2-3 response choices
- One option is always available (neutral response)
- One option requires CHA 4 (very low gate - most players pass): "Who took the kid, Gilliam? You know something." - teaches that some dialogue requires stats
- One option requires STR 6 (moderate gate - some players fail): "I'll break through the gate myself" - if failed, Gilliam responds "You couldn't. Not alone." This teaches that failing a check isn't game over; it just closes one path
- The stat requirement is shown next to the option: `[CHA 4]` or `[STR 6]` - this is the one concession to UI clarity over pure immersion, because players must understand stat gating early

**Key dialogue design:**
- Failed checks still produce meaningful dialogue (Gilliam's response acknowledges the attempt)
- The conversation naturally introduces the quest hook (find the missing child) without a quest log popup
- Gilliam mentions "talk to Whisper - she sees everything" - directing the player to their first companion

### Beat 5: Companion Introduction & Recruitment (4-6 minutes)

**Context:** Whisper is in the back corner of Car 1, watching. She's been observing the player since they woke up.

**What the player learns:** Companion dialogue, recruitment, party formation basics.

**How they learn it:**
- Whisper initiates conversation when approached: "You're looking for the kid. I saw which way they went."
- She offers to come along - the player can accept or decline
- If accepted, a brief "Whisper joined your party" notification appears (the only UI notification in Car 1)
- Whisper immediately demonstrates companion behavior by commenting on the environment as they move: "Watch the third step - it's rusted through" (teaches that companions provide useful information)
- If declined, Whisper follows at a distance anyway and can be recruited later

**Whisper as tutorial voice:** From this point forward, Whisper provides contextual hints in-character. She never says "press X to do Y." She says things like:
- "That panel's loose. Bet you could pry it open." (interaction hint)
- "Smells like chemicals. Careful." (environmental hazard warning)
- "Quiet. Jackboots ahead." (enemy awareness)

### Beat 6: First Stat Check Gate (5-7 minutes)

**Context:** The exit from Car 1 to Car 2 is partially blocked by a collapsed support beam.

**What the player learns:** Stat checks as environmental obstacles, multiple solutions.

**How they learn it:**
- **STR 5:** Push the beam aside (brute force)
- **CUN 4:** Find the maintenance hatch underneath (alternative path)
- **PER 3:** Notice the beam is only resting on debris; kick the debris out (easiest check)
- If all checks fail (very unlikely with starting stats), Whisper helps: "Together?" - and the combined effort succeeds. This teaches that companions can help with checks and that the game won't hard-lock you

**Design intent:** The player's first real obstacle has three solutions plus a failsafe. This teaches the multi-solution philosophy immediately without explaining it. The player simply experiences that there's more than one way through.

## Environment Design Notes

### Lighting as Guidance
- Gilliam's lantern is the brightest point in Car 1, drawing the player forward
- Interactive objects have subtle reflective highlights (metal glint, wet surface)
- Dark corners reward exploration (Whisper's location, hidden scrap)

### Sound as Teacher
- The water drip draws attention to the interactive cup before the player sees it
- Whisper's breathing/movement is audible before she's visible, teaching the player to listen
- The train's constant rumble and sway establishes the baseline - deviations from it (a creak, a clank) signal interactable elements

### Spatial Constraints
- Car 1 is narrow (3.2m wide) and cluttered with sleeping shelves
- This forces linear progression through the tutorial beats while feeling natural
- The "corridor" nature of the train justifies the linear onboarding without it feeling like a tutorial hallway

## Stat Check Summary (Car 1)

| Check | Stat | Value | Context | Fail State |
|-------|------|-------|---------|------------|
| Gilliam dialogue | CHA | 4 | Press for information | Neutral response available |
| Gilliam dialogue | STR | 6 | Volunteer for action | Gilliam redirects |
| Floor panel | STR | 3 | Scavenge scrap | Missed loot (minor) |
| Exit beam | STR | 5 | Push through | Two alternatives + companion failsafe |
| Exit beam | CUN | 4 | Find hatch | Two alternatives + companion failsafe |
| Exit beam | PER | 3 | Spot weakness | Two alternatives + companion failsafe |

## Timing

Car 1 should take 5-10 minutes for a thorough player, 3-5 minutes for someone rushing. No mechanic is gated behind another - a player who skips the water cup still reaches Gilliam, still meets Whisper, still exits to Car 2.
