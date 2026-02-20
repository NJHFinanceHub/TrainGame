# Car 3: The Pit - Combat Basics

## Setting Context

Car 3 houses The Pit, the Tail's fighting ring. It's a sunken area in the center of the car, ringed by spectators, where Tailies settle disputes and earn extra rations through combat. Grey, the Pit Master, runs it with brutal fairness.

This is where the player learns to fight - not through a tutorial overlay, but because they need information that Grey will only give to someone who steps into the ring.

## The Combat Hook: "Grey's Price"

### Beat 1: Entering The Pit (0-1 minute)

**Context:** The player arrives looking for the missing child. Grey is watching a fight from his raised seat. He knows something - he sees everything in the Tail.

**Dialogue approach:**
- Grey: "Yeah, I saw the kid. Jackboots took her through here two days ago."
- Player asks for more details
- Grey: "Information costs. You want to know where they went? Get in the ring. Win, and I talks."
- Alternative (CHA 6): "I could just ask nicely." Grey laughs. "Alright - but you still fight. After." This bypasses the information gate but Grey still expects a fight for the practice room later.

**Design intent:** The combat tutorial is motivated by story need. Grey isn't a tutorial NPC - he's a character with his own logic (he respects fighters, not talkers).

### Beat 2: Pre-Fight Preparation (1-2 minutes)

**Context:** Before entering the ring, the player has a moment to prepare. Grey points to the corner: "Gear up if you've got anything. Ring rules: no killing. First to yield or drop."

**What the player learns:** Equipment, weapon readiness, pre-combat inventory.

**How they learn it:**
- If the player crafted the Scrap Shiv in Car 2, they can equip it now. The equipment interface opens naturally when examining the weapon: "Equip?" with a body-slot display showing where it goes
- If they didn't craft a weapon, Grey tosses them a pipe: "Can't have you going in bare." This ensures every player has a weapon for the tutorial fight regardless of previous choices
- Whisper (if present): "Watch your stamina. Swing too much and you'll gas out." (foreshadows stamina system)

**Companion behavior:** Whisper stays ringside. "This one's yours. I'll watch." (Teaches that some fights are solo - party composition matters.)

### Beat 3: First Fight - "The Proving" (2-5 minutes)

**Context:** The opponent is Dog, a Tailie who fights regularly. He's tough but predictable - an ideal first opponent.

**What the player learns:** Core combat mechanics, introduced one at a time through the fight's natural progression.

**Round 1: Basic Attacks (first 30 seconds)**

Dog approaches slowly. He doesn't attack immediately - giving the player time to figure out the attack input.

- Light attack: Quick swing, low damage, low stamina cost
- Dog blocks the first hit (teaches that enemies can block)
- Dog retaliates with a telegraphed swing (slow wind-up animation)
- The player instinctively dodges or blocks - both work

**Whisper callout (if present):** "He telegraphs everything. Watch his shoulder."

**Round 2: Stamina Management (30-60 seconds)**

After exchanging a few blows, stamina becomes a factor.

- If the player spams attacks, their stamina bar depletes visibly and attacks slow down
- Dog backs off when his stamina is low, demonstrating the mechanic through enemy behavior
- Recovery happens naturally during lulls in combat
- The stamina bar is the ONLY combat UI element visible by default (health bar appears only when damaged)

**Whisper callout:** "Breathe. You're swinging wild."

**Round 3: Blocking and Dodging (60-90 seconds)**

Dog becomes more aggressive in the second half.

- He throws a heavy attack (big wind-up, glowing weapon) that will do significant damage if it lands
- Blocking reduces damage but costs stamina
- Dodging avoids damage completely but requires timing
- If the player takes the hit, Grey shouts: "Block or move! Standing still kills you!"

**Round 4: Resolution**

- Dog yields when his health drops below 25%
- He goes to one knee: "Enough. You've got it."
- Grey nods approval. The crowd reacts.
- If the player continues attacking after Dog yields, Grey intervenes: "I said no killing. Fight's over." (Teaches ring rules and foreshadows that excessive violence has social consequences)

### Beat 4: Grey's Information (after first fight)

**Context:** Grey fulfills his bargain.

- He tells the player the Jackboots took the child forward through Car 4 toward the checkpoint at Car 5
- He also mentions: "The Kronole Den's between here and there. Watch yourself."
- This advances the main quest while directing the player toward Car 4

### Beat 5: The Practice Room (optional, 5-15 minutes)

**Context:** After the fight, Grey offers access to the practice room - a partitioned area at the back of The Pit with training equipment.

Grey: "You survived. Want to get better? Practice room's in the back. Use it whenever."

**What the practice room contains:**

**Training Dummy (always available):**
- A padded post for practicing attack combos
- Displays damage numbers when hit (the ONLY place damage numbers are shown explicitly - they're hidden in real combat unless enabled in settings)
- Teaches combo timing: light-light-heavy does bonus damage
- Whisper demonstrates a combo if present: "Like this. Quick-quick-hard."

**Formation Post (available if Whisper is in party):**
- Two standing positions marked on the floor
- Teaches party formation: front/back positioning in the narrow corridor
- Whisper takes position: "You go front. I'll cover your back."
- The player can swap positions, learning the formation toggle
- Demonstrates ranged-over-shoulder: Whisper throws a knife past the player's shoulder at the dummy

**Sparring Partner (repeatable):**
- A rotating set of Tailie fighters who spar for practice
- Each has a different style:
  - **Knuckles:** Pure brawler (teaches blocking priority)
  - **Slim:** Fast dodger (teaches attack timing)
  - **Martha's kid:** Defensive fighter (teaches aggression)
- Winning earns nothing - it's pure practice. Grey: "No stakes. Just learning."

**Non-Lethal Equipment Rack:**
- Displays blunted weapons, a choke cloth, knockout powder
- Interacting with them teaches that non-lethal options exist: "These won't kill. Sometimes that matters."
- Whisper: "Not everyone deserves to die. Remember that when we get past the gate."

## Combat System Details (as taught through The Pit)

### What The Pit Teaches

| Mechanic | How It's Taught | Where |
|----------|----------------|-------|
| Light/heavy attacks | Dog fight, Round 1 | Main ring |
| Stamina management | Dog fight, Round 2 | Main ring |
| Block/dodge | Dog fight, Round 3 | Main ring |
| Combo timing | Training dummy | Practice room |
| Party formation | Formation post | Practice room |
| Ranged support | Whisper demo | Practice room |
| Non-lethal options | Equipment rack | Practice room |
| Enemy variety | Sparring partners | Practice room |

### What The Pit Does NOT Teach (saved for later)

- Environmental combat (using the train's movement, smashing into walls) - introduced in Car 5 during a Jackboot encounter
- Advanced companion tactics (AI behavior toggles) - introduced when a second companion joins
- Weapon degradation - the practice room uses training weapons that don't degrade. First real degradation happens in Car 5 or Zone 2
- Crafting mid-combat - too advanced for early game; introduced in Zone 3

## Environment Design Notes

### The Ring as Theater

- Spectators react to the fight: cheering hits, gasping at near-misses
- This creates feedback the player can feel without UI elements
- A big hit gets a crowd roar; a dodge gets an "ooh"
- Losing doesn't end the game - Dog "wins" and Grey says "Try again when you're ready"

### Sound Design

- The ring has different acoustics than the rest of the car (more echo, crowd noise)
- Weapon impacts have distinct sounds: metal-on-flesh vs metal-on-block
- Stamina depletion is indicated by heavier breathing before the bar empties
- These audio cues supplement the minimal UI

### The Practice Room as Safe Space

- No consequences for failure in the practice room
- The player can return to it at any time during the Tail section
- It's the one place where the game explicitly supports experimentation
- Grey watches from outside and occasionally comments: "Better. Again."

## Stat Relevance in Combat

| Stat | Combat Effect | How Player Discovers It |
|------|--------------|------------------------|
| STR | Melee damage, stagger chance | Hitting harder with higher STR |
| END | Stamina pool, HP recovery | Lasting longer in the ring |
| CUN | Critical hit chance | Occasional bonus damage flashes |
| PER | Dodge timing window, initiative | Reacting faster to telegraphs |
| SRV | Wound recovery between fights | Healing faster after the bout |

These aren't explained directly. The player experiences them as "I hit harder" or "I dodge easier" and connects it to their stats over time. On Easy difficulty, post-fight tooltips note: "Your Strength increased your damage by 15%."

## Timing

The mandatory portion (Grey's fight) takes 3-5 minutes. The practice room is entirely optional and can occupy 5-15 minutes depending on player interest. Grey's information is available immediately after the first fight, so players who want to advance the story can leave quickly.
