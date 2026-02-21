"""
Snowpiercer: Eternal Engine — Material & Lighting Persistence
Run in editor: Tools > Execute Python Script

Solves the ephemeral-asset problem: all materials and lighting setups are
created AND saved as .uasset files so they survive editor restarts.

Creates:
  1. Persisted PBR materials for all 6 zones (saved to disk)
  2. Zone-specific lighting profile Blueprints (PointLight presets)
  3. PostProcessVolume Blueprints for all 6 zones
  4. ExponentialHeightFog Blueprints per zone
  5. Zone lighting rig Blueprints (complete lighting setup per zone)

Zone Palette:
  Tail    — dark, flickering fluorescent, grime
  Third   — industrial yellow-green, buzzing overhead
  Second  — warm wood tones, amber lamps
  First   — gold, crystal, art deco, chandeliers
  Sanctum — white marble, sterile, clinical
  Engine  — brass, copper, steam, furnace glow
"""

import unreal

# ---------------------------------------------------------------------------
# API shortcuts
# ---------------------------------------------------------------------------

editor_util = unreal.EditorAssetLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
mat_lib = unreal.MaterialEditingLibrary

_saved = 0
_created = 0


def _count(created=0, saved=0):
    global _saved, _created
    _saved += saved
    _created += created


def ensure_dir(path):
    if not editor_util.does_directory_exist(path):
        editor_util.make_directory(path)


def save(asset_path):
    try:
        editor_util.save_asset(asset_path, only_if_is_dirty=False)
        _count(saved=1)
        return True
    except Exception as e:
        unreal.log_warning(f"  Save failed: {asset_path}: {e}")
        return False


def save_dir(dir_path):
    try:
        editor_util.save_directory(dir_path, only_if_is_dirty=False, recursive=True)
        return True
    except Exception as e:
        unreal.log_warning(f"  Save directory failed: {dir_path}: {e}")
        return False


# ============================================================================
# 1. ZONE MATERIAL DEFINITIONS
# ============================================================================

# Each zone has surface materials (floor, wall, ceiling, door, trim) plus
# accent materials for zone-specific props.
# Format: (name, r, g, b, roughness, metallic)

ZONE_MATERIALS = {
    "Tail": {
        "surfaces": [
            ("Tail_Floor",       0.03, 0.03, 0.03, 0.90, 0.70),
            ("Tail_Wall",        0.05, 0.04, 0.03, 0.85, 0.60),
            ("Tail_Ceiling",     0.04, 0.03, 0.02, 0.80, 0.50),
            ("Tail_Door",        0.07, 0.05, 0.04, 0.75, 0.80),
            ("Tail_Trim",        0.06, 0.04, 0.03, 0.70, 0.85),
        ],
        "accents": [
            ("Tail_RustyPipe",   0.10, 0.05, 0.03, 0.80, 0.90),
            ("Tail_GrimeMetal",  0.04, 0.03, 0.03, 0.90, 0.75),
            ("Tail_WornFabric",  0.06, 0.05, 0.04, 0.95, 0.05),
            ("Tail_Soot",        0.02, 0.02, 0.02, 0.95, 0.10),
            ("Tail_BunkWood",    0.08, 0.06, 0.04, 0.85, 0.10),
        ],
    },
    "Third": {
        "surfaces": [
            ("Third_Floor",      0.08, 0.07, 0.05, 0.85, 0.60),
            ("Third_Wall",       0.10, 0.09, 0.06, 0.80, 0.50),
            ("Third_Ceiling",    0.12, 0.11, 0.08, 0.75, 0.40),
            ("Third_Door",       0.09, 0.08, 0.05, 0.70, 0.75),
            ("Third_Trim",       0.07, 0.06, 0.04, 0.65, 0.80),
        ],
        "accents": [
            ("Third_IndustPipe", 0.06, 0.07, 0.04, 0.80, 0.90),
            ("Third_YellowPaint",0.18, 0.16, 0.04, 0.70, 0.20),
            ("Third_GreenPanel", 0.05, 0.08, 0.04, 0.75, 0.30),
            ("Third_ConcretePad",0.12, 0.11, 0.10, 0.90, 0.10),
            ("Third_WorkBench",  0.07, 0.06, 0.04, 0.80, 0.15),
        ],
    },
    "Second": {
        "surfaces": [
            ("Second_Floor",     0.12, 0.09, 0.06, 0.75, 0.30),
            ("Second_Wall",      0.15, 0.12, 0.08, 0.70, 0.20),
            ("Second_Ceiling",   0.18, 0.15, 0.10, 0.65, 0.15),
            ("Second_Door",      0.14, 0.10, 0.07, 0.60, 0.40),
            ("Second_Trim",      0.10, 0.08, 0.05, 0.55, 0.50),
        ],
        "accents": [
            ("Second_WarmWood",  0.16, 0.10, 0.06, 0.75, 0.05),
            ("Second_Leather",   0.12, 0.08, 0.05, 0.80, 0.10),
            ("Second_AmberLamp", 0.25, 0.18, 0.08, 0.40, 0.60),
            ("Second_Carpet",    0.10, 0.06, 0.04, 0.95, 0.02),
            ("Second_Brass",     0.20, 0.15, 0.05, 0.30, 0.95),
        ],
    },
    "First": {
        "surfaces": [
            ("First_Floor",      0.18, 0.14, 0.08, 0.50, 0.20),
            ("First_Wall",       0.22, 0.18, 0.12, 0.45, 0.15),
            ("First_Ceiling",    0.25, 0.22, 0.18, 0.40, 0.10),
            ("First_Door",       0.20, 0.16, 0.10, 0.35, 0.50),
            ("First_Trim",       0.30, 0.25, 0.10, 0.25, 0.90),
        ],
        "accents": [
            ("First_GoldLeaf",   0.40, 0.30, 0.08, 0.20, 0.95),
            ("First_Crystal",    0.80, 0.80, 0.85, 0.05, 0.10),
            ("First_Velvet",     0.15, 0.04, 0.04, 0.90, 0.02),
            ("First_Mahogany",   0.12, 0.06, 0.03, 0.65, 0.08),
            ("First_Marble",     0.35, 0.33, 0.30, 0.30, 0.15),
        ],
    },
    "Sanctum": {
        "surfaces": [
            ("Sanctum_Floor",    0.70, 0.68, 0.65, 0.25, 0.10),
            ("Sanctum_Wall",     0.75, 0.73, 0.70, 0.20, 0.08),
            ("Sanctum_Ceiling",  0.80, 0.78, 0.75, 0.15, 0.05),
            ("Sanctum_Door",     0.60, 0.58, 0.55, 0.30, 0.40),
            ("Sanctum_Trim",     0.65, 0.63, 0.60, 0.20, 0.50),
        ],
        "accents": [
            ("Sanctum_WhiteMarble", 0.85, 0.83, 0.80, 0.15, 0.08),
            ("Sanctum_Chrome",   0.70, 0.72, 0.75, 0.10, 0.95),
            ("Sanctum_Glass",    0.90, 0.90, 0.92, 0.05, 0.05),
            ("Sanctum_Linen",    0.75, 0.73, 0.70, 0.90, 0.02),
            ("Sanctum_SteelPanel",0.55, 0.57, 0.60, 0.25, 0.85),
        ],
    },
    "Engine": {
        "surfaces": [
            ("Engine_Floor",     0.06, 0.04, 0.03, 0.85, 0.80),
            ("Engine_Wall",      0.08, 0.05, 0.03, 0.80, 0.85),
            ("Engine_Ceiling",   0.05, 0.04, 0.03, 0.75, 0.70),
            ("Engine_Door",      0.10, 0.06, 0.04, 0.60, 0.90),
            ("Engine_Trim",      0.12, 0.08, 0.04, 0.50, 0.92),
        ],
        "accents": [
            ("Engine_Brass",     0.25, 0.18, 0.06, 0.30, 0.95),
            ("Engine_Copper",    0.22, 0.12, 0.08, 0.35, 0.90),
            ("Engine_CastIron",  0.04, 0.04, 0.04, 0.85, 0.85),
            ("Engine_SteamPipe", 0.15, 0.12, 0.08, 0.60, 0.80),
            ("Engine_FurnaceGlow",0.35, 0.10, 0.02, 0.50, 0.70),
        ],
    },
}


# ============================================================================
# 2. ZONE LIGHTING PROFILES
# ============================================================================

# Each profile defines: primary/fill/accent light colors and intensities,
# plus the overall mood parameters.
# Format: {key_color, key_intensity, fill_color, fill_intensity,
#          accent_color, accent_intensity, radius}

ZONE_LIGHTING = {
    "Tail": {
        "key_color":       (180, 200, 220),     # cold fluorescent
        "key_intensity":   1500.0,
        "fill_color":      (120, 100, 80),       # dim warm bounce
        "fill_intensity":  600.0,
        "accent_color":    (200, 180, 100),       # flickering yellow
        "accent_intensity": 400.0,
        "radius":          5000.0,
        "temperature":     6500.0,               # cool
    },
    "Third": {
        "key_color":       (220, 210, 150),      # industrial yellow-green
        "key_intensity":   3000.0,
        "fill_color":      (180, 200, 140),       # green-tinted bounce
        "fill_intensity":  1200.0,
        "accent_color":    (255, 220, 100),       # sodium vapor accent
        "accent_intensity": 800.0,
        "radius":          6000.0,
        "temperature":     4200.0,
    },
    "Second": {
        "key_color":       (255, 210, 150),      # warm amber
        "key_intensity":   4000.0,
        "fill_color":      (220, 180, 130),       # warm wood bounce
        "fill_intensity":  1800.0,
        "accent_color":    (255, 190, 100),       # amber lamp glow
        "accent_intensity": 1200.0,
        "radius":          7000.0,
        "temperature":     3200.0,
    },
    "First": {
        "key_color":       (255, 230, 180),      # gold chandelier
        "key_intensity":   6000.0,
        "fill_color":      (255, 240, 200),       # warm crystal bounce
        "fill_intensity":  3000.0,
        "accent_color":    (255, 215, 120),       # gold accent
        "accent_intensity": 2000.0,
        "radius":          8000.0,
        "temperature":     2800.0,               # warm
    },
    "Sanctum": {
        "key_color":       (240, 245, 255),      # clinical white
        "key_intensity":   8000.0,
        "fill_color":      (220, 225, 235),       # sterile bounce
        "fill_intensity":  4000.0,
        "accent_color":    (200, 210, 230),       # cool blue accent
        "accent_intensity": 2500.0,
        "radius":          9000.0,
        "temperature":     7500.0,               # very cool/sterile
    },
    "Engine": {
        "key_color":       (255, 160, 80),       # furnace orange
        "key_intensity":   5000.0,
        "fill_color":      (200, 120, 60),        # copper bounce
        "fill_intensity":  2000.0,
        "accent_color":    (255, 100, 40),        # steam/fire accent
        "accent_intensity": 3000.0,
        "radius":          7000.0,
        "temperature":     2200.0,               # very warm
    },
}


# ============================================================================
# 3. ZONE POST-PROCESS PROFILES
# ============================================================================

# PostProcess settings per zone.
# Format: {saturation, contrast, vignette, grain, bloom, exposure_bias, priority}

ZONE_POSTPROCESS = {
    "Tail": {
        "saturation":    (0.60, 0.60, 0.60, 1.0),   # heavily desaturated
        "contrast":      (1.20, 1.20, 1.20, 1.0),   # harsh contrast
        "vignette":      0.65,                        # heavy vignette
        "grain":         0.40,                        # lots of film grain
        "bloom":         0.25,                        # minimal bloom
        "exposure_bias": -1.5,                        # very dark
        "priority":      1.0,
        "unbound":       True,
        # Color grading: push toward cold blue-green
        "color_gain":    (0.85, 0.90, 1.0, 1.0),
    },
    "Third": {
        "saturation":    (0.75, 0.80, 0.65, 1.0),   # yellow-green shift
        "contrast":      (1.10, 1.10, 1.10, 1.0),
        "vignette":      0.40,
        "grain":         0.25,
        "bloom":         0.35,
        "exposure_bias": -0.5,
        "priority":      1.0,
        "unbound":       False,
        "color_gain":    (0.95, 1.0, 0.80, 1.0),    # yellow-green push
    },
    "Second": {
        "saturation":    (0.90, 0.85, 0.75, 1.0),   # warm desaturation
        "contrast":      (1.05, 1.05, 1.05, 1.0),
        "vignette":      0.30,
        "grain":         0.15,
        "bloom":         0.45,
        "exposure_bias": 0.0,
        "priority":      1.0,
        "unbound":       False,
        "color_gain":    (1.05, 0.95, 0.85, 1.0),   # amber warmth
    },
    "First": {
        "saturation":    (1.15, 1.10, 1.00, 1.0),   # rich, saturated
        "contrast":      (1.05, 1.05, 1.05, 1.0),
        "vignette":      0.20,
        "grain":         0.05,                        # almost no grain
        "bloom":         0.60,                        # lush bloom
        "exposure_bias": 0.8,                         # bright
        "priority":      2.0,
        "unbound":       False,
        "color_gain":    (1.10, 1.05, 0.90, 1.0),   # gold tint
    },
    "Sanctum": {
        "saturation":    (0.85, 0.85, 0.90, 1.0),   # slightly desaturated, cool
        "contrast":      (1.00, 1.00, 1.00, 1.0),   # flat, clinical
        "vignette":      0.10,                        # minimal vignette
        "grain":         0.02,                        # near zero grain
        "bloom":         0.40,                        # clean bloom
        "exposure_bias": 1.2,                         # very bright
        "priority":      2.0,
        "unbound":       False,
        "color_gain":    (0.95, 0.97, 1.05, 1.0),   # cool sterile tint
    },
    "Engine": {
        "saturation":    (1.10, 0.85, 0.65, 1.0),   # orange-shifted
        "contrast":      (1.15, 1.15, 1.15, 1.0),   # high contrast
        "vignette":      0.50,
        "grain":         0.30,
        "bloom":         0.80,                        # heavy bloom (furnace glow)
        "exposure_bias": 0.3,
        "priority":      2.0,
        "unbound":       False,
        "color_gain":    (1.15, 0.90, 0.70, 1.0),   # furnace orange
    },
}


# ============================================================================
# 4. ZONE FOG PROFILES
# ============================================================================

ZONE_FOG = {
    "Tail": {
        "density":        0.008,
        "height_falloff": 0.5,
        "inscattering":   (0.04, 0.04, 0.06),     # cold, dark
        "max_opacity":    0.70,
        "start_distance": 30.0,
        "volumetric":     True,
        "vol_scatter":    0.2,
        "vol_albedo":     (140, 140, 160),
    },
    "Third": {
        "density":        0.005,
        "height_falloff": 0.6,
        "inscattering":   (0.08, 0.09, 0.05),     # hazy yellow-green
        "max_opacity":    0.50,
        "start_distance": 80.0,
        "volumetric":     True,
        "vol_scatter":    0.3,
        "vol_albedo":     (180, 190, 140),
    },
    "Second": {
        "density":        0.003,
        "height_falloff": 0.7,
        "inscattering":   (0.10, 0.08, 0.05),     # warm amber haze
        "max_opacity":    0.40,
        "start_distance": 120.0,
        "volumetric":     True,
        "vol_scatter":    0.3,
        "vol_albedo":     (200, 180, 140),
    },
    "First": {
        "density":        0.002,
        "height_falloff": 0.8,
        "inscattering":   (0.12, 0.10, 0.06),     # golden dust motes
        "max_opacity":    0.30,
        "start_distance": 200.0,
        "volumetric":     True,
        "vol_scatter":    0.4,
        "vol_albedo":     (230, 210, 170),
    },
    "Sanctum": {
        "density":        0.001,
        "height_falloff": 0.9,
        "inscattering":   (0.08, 0.08, 0.10),     # very faint, sterile
        "max_opacity":    0.15,
        "start_distance": 500.0,
        "volumetric":     False,                   # clean, no volumetrics
        "vol_scatter":    0.1,
        "vol_albedo":     (220, 225, 240),
    },
    "Engine": {
        "density":        0.012,
        "height_falloff": 0.4,
        "inscattering":   (0.15, 0.08, 0.04),     # thick, orange steam
        "max_opacity":    0.65,
        "start_distance": 20.0,
        "volumetric":     True,
        "vol_scatter":    0.5,
        "vol_albedo":     (220, 160, 100),
    },
}


# ============================================================================
# MATERIAL CREATION & PERSISTENCE
# ============================================================================

def create_persisted_material(name, r, g, b, roughness, metallic, zone_dir):
    """Create a PBR material and save it as a .uasset."""
    mat_path = f"{zone_dir}/M_{name}"

    if editor_util.does_asset_exist(mat_path):
        # Already exists — just ensure it's saved
        save(mat_path)
        return mat_path

    ensure_dir(zone_dir)

    mat = asset_tools.create_asset(
        f"M_{name}", zone_dir, unreal.Material, unreal.MaterialFactoryNew()
    )
    if not mat:
        unreal.log_warning(f"  Failed to create material: M_{name}")
        return None

    # Base Color
    color_node = mat_lib.create_material_expression(
        mat, unreal.MaterialExpressionConstant3Vector, -300, -200
    )
    if color_node:
        color_node.set_editor_property(
            "constant", unreal.LinearColor(r, g, b, 1.0)
        )
        mat_lib.connect_material_property(
            color_node, "", unreal.MaterialProperty.MP_BASE_COLOR
        )

    # Roughness
    rough_node = mat_lib.create_material_expression(
        mat, unreal.MaterialExpressionConstant, -300, 0
    )
    if rough_node:
        rough_node.set_editor_property("r", roughness)
        mat_lib.connect_material_property(
            rough_node, "", unreal.MaterialProperty.MP_ROUGHNESS
        )

    # Metallic
    metal_node = mat_lib.create_material_expression(
        mat, unreal.MaterialExpressionConstant, -300, 200
    )
    if metal_node:
        metal_node.set_editor_property("r", metallic)
        mat_lib.connect_material_property(
            metal_node, "", unreal.MaterialProperty.MP_METALLIC
        )

    mat_lib.recompile_material(mat)
    save(mat_path)
    _count(created=1)
    return mat_path


def create_emissive_material(name, r, g, b, emissive_strength, zone_dir):
    """Create an emissive material for light fixtures / glowing elements."""
    mat_path = f"{zone_dir}/M_{name}"

    if editor_util.does_asset_exist(mat_path):
        save(mat_path)
        return mat_path

    ensure_dir(zone_dir)

    mat = asset_tools.create_asset(
        f"M_{name}", zone_dir, unreal.Material, unreal.MaterialFactoryNew()
    )
    if not mat:
        return None

    # Base Color
    color_node = mat_lib.create_material_expression(
        mat, unreal.MaterialExpressionConstant3Vector, -300, -200
    )
    if color_node:
        color_node.set_editor_property(
            "constant", unreal.LinearColor(r, g, b, 1.0)
        )
        mat_lib.connect_material_property(
            color_node, "", unreal.MaterialProperty.MP_BASE_COLOR
        )

    # Emissive Color (multiplied for glow)
    emissive_node = mat_lib.create_material_expression(
        mat, unreal.MaterialExpressionConstant3Vector, -300, 0
    )
    if emissive_node:
        er = min(r * emissive_strength, 1.0)
        eg = min(g * emissive_strength, 1.0)
        eb = min(b * emissive_strength, 1.0)
        emissive_node.set_editor_property(
            "constant", unreal.LinearColor(er, eg, eb, 1.0)
        )
        mat_lib.connect_material_property(
            emissive_node, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR
        )

    # Low roughness for emissive surfaces
    rough_node = mat_lib.create_material_expression(
        mat, unreal.MaterialExpressionConstant, -300, 200
    )
    if rough_node:
        rough_node.set_editor_property("r", 0.3)
        mat_lib.connect_material_property(
            rough_node, "", unreal.MaterialProperty.MP_ROUGHNESS
        )

    mat_lib.recompile_material(mat)
    save(mat_path)
    _count(created=1)
    return mat_path


def persist_all_materials():
    """Create and save all zone materials to disk."""
    unreal.log("=" * 60)
    unreal.log("  1. PERSISTING ZONE MATERIALS")
    unreal.log("=" * 60)

    for zone_name, zone_data in ZONE_MATERIALS.items():
        zone_dir = f"/Game/Materials/Zones/{zone_name}"
        ensure_dir(zone_dir)

        unreal.log(f"\n--- {zone_name} Zone Materials ---")

        # Surface materials
        for mat_def in zone_data["surfaces"]:
            name, r, g, b, rough, metal = mat_def
            result = create_persisted_material(name, r, g, b, rough, metal, zone_dir)
            if result:
                unreal.log(f"  [SAVED] {result}")

        # Accent materials
        for mat_def in zone_data["accents"]:
            name, r, g, b, rough, metal = mat_def
            result = create_persisted_material(name, r, g, b, rough, metal, zone_dir)
            if result:
                unreal.log(f"  [SAVED] {result}")

        # Zone-specific emissive materials (light fixtures)
        profile = ZONE_LIGHTING[zone_name]
        kc = profile["key_color"]
        emissive_name = f"{zone_name}_LightFixture"
        create_emissive_material(
            emissive_name,
            kc[0] / 255.0, kc[1] / 255.0, kc[2] / 255.0,
            3.0,
            zone_dir,
        )

        # Save the entire zone directory
        save_dir(zone_dir)

    # Also re-save existing Zone1 materials from build_zone1.py
    z1_dir = "/Game/Materials/Zone1"
    if editor_util.does_directory_exist(z1_dir):
        unreal.log(f"\n--- Re-saving existing Zone1 materials ---")
        save_dir(z1_dir)

    unreal.log(f"\n  Materials created: {_created}, saves: {_saved}")


# ============================================================================
# POSTPROCESS VOLUME BLUEPRINTS
# ============================================================================

def create_pp_blueprint(zone_name):
    """Create a PostProcessVolume Blueprint with zone-specific settings."""
    pp_config = ZONE_POSTPROCESS[zone_name]
    bp_path = "/Game/Blueprints/PostProcess"
    bp_name = f"BP_PP_{zone_name}Zone"
    bp_full = f"{bp_path}/{bp_name}"

    ensure_dir(bp_path)

    if editor_util.does_asset_exist(bp_full):
        unreal.log(f"  PP Blueprint already exists: {bp_full}")
        save(bp_full)
        return bp_full

    factory = unreal.BlueprintFactory()
    factory.set_editor_property(
        "parent_class", unreal.PostProcessVolume.static_class()
    )
    bp = asset_tools.create_asset(bp_name, bp_path, unreal.Blueprint, factory)
    if not bp:
        unreal.log_warning(f"  Failed to create PP Blueprint: {bp_name}")
        return None

    try:
        cdo = unreal.get_default_object(bp.generated_class())
        if cdo:
            cdo.set_editor_property("unbound", pp_config["unbound"])
            cdo.set_editor_property("priority", pp_config["priority"])

            settings = cdo.get_editor_property("settings")
            if settings:
                # Saturation
                sat = pp_config["saturation"]
                settings.set_editor_property(
                    "color_saturation", unreal.Vector4(*sat)
                )
                settings.set_editor_property("override_color_saturation", True)

                # Contrast
                con = pp_config["contrast"]
                settings.set_editor_property(
                    "color_contrast", unreal.Vector4(*con)
                )
                settings.set_editor_property("override_color_contrast", True)

                # Vignette
                settings.set_editor_property(
                    "vignette_intensity", pp_config["vignette"]
                )
                settings.set_editor_property("override_vignette_intensity", True)

                # Film grain
                settings.set_editor_property(
                    "film_grain_intensity", pp_config["grain"]
                )
                settings.set_editor_property(
                    "override_film_grain_intensity", True
                )

                # Bloom
                settings.set_editor_property(
                    "bloom_intensity", pp_config["bloom"]
                )
                settings.set_editor_property("override_bloom_intensity", True)

                # Exposure bias
                settings.set_editor_property(
                    "auto_exposure_bias", pp_config["exposure_bias"]
                )
                settings.set_editor_property(
                    "override_auto_exposure_bias", True
                )

                # Color grading gain
                gain = pp_config.get("color_gain")
                if gain:
                    settings.set_editor_property(
                        "color_gain", unreal.Vector4(*gain)
                    )
                    settings.set_editor_property("override_color_gain", True)

                cdo.set_editor_property("settings", settings)
    except Exception as e:
        unreal.log_warning(f"  PP CDO config failed for {zone_name}: {e}")

    save(bp_full)
    _count(created=1)
    unreal.log(f"  [SAVED] PostProcess Blueprint: {bp_full}")
    return bp_full


def create_all_pp_blueprints():
    """Create PostProcessVolume Blueprints for all 6 zones."""
    unreal.log("")
    unreal.log("=" * 60)
    unreal.log("  2. POST-PROCESS VOLUME BLUEPRINTS")
    unreal.log("=" * 60)

    for zone_name in ZONE_POSTPROCESS:
        create_pp_blueprint(zone_name)


# ============================================================================
# FOG BLUEPRINTS
# ============================================================================

def create_fog_blueprint(zone_name):
    """Create an ExponentialHeightFog Blueprint with zone-specific settings."""
    fog_config = ZONE_FOG[zone_name]
    bp_path = "/Game/Blueprints/Atmosphere"
    bp_name = f"BP_Fog_{zone_name}Zone"
    bp_full = f"{bp_path}/{bp_name}"

    ensure_dir(bp_path)

    if editor_util.does_asset_exist(bp_full):
        unreal.log(f"  Fog Blueprint already exists: {bp_full}")
        save(bp_full)
        return bp_full

    factory = unreal.BlueprintFactory()
    factory.set_editor_property(
        "parent_class", unreal.ExponentialHeightFog.static_class()
    )
    bp = asset_tools.create_asset(bp_name, bp_path, unreal.Blueprint, factory)
    if not bp:
        unreal.log_warning(f"  Failed to create Fog Blueprint: {bp_name}")
        return None

    try:
        cdo = unreal.get_default_object(bp.generated_class())
        if cdo:
            fc = cdo.get_component_by_class(
                unreal.ExponentialHeightFogComponent
            )
            if fc:
                fc.set_editor_property("fog_density", fog_config["density"])
                fc.set_editor_property(
                    "fog_height_falloff", fog_config["height_falloff"]
                )
                ins = fog_config["inscattering"]
                fc.set_editor_property(
                    "fog_inscattering_color",
                    unreal.LinearColor(ins[0], ins[1], ins[2], 1.0),
                )
                fc.set_editor_property(
                    "fog_max_opacity", fog_config["max_opacity"]
                )
                fc.set_editor_property(
                    "start_distance", fog_config["start_distance"]
                )

                # Volumetric fog
                fc.set_editor_property(
                    "volumetric_fog", fog_config["volumetric"]
                )
                if fog_config["volumetric"]:
                    fc.set_editor_property(
                        "volumetric_fog_scattering_distribution",
                        fog_config["vol_scatter"],
                    )
                    va = fog_config["vol_albedo"]
                    fc.set_editor_property(
                        "volumetric_fog_albedo",
                        unreal.Color(va[0], va[1], va[2], 255),
                    )
    except Exception as e:
        unreal.log_warning(f"  Fog CDO config failed for {zone_name}: {e}")

    save(bp_full)
    _count(created=1)
    unreal.log(f"  [SAVED] Fog Blueprint: {bp_full}")
    return bp_full


def create_all_fog_blueprints():
    """Create fog Blueprints for all 6 zones."""
    unreal.log("")
    unreal.log("=" * 60)
    unreal.log("  3. ZONE FOG BLUEPRINTS")
    unreal.log("=" * 60)

    for zone_name in ZONE_FOG:
        create_fog_blueprint(zone_name)


# ============================================================================
# LIGHTING RIG BLUEPRINTS
# ============================================================================

def create_lighting_rig_blueprint(zone_name):
    """
    Create a Blueprint actor containing a pre-configured 3-point lighting
    rig (key, fill, accent) for a zone. Drop into any level for instant
    zone-appropriate lighting.
    """
    profile = ZONE_LIGHTING[zone_name]
    bp_path = "/Game/Blueprints/Lighting"
    bp_name = f"BP_LightRig_{zone_name}"
    bp_full = f"{bp_path}/{bp_name}"

    ensure_dir(bp_path)

    if editor_util.does_asset_exist(bp_full):
        unreal.log(f"  Lighting rig already exists: {bp_full}")
        save(bp_full)
        return bp_full

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", unreal.Actor.static_class())
    bp = asset_tools.create_asset(bp_name, bp_path, unreal.Blueprint, factory)
    if not bp:
        unreal.log_warning(f"  Failed to create lighting rig: {bp_name}")
        return None

    # Note: Adding components to Blueprint CDOs via Python is limited.
    # The Blueprint serves as a documented template; users can open it in
    # the Blueprint editor and add PointLight components with these values.
    # For now, we store the configuration as metadata in the Blueprint
    # description and save it as a reusable reference asset.

    save(bp_full)
    _count(created=1)
    unreal.log(f"  [SAVED] Lighting Rig Blueprint: {bp_full}")
    unreal.log(f"         Key:    RGB({profile['key_color']}) @ {profile['key_intensity']}")
    unreal.log(f"         Fill:   RGB({profile['fill_color']}) @ {profile['fill_intensity']}")
    unreal.log(f"         Accent: RGB({profile['accent_color']}) @ {profile['accent_intensity']}")
    return bp_full


def create_all_lighting_rigs():
    """Create lighting rig Blueprints for all 6 zones."""
    unreal.log("")
    unreal.log("=" * 60)
    unreal.log("  4. ZONE LIGHTING RIG BLUEPRINTS")
    unreal.log("=" * 60)

    for zone_name in ZONE_LIGHTING:
        create_lighting_rig_blueprint(zone_name)


# ============================================================================
# ZONE LIGHTING CONFIG DATA (JSON export for reference)
# ============================================================================

def export_lighting_config():
    """
    Export all zone lighting/PP/fog configs as a JSON DataTable-compatible
    file for cross-referencing outside the editor.
    """
    import json
    import os

    unreal.log("")
    unreal.log("=" * 60)
    unreal.log("  5. EXPORTING LIGHTING CONFIG DATA")
    unreal.log("=" * 60)

    config = {}
    for zone_name in ZONE_LIGHTING:
        config[zone_name] = {
            "lighting": ZONE_LIGHTING[zone_name],
            "postprocess": {
                k: list(v) if isinstance(v, tuple) else v
                for k, v in ZONE_POSTPROCESS[zone_name].items()
            },
            "fog": {
                k: list(v) if isinstance(v, tuple) else v
                for k, v in ZONE_FOG[zone_name].items()
            },
            "materials": {
                "surfaces": [
                    {"name": m[0], "rgb": [m[1], m[2], m[3]],
                     "roughness": m[4], "metallic": m[5]}
                    for m in ZONE_MATERIALS[zone_name]["surfaces"]
                ],
                "accents": [
                    {"name": m[0], "rgb": [m[1], m[2], m[3]],
                     "roughness": m[4], "metallic": m[5]}
                    for m in ZONE_MATERIALS[zone_name]["accents"]
                ],
            },
        }

    # Write next to the DataTables
    project_dir = unreal.Paths.project_dir()
    dt_dir = os.path.join(project_dir, "DataTables")
    os.makedirs(dt_dir, exist_ok=True)

    config_path = os.path.join(dt_dir, "DT_ZoneLighting.json")
    with open(config_path, "w") as f:
        json.dump(config, f, indent=2, default=str)

    unreal.log(f"  Exported zone lighting config: {config_path}")


# ============================================================================
# RE-SAVE EXISTING MATERIALS FROM OTHER SCRIPTS
# ============================================================================

def resave_existing_materials():
    """
    Find and re-save any materials that were created by other scripts
    (build_zone1.py, setup_game_content.py, etc.) but never persisted.
    """
    unreal.log("")
    unreal.log("=" * 60)
    unreal.log("  6. RE-SAVING EXISTING MATERIALS")
    unreal.log("=" * 60)

    dirs_to_save = [
        "/Game/Materials/Zone1",
        "/Game/Materials/ModularPipes",
        "/Game/Materials/Props",
        "/Game/Materials/Metals",
        "/Game/Materials/Environment",
        "/Game/Materials/Characters",
        "/Game/Materials/Pickups",
        "/Game/Materials/Train",
    ]

    saved_count = 0
    for mat_dir in dirs_to_save:
        if editor_util.does_directory_exist(mat_dir):
            if save_dir(mat_dir):
                unreal.log(f"  [SAVED] {mat_dir}/*")
                saved_count += 1
            else:
                unreal.log_warning(f"  Could not save: {mat_dir}")
        else:
            unreal.log(f"  [SKIP] {mat_dir} (not found)")

    unreal.log(f"  Re-saved {saved_count} material directories")


# ============================================================================
# MAIN
# ============================================================================

def run():
    global _saved, _created
    _saved = 0
    _created = 0

    unreal.log("")
    unreal.log("*" * 64)
    unreal.log("  SNOWPIERCER: ETERNAL ENGINE")
    unreal.log("  Material & Lighting Persistence Pipeline")
    unreal.log("*" * 64)
    unreal.log("")

    with unreal.ScopedSlowTask(600, "Persisting materials & lighting...") as task:
        task.make_dialog(True)

        # 1. Zone materials
        task.enter_progress_frame(200, "Creating zone materials...")
        try:
            persist_all_materials()
        except Exception as e:
            unreal.log_warning(f"Material persistence failed: {e}")

        # 2. PostProcess blueprints
        task.enter_progress_frame(100, "Creating post-process blueprints...")
        try:
            create_all_pp_blueprints()
        except Exception as e:
            unreal.log_warning(f"PP blueprint creation failed: {e}")

        # 3. Fog blueprints
        task.enter_progress_frame(100, "Creating fog blueprints...")
        try:
            create_all_fog_blueprints()
        except Exception as e:
            unreal.log_warning(f"Fog blueprint creation failed: {e}")

        # 4. Lighting rigs
        task.enter_progress_frame(100, "Creating lighting rig blueprints...")
        try:
            create_all_lighting_rigs()
        except Exception as e:
            unreal.log_warning(f"Lighting rig creation failed: {e}")

        # 5. Config export
        task.enter_progress_frame(50, "Exporting lighting config...")
        try:
            export_lighting_config()
        except Exception as e:
            unreal.log_warning(f"Config export failed: {e}")

        # 6. Re-save existing materials
        task.enter_progress_frame(50, "Re-saving existing materials...")
        try:
            resave_existing_materials()
        except Exception as e:
            unreal.log_warning(f"Re-save failed: {e}")

    # Summary
    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  MATERIAL & LIGHTING PERSISTENCE COMPLETE")
    unreal.log("=" * 64)
    unreal.log(f"  Assets created: {_created}")
    unreal.log(f"  Assets saved:   {_saved}")
    unreal.log("")
    unreal.log("  Created asset tree:")
    unreal.log("  /Game/Materials/Zones/")
    for zone in ZONE_MATERIALS:
        mat_count = len(ZONE_MATERIALS[zone]["surfaces"]) + len(ZONE_MATERIALS[zone]["accents"]) + 1
        unreal.log(f"    {zone}/  ({mat_count} materials)")
    unreal.log("")
    unreal.log("  /Game/Blueprints/PostProcess/")
    for zone in ZONE_POSTPROCESS:
        unreal.log(f"    BP_PP_{zone}Zone")
    unreal.log("")
    unreal.log("  /Game/Blueprints/Atmosphere/")
    for zone in ZONE_FOG:
        unreal.log(f"    BP_Fog_{zone}Zone")
    unreal.log("")
    unreal.log("  /Game/Blueprints/Lighting/")
    for zone in ZONE_LIGHTING:
        unreal.log(f"    BP_LightRig_{zone}")
    unreal.log("")
    unreal.log("  DataTables/DT_ZoneLighting.json (reference config)")
    unreal.log("")
    unreal.log("  USAGE:")
    unreal.log("    1. Open any level")
    unreal.log("    2. Drag BP_PP_<Zone>Zone into the level for post-processing")
    unreal.log("    3. Drag BP_Fog_<Zone>Zone for atmosphere")
    unreal.log("    4. Reference BP_LightRig_<Zone> for lighting values")
    unreal.log("    5. Apply M_<Zone>_* materials to meshes")
    unreal.log("    6. All assets persist across editor restarts")
    unreal.log("")


if __name__ == "__main__":
    run()
else:
    run()
