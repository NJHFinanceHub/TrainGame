# Frozen Landmarks

Landmarks are fixed exterior set-pieces tied to route segments. Each landmark is visible
for a limited real-time window as the train passes. Observation quality depends on weather,
time of day, player position (window vs. rooftop), and story zone progression (thaw state
modifies what's visible).

## Landmark Observation System

### Viewing Methods

| Method | Detail Level | Risk | Unlock |
|---|---|---|---|
| **Interior window** | Low-Medium. Frost, glass distortion, limited angle. | None | Default |
| **Observation lounge** | Medium-High. Panoramic glass, heated seating. | Social (need access/disguise) | Zone 3+ |
| **Rooftop** | Maximum. 360° unobstructed view. | Cold exposure, wind, tunnel risk | Cold Suit required |
| **Telescope** (First Class) | Maximum + zoom. Architectural details visible. | Social (First Class access) | Zone 5 |

### Observation Windows

Each landmark has a pass-by duration based on train speed and landmark size. Players
receive a subtle audio cue (chime + companion comment) 30 seconds before a landmark
enters viewing range. Landmarks missed can be seen on the next circuit (New Game+) or
from rooftop at extended range.

## Major Landmarks

### The Frozen City (European Segment — Leg 2)

- **Pass-by duration**: 8 minutes
- **Viewing cars**: 10-15 (Tail/Third Class), 40-45 (Second Class)
- **Description**: A major European metropolis buried under 30-40 meters of ice and
  compacted snow. Only the upper floors of the tallest buildings protrude. Glass towers
  are shattered, steel frames bent under ice weight. A cathedral spire is the highest
  visible point, its cross still catching light.
- **Thaw progression**:
  - Zone 1-2: Monochrome white mounds. Shapes barely distinguishable as buildings.
    Wind-driven snow obscures details.
  - Zone 3-4: Ice has settled. Building silhouettes sharper. Dark window cavities
    visible in upper floors. A billboard frozen mid-peel is legible: an advertisement
    for CW-7 ("A Cooler Tomorrow").
  - Zone 5-6: Lower floors exposed. Street-level ice cracking. Dark patches where
    meltwater has pooled and refrozen. A collapsed pedestrian bridge hangs at an angle.
  - Zone 7: Standing water at street level. A flock of birds circles the cathedral
    spire. The CW-7 billboard has fallen.
- **Rooftop bonus**: From rooftop with telescope perk (PER 14+), player can spot a
  figure in a high window—a mannequin, or possibly a frozen body positioned at a desk.
  Unlocks journal entry: "Someone stayed."
- **NPC reactions**: Tailies who remember cities weep. Children born on the train stare
  in confusion—they have no frame of reference for buildings.

### Yekaterina Bridge (Ural Mountains — Leg 2)

- **Pass-by duration**: 3 minutes (train slows to 40 km/h for the crossing)
- **Viewing**: All cars (train crosses directly)
- **Description**: A massive steel railway bridge spanning a 200-meter gorge. The gorge
  walls are sheer ice-covered rock. Below, a frozen river is visible as a pale ribbon.
  The bridge itself is reinforced with Wilford's engineering—welded plates, de-icing
  coils (powered by the Engine), and emergency rail clamps.
- **Thaw progression**:
  - Zone 1-4: Gorge filled with ice. River invisible. Bridge coated in rime.
  - Zone 5-6: Ice in gorge has thinned. Dark water visible beneath translucent ice
    in the deepest section. Icicles on bridge struts are shorter.
  - Zone 7: River flowing. Water sounds audible through hull at slow speed. Ice
    remaining only on north-facing gorge wall.
- **Gameplay event**: During one crossing per playthrough (randomized), the bridge
  groans and shifts 2cm laterally. The entire train lurches. Unsecured objects fall.
  NPCs panic briefly. The Tail feels it worst (end-of-train whip effect). If on
  rooftop during the event: QTE to grab railing or be thrown toward the edge. Failing
  the QTE doesn't kill—player catches the rail at the last second, taking 15% HP
  damage and gaining the "Bridge Survivor" journal entry.
- **Lore**: Pre-Freeze, this was the Trans-Siberian Railway's most dramatic crossing.
  Wilford reinforced it in Year 1 after cracks appeared. Maintenance crews (Third
  Class) inspect it every circuit. A maintenance log in Car 52 notes the bridge has
  shifted 47cm total since Year 1—it won't last forever.

### The Dead Trains (European Segment — Leg 2)

- **Pass-by duration**: 4 minutes
- **Viewing cars**: 60-65 (Working Spine/First Class boundary)
- **Description**: A train graveyard. Three derailed trains lie on parallel tracks,
  progressively buried in snow. The nearest is a passenger train, doors frozen open,
  windows dark. The second is a freight train, containers scattered like fallen
  dominoes. The third is partially obscured—only its engine visible, nose-down in a
  snowdrift.
- **Thaw progression**:
  - Zone 1-3: Shapes under snow. Could be anything.
  - Zone 4-5: Paint visible on nearest train. A national railway logo. Doors frozen
    mid-slide. Dark shapes inside the passenger cars (luggage? bodies?).
  - Zone 6-7: Full exposure. The passenger train's interior visible—seats, belongings,
    a child's backpack pressed against a window. The freight containers have split open,
    spilling consumer goods now frozen solid: electronics, clothing, canned food.
- **Lore significance**: These trains attempted to flee south when CW-7 hit. They ran
  out of fuel, or the tracks froze, or the switches failed. Their passengers had hours,
  maybe a day, before the cold took them. Snowpiercer passes them every circuit as a
  reminder.

### The Geothermal Vent (South American Segment — Leg 5)

- **Pass-by duration**: 6 minutes
- **Viewing cars**: 50-55 (Working Spine)
- **Description**: A fissure in a mountainside where steam rises continuously. The
  surrounding rock is dark and bare—no ice within 200 meters of the vent. Mineral
  deposits (yellow-white sulfur, orange iron oxide) color the rock around the opening.
  The steam plume rises 50-100 meters before dispersing.
- **Thaw progression**:
  - Zone 1-3: Steam visible but surroundings still ice-covered except the immediate
    vent area. The bare rock patch is ~50 meters diameter.
  - Zone 4-5: Bare rock expanded to ~150 meters. Dark soil visible. Patches of moss
    or lichen on south-facing rocks (first visible plant life).
  - Zone 6-7: Vegetation ring around vent: moss, grass, small shrubs. Steam plume
    thinner (less temperature differential as ambient warms). Insects visible as
    dark specks in the warm air column.
- **Lore significance**: First hard evidence that the Earth retains internal heat.
  Cold Lab scientists in the Working Spine track this vent's activity across circuits.
  Their data shows the bare-rock radius expanding—proof of warming. This is the
  physical evidence behind Sparks' Year 12 broadcast.
- **Rooftop bonus**: From rooftop, the warm air is physically felt—cold suit timer
  pauses for the 6-minute pass-by. The temperature display spikes to -12°C. The only
  time exterior temperature rises above -20°C before Zone 7 content.

### The Green Valley (Late-Game — Leg 5/6 Boundary)

- **Pass-by duration**: 10 minutes
- **Viewing cars**: 80-85 (First Class)
- **Description**: A sheltered valley between two mountain ranges where microclimatic
  conditions have allowed vegetation recovery. Only visible from Zone 5 onward in the
  story progression (earlier zones: the valley is snow-covered and indistinguishable
  from surroundings).
- **Thaw progression**:
  - Zone 5: Faint green tinge on south-facing slopes. Could be mineral discoloration.
    Debatable among NPCs.
  - Zone 6: Unmistakable green. Grass on lower slopes. Bare deciduous trees with
    visible bud formations. A stream (liquid water!) running through the valley
    floor, catching sunlight.
  - Zone 7: Full vegetation. Coniferous trees with green needles. Grass meadow.
    A herd of animals (deer or similar ungulates) visible as brown shapes on the
    valley floor. Birds circling above. The stream has widened to a small river.
- **Emotional significance**: The single most important visual in the game. This is
  undeniable proof that the world can support life. Characters react viscerally.
  Companions stop talking. The train falls quiet. Tailies who've never seen green
  don't have words for it. First Class passengers weep. The player's journal auto-
  records: "It's alive."
- **Gameplay**: Viewing the Green Valley at Zone 6+ is required to unlock the
  "Leave the Train" ending option in Zone 7. Without witnessing it, the player
  cannot be certain the outside is survivable, and the dialogue option doesn't appear.

### The Shelter (Endgame — Leg 5)

- **Pass-by duration**: 2 minutes
- **Viewing**: Only visible from rooftop with telescope perk OR if player has obtained
  the Shelter Coordinates from the Cold Lab (Working Spine quest).
- **Description**: A structure on a mountainside at approximately 3,000m elevation.
  Partially built into the rock. Metal walls, solar panels (non-functional but intact),
  a radio antenna. A pre-Freeze survival bunker, possibly military or research. Snow
  cleared from the entrance by geothermal venting.
- **Thaw progression**: Always partially visible (geothermal keeps it clear). As thaw
  progresses, more of the structure's lower sections become visible, revealing it's
  larger than initially apparent—a multi-level facility.
- **Lore significance**: The Shelter represents the ultimate question of the endgame.
  Is there a place to go? The coordinates are hidden in Cold Lab data that Wilford
  suppressed. Finding them is optional but changes the final confrontation with Wilford
  from theoretical ("maybe we could leave") to concrete ("there is a specific place
  we can go").

### Atlantic Bridge Pylons (Atlantic Crossing — Leg 4)

- **Pass-by duration**: 18 minutes (full crossing)
- **Viewing**: All cars (the bridge IS the track)
- **Description**: The train crosses 3,200 km of frozen ocean on a pylon-supported
  bridge built into the ice-covered seabed. The pylons are massive concrete-and-steel
  columns rising 40 meters from the frozen ocean surface. Between pylons, the frozen
  sea stretches flat to every horizon. Wind is extreme and unobstructed.
- **Thaw progression**:
  - Zone 1-4: Solid white ice surface. Pylons coated in rime. Horizon
    indistinguishable from sky.
  - Zone 5-6: Ice surface shows pressure ridges and dark fracture lines. Pylon
    bases show water staining—tidal motion beneath the ice.
  - Zone 7: Open water visible in fracture lines. The ice is thinning. The pylons
    show waterline marks well above current ice level. The bridge is engineered for
    frozen conditions—as the ice melts, the bridge becomes more dangerous, not less.
- **Gameplay event**: The Atlantic Crossing is the most dangerous regular event.
  Wind forces are maximum. The train sways on the bridge. Unsecured items slide.
  On rooftop: impossible without Cold Suit Mk.III (wind force alone can kill via
  knockoff). Interior: First Class holds a "Crossing Party" with champagne.
  The Tail huddles and prays.

### The Bering Shake (Bering Strait — Leg 7)

- **Pass-by duration**: 5 minutes (the event itself; full strait crossing is 45 min)
- **Viewing**: All cars (felt, not just seen)
- **Description**: The track across the Bering Strait sits on pack ice over open
  ocean. Unlike the Atlantic Bridge (fixed pylons), the Bering track is laid on
  ice shelves that shift seasonally. At one specific point, tectonic and tidal
  forces create a regular seismic-like shudder.
- **Thaw progression**:
  - Zone 1-4: A sharp lurch. Items fall. NPCs brace. Over in seconds.
  - Zone 5-6: The shake is stronger. A visible crack races across the ice surface
    outside. The train's emergency braking system briefly engages (players feel
    deceleration). Hull stress sounds.
  - Zone 7: The shake is alarming. A section of ice 500m behind the train collapses
    into open water. If on rooftop: the player sees the track they just crossed
    disappear into black water. NPCs throughout the train panic. Working Spine
    crew scramble to assess hull integrity. This is foreshadowing—the Bering
    crossing won't survive many more circuits.
- **Lore**: Engineer NPCs in the Working Spine openly discuss the Bering problem.
  Their models show the ice shelf thinning 3-5% per circuit. Within 4-6 circuits,
  the crossing will be impassable. When the Bering fails, Snowpiercer's circuit
  breaks. The train dies. This is the hidden clock behind the endgame urgency.

## Minor Landmarks

Smaller points of interest visible during transit. No major gameplay impact but
contribute to environmental storytelling and world-building.

| Landmark | Leg | Duration | Description |
|---|---|---|---|
| Radar Array | 1 | 1 min | Military installation. Dishes pointed at sky. One has collapsed. |
| Frozen Waterfall | 2 | 2 min | 80m cascade frozen mid-flow. Blue-white ice. Thaw: water runs behind ice curtain. |
| Aircraft Graveyard | 3 | 3 min | Airport runway. Dozens of planes frozen in takeoff positions. Wings snapped. |
| Container Port | 3 | 2 min | Shipping containers stacked like children's blocks, some toppled by ice expansion. |
| The Lighthouse | 4 | 1 min | Coastal lighthouse still standing. Light dark. Thaw: glass intact, could be relit. |
| Mining Pit | 5 | 2 min | Open-pit mine. Terraced walls visible. Machinery frozen at work positions. |
| Highway | 6 | 4 min | Interstate highway visible for several km. Cars frozen in traffic jam. Headlights still aimed forward. |
| Stadium | 6 | 2 min | Sports stadium. Roof collapsed under snow. Seats visible through gaps. |
| Military Base | 7 | 2 min | Fenced compound. Vehicles in rows. A flag still flies, shredded to strips. |
| Satellite Dish Farm | 7 | 1 min | Deep-space communication array. Dishes intact but frost-covered. |

## Landmark Journal Integration

Observing a landmark adds a journal entry under "The World Outside." Each entry includes:

- Location name and route segment
- Description of what the player saw
- Thaw state observation (changes if revisited at higher zone)
- Optional companion commentary (if a companion was present)

Completing all major landmark observations unlocks the "Cartographer" achievement and
a codex entry: "The Map of What Remains," a reconstructed world map showing confirmed
surviving structures and emerging ecosystems.
