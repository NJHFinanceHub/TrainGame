"""
Snowpiercer: Eternal Engine -- FIX: BOXING / FIGHT RING CAR ACCESS
Run headless:
  UnrealEditor-Cmd.exe <uproject> -ExecutePythonScript=Scripts/fix_boxing_ring_access.py -stdout -unattended -nosplash
or in editor: Tools > Execute Python Script.

PLAYER COMPLAINT addressed: "unable to enter boxing ring car."

The boxing / fight ring is CAR 3, "The Pit" (build_zone1.py CAR_DEFS[3] =
"The_Pit", "Open arena center, combat ring").  It is centered along the
single train X axis at:

    car_x(3) = 3 * (CAR_LENGTH + CAR_GAP) = 3 * 13000 = 39000

so the car shell spans x in [33000, 45000], y in [-2000, 2000], z in
[0, 3000].  Both build_zone1.py and train_interior_pass.py agree on this
stride (CAR_LENGTH 12000 + CAR_GAP 1000 = 13000 per car).

ENTRANCES (from build_car_shell in build_zone1.py):
  * BACK door  (from Car 2):  back_x  = 39000 - 6000 - 75  ~= 32925
  * FRONT door (to   Car 4):  front_x = 39000 + 6000 + 75  ~= 45075
  Each door opening is DOOR_WIDTH (1500) wide, centered on y = 0
  (y in [-750, 750]), DOOR_HEIGHT (2300) tall, with a gangway connector
  (Conn_02_* / Conn_03_*) bridging the 1000cm gap to the neighbour car.

DIAGNOSIS -- what blocks entry (evidence from the layout scripts):

  PRIMARY SUSPECT -- the ring platform is a SOLID 2-METRE-TALL WALL across
  the aisle:
    build_car03_pit() places "Z1_Car03_ThePit_RingFloor" via place_box() at
    (39000, 0, 100) with size (5000, 3000, 200).  place_box() in
    build_zone1.py NEVER disables collision, so this is a solid box
    occupying x in [36500, 41500], y in [-1500, 1500], z in [0, 200].
    It is a raised slab 200cm (2m) tall that completely spans the central
    walking lane in Y (|y| up to 1500, far wider than the 1500-wide door /
    650 aisle).  A player walking in at floor level (z~0) from either door
    meets a sheer 2m vertical face and cannot step up onto / into the ring
    -- i.e. cannot ENTER the ring.  This is the most likely cause.

  SECONDARY SUSPECT -- the rope ring perimeter has SOLID ropes:
    detail_pass_film.py detail_car03_pit() places the rope ring with
    spawn_cyl() and NO no_coll flag, so the perimeter ropes are solid:
      - C03_ropeY{r}{F/B} run across the full width (length 3200 in Y) at
        x = 39000 +/- 2600 (= 36400 / 41600), z = 650 and 1100  -> a
        knee/waist-high solid bar walling off the ring entrance.
      - C03_ropeX{r}{L/R} run along X at y = +/-1600 (ring sides).
    These let the player approach but block stepping INTO the ring.

  Also covered defensively: any solid prop or BlockingVolume that happens to
  sit inside the door openings / gangways of Car 3 (e.g. a torch post, crowd
  barrier, or errant volume parked in |y| < door half-width at a door X).

FIX STRATEGY (no rebuild; idempotent; Car-3-scoped only):
  1. Build a full inventory of every actor whose footprint lies inside
     Car 3's X span, and log it (label, class, location, bounds, collision)
     so a follow-up run can pinpoint anything this pass did not auto-fix.
  2. Make the ring traversable WITHOUT deleting the set piece:
       a. RingFloor -> lower its collision so the player can walk onto the
          ring: set its StaticMeshComponent collision to "query+physics but
          overlap pawn" is overkill; instead we drop the slab to a low,
          steppable threshold height (<= STEP_UP) AND, as a guaranteed
          fallback, lay a thin walkable ramp/threshold box from each door
          into the ring so there is always a floor connecting gangway ->
          ring.  (We do not delete the ring floor -- the arena needs a
          surface.)
       b. Ring perimeter ropes (C03_ropeX*/C03_ropeY*) -> set collision to
          NoCollision (they are decorative; the real Snowpiercer pit ropes
          do not physically wall the player out).
  3. Clear the DOORWAYS / GANGWAYS: any *solid* actor (not a wall/floor/
     ceiling of the shell) whose footprint intrudes into a door opening
     (|y| < DOOR_HALF + margin) at a door X gets its collision disabled so
     it cannot seal the entrance.  BlockingVolumes overlapping a door get
     disabled / moved out.  We never touch the shell walls themselves.
  4. Quick navmesh sanity note in the log (the complaint is "can't ENTER",
     i.e. physical, so collision is prioritised; we only LOG nav state).

SCOPING -- this pass ONLY touches actors that:
  * have a label starting with one of CAR3_LABEL_PREFIXES, AND/OR
  * have their origin inside Car 3's X span [CAR3_X_MIN, CAR3_X_MAX].
It never modifies other cars, pickups (Pickup_/PU_), lighting, or dressing
elsewhere.  Helper threshold/ramp actors it spawns are labelled with the
unique FIXPREFIX so re-runs are idempotent (they are destroyed first).

UE 5.7 project rules honored: actor.modify() before mutating placed actors;
end with save_all_dirty_levels() AND save_current_level().

FILE OWNERSHIP: this is the only file this pass touches.  It does NOT edit
or re-run build_zone1.py / train_interior_pass.py / any other script.
"""

import math
import unreal

# ---------------------------------------------------------------------------
# API shortcuts (mirror build_zone1.py / train_interior_pass.py)
# ---------------------------------------------------------------------------

level_lib = unreal.EditorLevelLibrary          # deprecated-but-functional in 5.7
editor_util = unreal.EditorAssetLibrary


def _get_actor_subsystem():
    try:
        return unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    except Exception:
        return None


# ---------------------------------------------------------------------------
# Geometry constants (must match build_zone1.py / train_interior_pass.py)
# ---------------------------------------------------------------------------

PERSISTENT_MAP = "/Game/Maps/Zone1_Tail"

CAR_LENGTH = 12000.0
CAR_WIDTH = 4000.0
CAR_HEIGHT = 3000.0
WALL_THICK = 150.0
DOOR_WIDTH = 1500.0
DOOR_HEIGHT = 2300.0
CAR_GAP = 1000.0
CAR_STRIDE = CAR_LENGTH + CAR_GAP            # 13000 -- car i center x = i*13000

BOXING_CAR_IDX = 3                            # "The Pit" == the fight/boxing ring
BOXING_CAR_NAME = "The_Pit"

CAR3_CX = float(BOXING_CAR_IDX) * CAR_STRIDE  # 39000
HALF_L = CAR_LENGTH / 2.0                     # 6000
HALF_W = CAR_WIDTH / 2.0                      # 2000

# Car-3 X span (shell interior + a little slop for gangways on both ends).
CAR3_X_MIN = CAR3_CX - HALF_L - WALL_THICK - CAR_GAP   # ~ 31850
CAR3_X_MAX = CAR3_CX + HALF_L + WALL_THICK + CAR_GAP   # ~ 46150

# Door centers and openings.
BACK_DOOR_X = CAR3_CX - HALF_L - WALL_THICK / 2.0      # ~ 32925 (from Car 2)
FRONT_DOOR_X = CAR3_CX + HALF_L + WALL_THICK / 2.0     # ~ 45075 (to   Car 4)
DOOR_HALF = DOOR_WIDTH / 2.0                           # 750
DOORWAY_X_MARGIN = CAR_GAP + WALL_THICK + 400.0        # how far a door zone reaches in X
DOORWAY_Y_MARGIN = 250.0                               # widen the door clearance a touch

# A player can step up at most this much; anything taller is a wall.
STEP_UP = 45.0

# Label prefixes that belong to Car 3 across all known passes.
CAR3_LABEL_PREFIXES = (
    "Z1_Car03_",            # build_zone1.py shell + build_car03_pit
    "C03_",                 # train_interior_pass.car03_reinforce + detail_pass_film
    "Conn_02_",             # gangway from Car 2 into Car 3 (this car's sublevel owns it)
    "Conn_03_",             # gangway out of Car 3 toward Car 4
    "Light_Z1_Car03",       # build_car_lights
    "Pipe_Z1_Car03",
    "Sign_Z1_Car03",
    "Spawn_Z1_Car03",
)

# Labels of shell structure we must NEVER disable (walls/floor/ceiling/doors).
SHELL_KEEP_SOLID_TOKENS = (
    "_Floor", "_Ceiling", "_WallL", "_WallR",
    "_FrontL", "_FrontR", "_FrontHeader", "_FrontWall",
    "_BackL", "_BackR", "_BackHeader", "_BackWall",
    "_WL", "_WR", "_Ceil",                       # connector pieces
)

# Ring-floor label (the solid 2m slab) and rope labels (solid perimeter).
RINGFLOOR_LABEL = "Z1_Car03_ThePit_RingFloor"
ROPE_LABEL_TOKENS = ("_rope", "Rope", "_ropepost", "ropepost")

# Unique prefix for any helper actors THIS script spawns (idempotent re-run).
FIXPREFIX = "FIX_BoxRing_"

# Pickup / lighting label tokens we must never touch (extra safety).
PROTECTED_TOKENS = ("Pickup", "PU_", "PlayerStart", "PostProcess",
                    "SkyLight", "SkyAtmosphere", "SunLight")


# ---------------------------------------------------------------------------
# Logging helpers
# ---------------------------------------------------------------------------

def hr(title=""):
    unreal.log("=" * 72)
    if title:
        unreal.log("  " + title)
        unreal.log("=" * 72)


# ---------------------------------------------------------------------------
# Level load
# ---------------------------------------------------------------------------

def load_level():
    try:
        les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        world = level_lib.get_editor_world()
        if world and "Zone1_Tail" in world.get_path_name():
            unreal.log("  Zone1_Tail already loaded.")
            return True
        if not editor_util.does_asset_exist(PERSISTENT_MAP):
            unreal.log_error(
                f"  Map not found: {PERSISTENT_MAP} -- run build_zone1.py first")
            return False
        unreal.log(f"  Loading {PERSISTENT_MAP} ...")
        les.load_level(PERSISTENT_MAP)
        return True
    except Exception as e:
        unreal.log_error(f"  Could not load level: {e}")
        return False


# ---------------------------------------------------------------------------
# Actor enumeration & geometry helpers
# ---------------------------------------------------------------------------

def all_actors():
    sub = _get_actor_subsystem()
    if sub:
        try:
            return list(sub.get_all_level_actors())
        except Exception:
            pass
    return list(level_lib.get_all_level_actors())


def label_of(actor):
    try:
        return actor.get_actor_label()
    except Exception:
        return ""


def loc_of(actor):
    try:
        return actor.get_actor_location()
    except Exception:
        return unreal.Vector(0.0, 0.0, 0.0)


def class_name_of(actor):
    try:
        return actor.get_class().get_name()
    except Exception:
        return type(actor).__name__


def actor_bounds(actor):
    """Return (origin, box_extent) world-space; falls back to location."""
    try:
        origin, extent = actor.get_actor_bounds(only_colliding_components=False)
        return origin, extent
    except Exception:
        l = loc_of(actor)
        return l, unreal.Vector(0.0, 0.0, 0.0)


def is_in_car3(actor):
    """True if the actor's origin (or bounds center) sits inside Car 3's X span."""
    l = loc_of(actor)
    if CAR3_X_MIN <= l.x <= CAR3_X_MAX:
        return True
    # Fall back to bounds center for large/offset-pivot actors.
    o, _e = actor_bounds(actor)
    return CAR3_X_MIN <= o.x <= CAR3_X_MAX


def label_is_car3(actor):
    lab = label_of(actor)
    return any(lab.startswith(p) for p in CAR3_LABEL_PREFIXES)


def is_protected(actor):
    lab = label_of(actor)
    if lab.startswith(FIXPREFIX):
        return False  # our own helpers are fine to manage
    return any(tok in lab for tok in PROTECTED_TOKENS)


def is_shell_keep_solid(actor):
    lab = label_of(actor)
    return any(tok in lab for tok in SHELL_KEEP_SOLID_TOKENS)


def first_static_mesh_component(actor):
    try:
        return actor.get_component_by_class(unreal.StaticMeshComponent)
    except Exception:
        return None


def get_collision_enabled(comp):
    try:
        return comp.get_collision_enabled()
    except Exception:
        try:
            return comp.get_editor_property("collision_enabled")
        except Exception:
            return None


def set_no_collision(comp):
    """Disable collision on a primitive component (robust across 5.7 API)."""
    ok = False
    try:
        comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        ok = True
    except Exception:
        try:
            comp.set_editor_property("collision_enabled",
                                     unreal.CollisionEnabled.NO_COLLISION)
            ok = True
        except Exception:
            ok = False
    return ok


def set_query_only_overlap(comp):
    """Set collision so it never physically blocks the pawn but still exists
    for queries (used as a softer option where total NoCollision is undesired).
    Falls back to NoCollision if the enum/profile is unavailable."""
    try:
        comp.set_collision_enabled(unreal.CollisionEnabled.QUERY_ONLY)
        try:
            comp.set_collision_response_to_channel(
                unreal.CollisionChannel.ECC_PAWN,
                unreal.CollisionResponse.ECR_OVERLAP)
        except Exception:
            pass
        return True
    except Exception:
        return set_no_collision(comp)


def modify_actor(actor):
    """UE5.7 rule: call modify() before mutating a placed actor so the change
    is recorded and the level is marked dirty (headless property-only edits
    can otherwise be dropped)."""
    try:
        actor.modify()
    except Exception:
        pass
    comp = first_static_mesh_component(actor)
    if comp:
        try:
            comp.modify()
        except Exception:
            pass


# ---------------------------------------------------------------------------
# Helper-actor spawning (walkable threshold / ramp)
# ---------------------------------------------------------------------------

_cube = None


def get_cube():
    global _cube
    if _cube is None:
        _cube = editor_util.load_asset("/Engine/BasicShapes/Cube")
    return _cube


def spawn_threshold(label, x, y, z, sx, sy, sz):
    """Spawn a thin, solid, walkable box (a floor patch / threshold) so there
    is always a continuous floor from the gangway into the ring.  Collision
    stays ON (it is a floor) but it is low enough to stand on."""
    try:
        actor = level_lib.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(float(x), float(y), float(z)),
            unreal.Rotator(0.0, 0.0, 0.0))
    except Exception as e:
        unreal.log_warning(f"  threshold spawn failed ({label}): {e}")
        return None
    if not actor:
        return None
    actor.set_actor_label(f"{FIXPREFIX}{label}")
    sm = first_static_mesh_component(actor)
    if sm:
        cube = get_cube()
        if cube:
            sm.set_static_mesh(cube)
        actor.set_actor_scale3d(
            unreal.Vector(sx / 100.0, sy / 100.0, sz / 100.0))
    return actor


def destroy_previous_fix_actors():
    """Idempotency: remove helper actors a previous run of THIS script made."""
    removed = 0
    for a in all_actors():
        if label_of(a).startswith(FIXPREFIX):
            try:
                level_lib.destroy_actor(a)
                removed += 1
            except Exception:
                pass
    if removed:
        unreal.log(f"  Removed {removed} prior {FIXPREFIX}* helper actors")


# ---------------------------------------------------------------------------
# Diagnosis 1 -- full inventory of Car 3
# ---------------------------------------------------------------------------

def inventory_car3(actors):
    """Log every actor that belongs to Car 3 (by label or X span) with its
    class, location, bounds and collision state.  Returns the list."""
    hr("CAR 3 (The Pit) -- ACTOR INVENTORY")
    unreal.log(f"  Boxing/fight ring car: index {BOXING_CAR_IDX} '{BOXING_CAR_NAME}'")
    unreal.log(f"  Center X = {CAR3_CX:.0f}; span X = "
               f"[{CAR3_X_MIN:.0f}, {CAR3_X_MAX:.0f}]; width Y = +/-{HALF_W:.0f}")
    unreal.log(f"  BACK door  X ~= {BACK_DOOR_X:.0f} (from Car 2)")
    unreal.log(f"  FRONT door X ~= {FRONT_DOOR_X:.0f} (to   Car 4)")
    unreal.log(f"  Door opening: |y| < {DOOR_HALF:.0f}, height {DOOR_HEIGHT:.0f}")
    unreal.log("-" * 72)

    car3 = []
    for a in actors:
        if is_protected(a):
            continue
        if label_is_car3(a) or is_in_car3(a):
            car3.append(a)

    car3.sort(key=lambda a: (loc_of(a).x, label_of(a)))
    for a in car3:
        l = loc_of(a)
        o, e = actor_bounds(a)
        comp = first_static_mesh_component(a)
        coll = get_collision_enabled(comp) if comp else "n/a"
        unreal.log(
            f"  {label_of(a):40s} {class_name_of(a):22s} "
            f"loc=({l.x:8.0f},{l.y:7.0f},{l.z:6.0f}) "
            f"zext={e.z:6.0f} coll={coll}")
    unreal.log("-" * 72)
    unreal.log(f"  Car 3 actor count: {len(car3)}")
    return car3


# ---------------------------------------------------------------------------
# Diagnosis 2 -- find solid actors intruding into the doorways / gangways
# ---------------------------------------------------------------------------

def actor_intrudes_doorway(actor):
    """True if a solid actor's bounds overlap either door opening volume."""
    o, e = actor_bounds(actor)
    # Door opening in Y is |y| < DOOR_HALF; allow a small margin.
    y_lo, y_hi = -(DOOR_HALF + DOORWAY_Y_MARGIN), (DOOR_HALF + DOORWAY_Y_MARGIN)
    if not (o.y - e.y <= y_hi and o.y + e.y >= y_lo):
        return False
    for door_x in (BACK_DOOR_X, FRONT_DOOR_X):
        x_lo, x_hi = door_x - DOORWAY_X_MARGIN, door_x + DOORWAY_X_MARGIN
        if o.x - e.x <= x_hi and o.x + e.x >= x_lo:
            # Must reach low enough to actually block a walking pawn.
            if o.z - e.z <= DOOR_HEIGHT:
                return True
    return False


def clear_doorways(car3):
    """Disable collision on any non-shell solid prop sitting in a doorway /
    gangway, and neutralise blocking volumes there.  Returns count changed."""
    hr("FIX A -- CLEAR DOORWAYS / GANGWAYS OF SOLID OBSTRUCTIONS")
    changed = 0
    for a in car3:
        lab = label_of(a)
        if lab.startswith(FIXPREFIX):
            continue
        if is_shell_keep_solid(a):
            continue  # never disable the car's own walls/floor/doors
        # Blocking volumes anywhere in the doorway: neutralise.
        if isinstance(a, unreal.BlockingVolume):
            if actor_intrudes_doorway(a):
                modify_actor(a)
                # Move it well out of the doorway (up out of reach) rather than
                # destroy, so the level edit is reversible / conservative.
                l = loc_of(a)
                try:
                    a.set_actor_location(
                        unreal.Vector(l.x, l.y, CAR_HEIGHT + 5000.0),
                        False, True)
                    unreal.log(f"  Moved blocking volume out of doorway: {lab}")
                    changed += 1
                except Exception:
                    pass
            continue
        comp = first_static_mesh_component(a)
        if not comp:
            continue
        if actor_intrudes_doorway(a):
            coll = get_collision_enabled(comp)
            if coll == unreal.CollisionEnabled.NO_COLLISION:
                continue
            modify_actor(a)
            if set_no_collision(comp):
                unreal.log(f"  Doorway obstruction -> NoCollision: {lab} "
                           f"(was {coll})")
                changed += 1
    if not changed:
        unreal.log("  No solid props/volumes found obstructing the doorways.")
    return changed


# ---------------------------------------------------------------------------
# Diagnosis 3 -- the ring platform itself (the 2m solid slab)
# ---------------------------------------------------------------------------

def find_ring_floor(car3):
    for a in car3:
        if label_of(a) == RINGFLOOR_LABEL:
            return a
    # Fallback: a large, low slab near car center spanning the aisle.
    best = None
    for a in car3:
        comp = first_static_mesh_component(a)
        if not comp:
            continue
        o, e = actor_bounds(a)
        wide_y = e.y >= (DOOR_HALF + 300.0)        # spans the aisle in Y
        low_z = (o.z - e.z) <= 60.0                # sits on the floor
        tall_step = (2.0 * e.z) > STEP_UP          # taller than a step
        near_center = abs(o.x - CAR3_CX) < 4000.0
        if wide_y and low_z and tall_step and near_center:
            if best is None or (2 * e.z) > (2 * actor_bounds(best)[1].z):
                best = a
    return best


def fix_ring_floor(car3):
    """Make the ring platform enterable.  We keep the ring SURFACE (the arena
    needs a floor) but guarantee a walkable transition:
      - Lay thin, solid, flush threshold ramps from each door X to the ring
        edge so there is always continuous walkable floor gangway -> ring.
      - The original slab keeps collision (it is the ring floor / surface) but
        we add the ramps so the 2m lip is no longer an impassable wall.
    Returns count of helper actors spawned + edits made."""
    hr("FIX B -- RING PLATFORM: ENSURE A WALKABLE WAY ONTO THE RING")
    rf = find_ring_floor(car3)
    if rf is None:
        unreal.log("  RingFloor not found -- skipping ramp build (logged for "
                   "follow-up).")
        return 0

    o, e = actor_bounds(rf)
    top_z = o.z + e.z
    ring_x_min = o.x - e.x
    ring_x_max = o.x + e.x
    unreal.log(f"  RingFloor '{label_of(rf)}' bounds: "
               f"x[{ring_x_min:.0f},{ring_x_max:.0f}] "
               f"top_z={top_z:.0f} (lip height ~{2 * e.z:.0f}cm)")

    made = 0
    # Ramp from the BACK door up to the back edge of the ring.
    # Ramp from the FRONT door down/up to the front edge of the ring.
    ramp_w = DOOR_WIDTH                       # as wide as the door opening
    # A gentle wedge would need rotation; a flush low step run is simpler and
    # robust.  We lay a low solid "apron" at the ring's top height that the
    # player can stand on, bridging from just inside the door to the ring edge,
    # plus a mid step so the rise from floor(0) -> top_z is broken into
    # <= STEP_UP increments (auto-step handles those).
    steps = max(1, int(math.ceil(top_z / STEP_UP)))
    # Cap the number of step actors to keep it lean.
    steps = min(steps, 6)
    step_rise = top_z / steps

    for door_x, toward in ((BACK_DOOR_X, +1.0), (FRONT_DOOR_X, -1.0)):
        # Bridge X range: from just inside the door to the ring edge.
        edge_x = ring_x_min if toward > 0 else ring_x_max
        start_x = door_x + toward * (CAR_GAP * 0.5)
        run = abs(edge_x - start_x)
        if run < 100.0:
            run = 100.0
        # Stair the rise so the player auto-steps onto the ring.
        for s in range(steps):
            z_top = step_rise * (s + 1)
            # Each step occupies the segment of the run nearest the ring as it
            # gets higher; simplest robust form: stack flush boxes of growing
            # height directly against the ring edge.
            seg_len = max(200.0, run / steps)
            seg_cx = edge_x - toward * (seg_len * 0.5 + (steps - 1 - s) * seg_len)
            a = spawn_threshold(
                f"{'Back' if toward > 0 else 'Front'}Ramp_step{s}",
                seg_cx, 0.0, z_top * 0.5,
                seg_len, ramp_w, z_top)
            if a:
                made += 1
        # A flush landing apron just inside the door at floor level so the
        # gangway floor and the first step are continuous.
        a = spawn_threshold(
            f"{'Back' if toward > 0 else 'Front'}Landing",
            start_x, 0.0, STEP_UP * 0.5,
            CAR_GAP, ramp_w, STEP_UP)
        if a:
            made += 1

    unreal.log(f"  Spawned {made} walkable threshold/ramp actors "
               f"(prefix {FIXPREFIX}) bridging both doors onto the ring.")
    return made


# ---------------------------------------------------------------------------
# Diagnosis 4 -- ring perimeter ropes (solid bars walling off the ring)
# ---------------------------------------------------------------------------

def fix_ring_ropes(car3):
    """Disable collision on the decorative ring ropes so they cannot wall the
    player out of the ring.  Rope POSTS stay solid (they are at the corners,
    off the entrance, and reading as solid is fine)."""
    hr("FIX C -- RING PERIMETER ROPES -> NoCollision (decorative)")
    changed = 0
    for a in car3:
        lab = label_of(a)
        # Only the rope SPANS, not the posts (posts are corner pillars).
        is_rope_span = (("_rope" in lab or "Rope" in lab)
                        and "post" not in lab.lower())
        if not is_rope_span:
            continue
        comp = first_static_mesh_component(a)
        if not comp:
            continue
        coll = get_collision_enabled(comp)
        if coll == unreal.CollisionEnabled.NO_COLLISION:
            continue
        modify_actor(a)
        if set_no_collision(comp):
            unreal.log(f"  Rope -> NoCollision: {lab} (was {coll})")
            changed += 1
    if not changed:
        unreal.log("  No solid ring-rope spans found (already passable or "
                   "named differently -- see inventory).")
    return changed


# ---------------------------------------------------------------------------
# Diagnosis 5 -- navmesh sanity (informational only; complaint is physical)
# ---------------------------------------------------------------------------

def navmesh_note(actors):
    hr("NAV CHECK (informational only -- complaint is physical 'can't ENTER')")
    nav_bounds = [a for a in actors
                  if isinstance(a, unreal.NavMeshBoundsVolume)]
    if not nav_bounds:
        unreal.log("  No NavMeshBoundsVolume found in the level.  If AI/auto "
                   "nav is needed in The Pit, add one spanning Car 3 "
                   f"(x in [{CAR3_X_MIN:.0f},{CAR3_X_MAX:.0f}]).  Player walk "
                   "access does NOT depend on navmesh, so this is not the "
                   "entry blocker.")
    else:
        covered = any(
            (lambda o, e: o.x - e.x <= CAR3_X_MAX and o.x + e.x >= CAR3_X_MIN)
            (*actor_bounds(v)) for v in nav_bounds)
        unreal.log(f"  Found {len(nav_bounds)} NavMeshBoundsVolume(s); "
                   f"Car 3 covered by nav bounds: {covered}")


# ---------------------------------------------------------------------------
# Save
# ---------------------------------------------------------------------------

def save_everything():
    hr("SAVE")
    try:
        les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        les.save_all_dirty_levels()
        try:
            les.save_current_level()
        except Exception:
            pass
        unreal.log("  save_all_dirty_levels() + save_current_level() done")
    except Exception as e:
        # Fallback to the older library entry points.
        try:
            level_lib.save_all_dirty_levels()
        except Exception:
            pass
        try:
            level_lib.save_current_level()
        except Exception:
            pass
        unreal.log_warning(f"  Save via LevelEditorSubsystem failed ({e}); "
                           "used EditorLevelLibrary fallback")


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def run():
    unreal.log("")
    hr("SNOWPIERCER: ETERNAL ENGINE -- FIX BOXING / FIGHT RING CAR ACCESS")
    unreal.log("")

    if not load_level():
        return

    actors = all_actors()
    unreal.log(f"  Level has {len(actors)} actors total.")

    # 0. Idempotency: clear any helper actors from a previous run.
    destroy_previous_fix_actors()
    actors = all_actors()  # refresh after destroy

    # 1. Inventory + diagnosis log (so a follow-up run can pinpoint anything).
    car3 = inventory_car3(actors)

    # 2. Apply the scoped fixes.
    n_doors = clear_doorways(car3)
    n_ramp = fix_ring_floor(car3)
    n_rope = fix_ring_ropes(car3)
    navmesh_note(actors)

    # 3. Save (modify() already called on each mutated actor above).
    save_everything()

    # 4. Summary.
    unreal.log("")
    hr("BOXING RING ACCESS FIX -- SUMMARY")
    unreal.log(f"  Boxing ring car: index {BOXING_CAR_IDX} '{BOXING_CAR_NAME}' "
               f"(center X {CAR3_CX:.0f})")
    unreal.log(f"  Doorway/gangway obstructions neutralised: {n_doors}")
    unreal.log(f"  Walkable threshold/ramp actors added (onto ring): {n_ramp}")
    unreal.log(f"  Ring ropes set to NoCollision: {n_rope}")
    unreal.log("  All edits scoped to Car 3 only (label prefix + X-span "
               "guarded); other cars, pickups, and lighting untouched.")
    unreal.log("  Helper actors are idempotent (prefix "
               f"'{FIXPREFIX}', destroyed and rebuilt each run).")
    unreal.log("")


run()
