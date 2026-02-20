# Weather System

Dynamic weather drives exterior visual presentation, gameplay hazard intensity, and narrative
atmosphere. Weather is tied to route segment, story zone progression, and circuit timing.

## Weather States

### Blizzard

- **Visibility**: 2-5 meters. Screen fills with horizontal snow particles. Far geometry
  hidden behind white wall. Train hull groans audibly.
- **Wind force**: 80-120 km/h gusts. Player pushed laterally on rooftop (+0.3m/s drift
  toward downwind rail). Standing upright doubles drift. Crouching halves it.
- **Temperature modifier**: -15°C to ambient. Effective exterior temp during Siberian
  blizzard reaches -100°C.
- **Frequency**: Dominant in Legs 1 (Siberia), 7 (Bering). Common in Legs 2, 6. Rare
  in Legs 3-5.
- **Gameplay**: Rooftop traversal near-impossible without Cold Suit Mk.II+. Exterior
  objectives receive time penalty (+50% frostbite accumulation). Visibility reduction
  makes tunnel warnings harder to spot visually (audio cues critical).

### Heavy Snow

- **Visibility**: 10-20 meters. Steady snowfall with occasional gusts. Train cars ahead
  visible as silhouettes. Distant landmarks obscured.
- **Wind force**: 30-60 km/h. Mild lateral drift on rooftop (+0.1m/s).
- **Temperature modifier**: -8°C to ambient.
- **Frequency**: Common across all legs except Andean Tunnels.
- **Gameplay**: Standard exterior hazard. Cold Suit Mk.I sufficient for short excursions.
  Landmarks partially visible through snow curtain.

### Overcast / Light Snow

- **Visibility**: 50-100 meters. Thin snowflakes drift lazily. Horizon line visible.
  Landmarks clearly defined. Sky a flat grey-white.
- **Wind force**: 10-20 km/h. Negligible player drift.
- **Temperature modifier**: -3°C to ambient.
- **Frequency**: Common in Legs 3-6. Increases as thaw progresses.
- **Gameplay**: Best conditions for rooftop exploration. Landmark observation windows
  widened. Cold suit timer runs at base rate.

### Clear Skies

- **Visibility**: Maximum render distance. Horizon sharp. Stars visible at night. Sun
  sits low on horizon (polar positioning), casting long blue-gold shadows across ice.
- **Wind force**: 5-15 km/h. No meaningful drift.
- **Temperature modifier**: +0°C (no modification, but radiative cooling at night adds
  -5°C between 22:00-04:00).
- **Frequency**: Rare in Legs 1, 7. Uncommon in Legs 2, 6. Common in Legs 3-5 as thaw
  progresses.
- **Gameplay**: Optimal exterior conditions. Full landmark visibility. Night clear skies
  enable aurora events.

### Aurora Borealis

- **Trigger**: Clear skies + night cycle + latitude above 55°N (Legs 1, 2, 6, 7) OR
  latitude above 45°S (Leg 5, southern aurora australis).
- **Visuals**: Curtains of green, violet, and pale blue ripple across the sky. Reflections
  shimmer on ice surfaces. The train's hull catches faint color. Snow crystals in the air
  scatter the light into ground-level prismatic effects.
- **Duration**: 10-30 minutes real-time per event. 2-3 events per circuit in eligible
  segments.
- **Gameplay**: No mechanical effect. Pure atmospheric reward. Characters on rooftop
  comment on the sight. Companions have unique dialogue. First aurora sighting triggers
  a journal entry. If the player witnesses all three aurora color variants (green, violet,
  red) across the playthrough, unlocks the "Northern Lights" codex entry detailing
  pre-Freeze aurora science.

### Whiteout

- **Trigger**: Extreme blizzard escalation. Occurs 2-3 times per circuit in Siberian
  and Bering segments.
- **Visibility**: 0-1 meter. Screen nearly fully white. Only the player's hands and
  the hull surface directly underfoot are visible. Audio becomes muffled except for
  wind roar.
- **Wind force**: 120-180 km/h. Standing upright on rooftop = automatic knockdown
  (prone, sliding toward rail). Crouch-only movement at 30% speed.
- **Temperature modifier**: -25°C to ambient.
- **Duration**: 3-8 minutes. Telegraphed 60 seconds in advance by rising wind audio
  and screen-edge frost effects.
- **Gameplay**: Exterior traversal effectively blocked. Players caught outside must
  reach shelter (heated vent, hatch) or die. Cold Suit Mk.III provides survival but
  movement remains severely restricted. Interior-facing windows frost over completely.

## Day-Night Cycle

The train operates on a 24-hour cycle maintained by Wilford's timekeeping system.
Actual solar position varies dramatically by latitude and season.

| Time Block | Duration | Exterior Lighting |
|---|---|---|
| Dawn (05:00-07:00) | 2 hr | Orange-pink horizon glow. Ice surfaces catch warm light. |
| Day (07:00-17:00) | 10 hr | Flat grey-white light (overcast) or sharp blue-white (clear). Low sun angle casts long shadows. |
| Dusk (17:00-19:00) | 2 hr | Deep blue-violet sky. Train headlight beam visible cutting forward through snow/mist. |
| Night (19:00-05:00) | 10 hr | Near-total darkness. Starfield visible on clear nights. Train running lights create pools of warm yellow along hull. Interior light spills from windows. |

During polar segments (Legs 1, 7), extended periods of perpetual twilight or perpetual
night occur. The sun may not rise above the horizon for 3-4 days of travel time.

## Weather Transitions

Weather does not switch instantly. Transitions take 2-5 minutes:

- **Blizzard → Clear**: Wind dies gradually. Snow thins from horizontal to drifting to
  absent. Cloud layer breaks apart revealing sky. Temperature modifier lifts over 3 min.
- **Clear → Blizzard**: Horizon clouds mass and approach. Wind builds. First flakes
  arrive light, then intensify. Full blizzard in 4-5 min.
- **Any → Whiteout**: Existing snow/wind intensifies sharply. 60-second warning window.
  Screen frost creeps from edges.

## Weather by Route Segment

| Leg | Primary Weather | Secondary | Rare Events |
|---|---|---|---|
| 1 - East Asia → Siberia | Blizzard (50%) | Heavy Snow (30%) | Whiteout (15%), Clear (5%) |
| 2 - Siberia → Europe | Heavy Snow (40%) | Blizzard (25%) | Overcast (20%), Clear (10%), Aurora (5%) |
| 3 - Europe → Africa | Overcast (45%) | Heavy Snow (30%) | Clear (15%), Blizzard (10%) |
| 4 - Africa → S. America | Heavy Snow (35%) | Overcast (35%) | Clear (20%), Blizzard (10%) |
| 5 - South America | Overcast (40%) | Clear (25%) | Heavy Snow (20%), Aurora Australis (10%), Blizzard (5%) |
| 6 - S. America → N. America | Heavy Snow (35%) | Overcast (30%) | Clear (20%), Blizzard (10%), Aurora (5%) |
| 7 - N. America → E. Asia | Blizzard (45%) | Heavy Snow (30%) | Whiteout (15%), Clear (5%), Aurora (5%) |

## Interior Weather Effects

Weather is not just an exterior system. Interior environments respond:

- **Window frost**: Accumulates during blizzards, clears during calm weather. Obscures
  landmark observation. Players can scrape frost from windows (interaction prompt).
- **Hull condensation**: Warmer interior cars (First Class, Working Spine) develop
  condensation streams on walls during temperature differentials. Cosmetic detail.
- **Sound**: Blizzard wind audible inside all cars, louder near hull. Muffled in
  interior rooms, prominent in corridor cars. Whiteout produces a deep bass rumble
  that vibrates loose objects.
- **Lighting flicker**: Extreme weather causes intermittent power fluctuations in
  outer-hull cars (Tail, Third Class). Lights flicker. Screens glitch. 2-3 second
  blackouts during whiteouts.
- **Temperature bleed**: Hull-adjacent sleeping quarters in Tail and Third Class
  drop 2-4°C during blizzards. NPCs huddle, pull blankets tighter, complain.
  Cosmetic but reinforces class disparity (First Class is fully insulated).

## External Temperature Display

The external temperature is shown on environmental monitors throughout the train
and on the player's HUD when near windows or on the exterior.

### Display Locations

- **Tail**: Single cracked monitor in Car 2 (Protein Kitchen). Often broken. Shows
  temperature in Celsius only. Updated every 10 minutes. Tailies gather around it
  during clear weather, hoping for warmer numbers.
- **Third Class**: Digital readouts at car junctions. Updated every 5 minutes. Workers
  check before maintenance shifts.
- **Second Class**: Wall-mounted environmental panels with temperature, humidity, wind
  speed, and barometric pressure. Real-time updates.
- **Working Spine**: Full meteorological dashboard in Cold Lab (Car 52). Most detailed
  data available. Includes historical graphs showing temperature trends across circuits.
  This is where the thaw data is discoverable.
- **First Class**: Elegant brass-and-glass thermometers at observation lounges. Decorative.
  Temperature displayed in both Celsius and Fahrenheit. Updated real-time.
- **Sanctum**: Environmental data restricted. Only the Order's inner circle sees raw data.
  Public displays show "Wilford's Mercy Sustains" instead of numbers.

### HUD Temperature Display

- Appears in bottom-left corner when player is on exterior or looking through a window.
- Format: `-67°C | WIND: 85 km/h | EFFECTIVE: -89°C`
- Color-coded: Blue (-30 to -50°C), Deep Blue (-50 to -70°C), Purple (-70 to -90°C),
  White/flashing (-90°C and below).
- Wind chill calculation: `effective = ambient - (wind_speed × 0.25)` (simplified for
  gameplay clarity).
- During thaw progression, the numbers visibly warm. Players who track the display across
  zones notice the trend. No explicit notification—discovery is organic.
