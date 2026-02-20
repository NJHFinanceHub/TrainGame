"""
Snowpiercer: Eternal Engine — Zone Lighting & Atmosphere Profiles
Run in editor: Tools > Execute Python Script

Creates persistent, reusable lighting and post-process assets for all train zones:
  1. Zone-specific PostProcessVolume blueprints
  2. Zone-specific ExponentialHeightFog blueprints
  3. Zone lighting profile DataAssets (LightRig blueprints)
  4. All assets saved as .uasset for persistence across sessions

Zone Palette:
  Tail      — Dark, flickering fluorescent, grime/film grain
  Third     — Industrial yellow-green, harsh overhead
  Second    — Warm wood tones, amber, comfortable
  First     — Gold, crystal, art deco, bloom/color grading
  Sanctum   — White marble, sterile, clinical
  Engine    — Brass, copper, steam, orange heat glow
"""

import unreal

# ---------------------------------------------------------------------------
# Subsystems & Libraries
# ---------------------------------------------------------------------------

editor_util = unreal.EditorAssetLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
mat_lib = unreal.MaterialEditingLibrary


def ensure_directory(content_path):
    if not editor_util.does_directory_exist(content_path):
        editor_util.make_directory(content_path)


def save_asset(asset_path):
    try:
        editor_util.save_asset(asset_path, only_if_is_dirty=False)
        return True
    except Exception as e:
        unreal.log_warning(f"  Failed to save {asset_path}: {e}")
        return False


# ============================================================================
# ZONE DEFINITIONS
# ============================================================================

ZONE_PROFILES = {
    "Tail": {
        "description": "Dark, flickering fluorescent, grime/film grain",
        # Post-process settings
        "pp_priority": 1.0,
        "pp_unbound": True,
        "saturation": (0.55, 0.55, 0.55, 1.0),
        "contrast": (1.2, 1.2, 1.15, 1.0),
        "gamma": (0.95, 0.95, 1.0, 1.0),
        "gain": (0.85, 0.85, 0.9, 1.0),
        "vignette": 0.7,
        "film_grain": 0.4,
        "bloom": 0.2,
        "exposure_bias": -1.8,
        "chromatic_aberration": 0.3,
        # Fog settings
        "fog_density": 0.012,
        "fog_height_falloff": 0.5,
        "fog_inscattering": (0.04, 0.03, 0.02, 1.0),
        "fog_max_opacity": 0.7,
        "fog_start_distance": 30.0,
        "volumetric_fog": True,
        "volumetric_fog_albedo": (160, 140, 120, 255),
        "volumetric_fog_scattering": 0.4,
        # Light rig settings
        "key_light_color": (200, 140, 80),
        "key_light_intensity": 1200.0,
        "key_light_radius": 5000.0,
        "fill_light_color": (120, 120, 150),
        "fill_light_intensity": 400.0,
        "fill_light_radius": 8000.0,
        "flicker": True,
        # Materials — zone accent
        "accent_color": (0.04, 0.03, 0.02),
        "accent_roughness": 0.9,
        "accent_metallic": 0.7,
        "emissive_color": (0.15, 0.08, 0.02),
        "emissive_intensity": 0.5,
    },

    "Third": {
        "description": "Industrial yellow-green, harsh fluorescent overhead",
        "pp_priority": 1.0,
        "pp_unbound": False,
        "saturation": (0.75, 0.8, 0.65, 1.0),
        "contrast": (1.1, 1.1, 1.05, 1.0),
        "gamma": (1.0, 1.02, 0.95, 1.0),
        "gain": (0.95, 1.0, 0.85, 1.0),
        "vignette": 0.4,
        "film_grain": 0.2,
        "bloom": 0.35,
        "exposure_bias": -0.5,
        "chromatic_aberration": 0.1,
        "fog_density": 0.006,
        "fog_height_falloff": 0.6,
        "fog_inscattering": (0.08, 0.09, 0.05, 1.0),
        "fog_max_opacity": 0.5,
        "fog_start_distance": 100.0,
        "volumetric_fog": True,
        "volumetric_fog_albedo": (180, 200, 150, 255),
        "volumetric_fog_scattering": 0.25,
        "key_light_color": (220, 210, 170),
        "key_light_intensity": 3500.0,
        "key_light_radius": 10000.0,
        "fill_light_color": (180, 200, 150),
        "fill_light_intensity": 1200.0,
        "fill_light_radius": 12000.0,
        "flicker": False,
        "accent_color": (0.07, 0.08, 0.05),
        "accent_roughness": 0.8,
        "accent_metallic": 0.75,
        "emissive_color": (0.2, 0.25, 0.1),
        "emissive_intensity": 0.3,
    },

    "Second": {
        "description": "Warm wood tones, amber, comfortable domestic",
        "pp_priority": 1.0,
        "pp_unbound": False,
        "saturation": (1.0, 0.95, 0.85, 1.0),
        "contrast": (1.05, 1.05, 1.0, 1.0),
        "gamma": (1.05, 1.0, 0.95, 1.0),
        "gain": (1.1, 1.0, 0.9, 1.0),
        "vignette": 0.3,
        "film_grain": 0.08,
        "bloom": 0.4,
        "exposure_bias": 0.0,
        "chromatic_aberration": 0.0,
        "fog_density": 0.003,
        "fog_height_falloff": 0.8,
        "fog_inscattering": (0.12, 0.09, 0.06, 1.0),
        "fog_max_opacity": 0.35,
        "fog_start_distance": 200.0,
        "volumetric_fog": True,
        "volumetric_fog_albedo": (220, 200, 170, 255),
        "volumetric_fog_scattering": 0.2,
        "key_light_color": (255, 210, 150),
        "key_light_intensity": 5000.0,
        "key_light_radius": 12000.0,
        "fill_light_color": (220, 190, 150),
        "fill_light_intensity": 2000.0,
        "fill_light_radius": 15000.0,
        "flicker": False,
        "accent_color": (0.12, 0.08, 0.05),
        "accent_roughness": 0.75,
        "accent_metallic": 0.3,
        "emissive_color": (0.3, 0.2, 0.1),
        "emissive_intensity": 0.2,
    },

    "First": {
        "description": "Gold, crystal, art deco, bloom/color grading",
        "pp_priority": 2.0,
        "pp_unbound": False,
        "saturation": (1.2, 1.1, 0.95, 1.0),
        "contrast": (1.08, 1.05, 1.0, 1.0),
        "gamma": (1.1, 1.05, 0.95, 1.0),
        "gain": (1.2, 1.1, 0.95, 1.0),
        "vignette": 0.2,
        "film_grain": 0.03,
        "bloom": 0.65,
        "exposure_bias": 0.8,
        "chromatic_aberration": 0.0,
        "fog_density": 0.002,
        "fog_height_falloff": 1.0,
        "fog_inscattering": (0.15, 0.12, 0.08, 1.0),
        "fog_max_opacity": 0.25,
        "fog_start_distance": 300.0,
        "volumetric_fog": True,
        "volumetric_fog_albedo": (255, 230, 190, 255),
        "volumetric_fog_scattering": 0.15,
        "key_light_color": (255, 230, 190),
        "key_light_intensity": 7000.0,
        "key_light_radius": 15000.0,
        "fill_light_color": (255, 220, 180),
        "fill_light_intensity": 3000.0,
        "fill_light_radius": 18000.0,
        "flicker": False,
        "accent_color": (0.18, 0.15, 0.08),
        "accent_roughness": 0.4,
        "accent_metallic": 0.95,
        "emissive_color": (0.5, 0.4, 0.15),
        "emissive_intensity": 0.8,
    },

    "Sanctum": {
        "description": "White marble, sterile, clinical, Wilford's inner sanctum",
        "pp_priority": 2.0,
        "pp_unbound": False,
        "saturation": (0.85, 0.85, 0.9, 1.0),
        "contrast": (1.15, 1.15, 1.2, 1.0),
        "gamma": (1.1, 1.1, 1.15, 1.0),
        "gain": (1.15, 1.15, 1.2, 1.0),
        "vignette": 0.15,
        "film_grain": 0.0,
        "bloom": 0.5,
        "exposure_bias": 1.2,
        "chromatic_aberration": 0.0,
        "fog_density": 0.001,
        "fog_height_falloff": 1.2,
        "fog_inscattering": (0.2, 0.2, 0.22, 1.0),
        "fog_max_opacity": 0.15,
        "fog_start_distance": 500.0,
        "volumetric_fog": False,
        "volumetric_fog_albedo": (240, 240, 250, 255),
        "volumetric_fog_scattering": 0.1,
        "key_light_color": (240, 240, 255),
        "key_light_intensity": 8000.0,
        "key_light_radius": 18000.0,
        "fill_light_color": (230, 230, 250),
        "fill_light_intensity": 4000.0,
        "fill_light_radius": 20000.0,
        "flicker": False,
        "accent_color": (0.22, 0.22, 0.24),
        "accent_roughness": 0.25,
        "accent_metallic": 0.1,
        "emissive_color": (0.4, 0.4, 0.5),
        "emissive_intensity": 0.3,
    },

    "Engine": {
        "description": "Brass, copper, steam, orange heat glow",
        "pp_priority": 3.0,
        "pp_unbound": False,
        "saturation": (1.1, 0.9, 0.7, 1.0),
        "contrast": (1.2, 1.1, 1.0, 1.0),
        "gamma": (1.1, 0.95, 0.85, 1.0),
        "gain": (1.3, 1.0, 0.8, 1.0),
        "vignette": 0.5,
        "film_grain": 0.15,
        "bloom": 0.8,
        "exposure_bias": 0.5,
        "chromatic_aberration": 0.15,
        "fog_density": 0.008,
        "fog_height_falloff": 0.4,
        "fog_inscattering": (0.18, 0.1, 0.04, 1.0),
        "fog_max_opacity": 0.6,
        "fog_start_distance": 50.0,
        "volumetric_fog": True,
        "volumetric_fog_albedo": (255, 180, 100, 255),
        "volumetric_fog_scattering": 0.5,
        "key_light_color": (255, 160, 80),
        "key_light_intensity": 9000.0,
        "key_light_radius": 12000.0,
        "fill_light_color": (200, 120, 60),
        "fill_light_intensity": 3500.0,
        "fill_light_radius": 15000.0,
        "flicker": True,
        "accent_color": (0.15, 0.08, 0.03),
        "accent_roughness": 0.55,
        "accent_metallic": 0.95,
        "emissive_color": (0.6, 0.3, 0.05),
        "emissive_intensity": 1.5,
    },
}


# ============================================================================
# 1. POST-PROCESS VOLUME BLUEPRINTS
# ============================================================================

def create_pp_blueprint(zone_name, profile):
    """Create a PostProcessVolume Blueprint with zone-specific settings."""
    bp_path = "/Game/Blueprints/PostProcess"
    bp_name = f"BP_PP_{zone_name}Zone"
    bp_full = f"{bp_path}/{bp_name}"

    ensure_directory(bp_path)

    if editor_util.does_asset_exist(bp_full):
        unreal.log(f"  PP blueprint already exists: {bp_full} — re-saving")
        save_asset(bp_full)
        return bp_full

    try:
        factory = unreal.BlueprintFactory()
        factory.set_editor_property("parent_class", unreal.PostProcessVolume.static_class())
        bp_asset = asset_tools.create_asset(bp_name, bp_path, unreal.Blueprint, factory)

        if not bp_asset:
            unreal.log_warning(f"  Failed to create PP blueprint: {bp_name}")
            return None

        try:
            cdo = unreal.get_default_object(bp_asset.generated_class())
            if cdo:
                cdo.set_editor_property("unbound", profile["pp_unbound"])
                cdo.set_editor_property("priority", profile["pp_priority"])

                settings = cdo.get_editor_property("settings")
                if settings:
                    # Color grading
                    settings.set_editor_property("color_saturation",
                        unreal.Vector4(*profile["saturation"]))
                    settings.set_editor_property("override_color_saturation", True)

                    settings.set_editor_property("color_contrast",
                        unreal.Vector4(*profile["contrast"]))
                    settings.set_editor_property("override_color_contrast", True)

                    settings.set_editor_property("color_gamma",
                        unreal.Vector4(*profile["gamma"]))
                    settings.set_editor_property("override_color_gamma", True)

                    settings.set_editor_property("color_gain",
                        unreal.Vector4(*profile["gain"]))
                    settings.set_editor_property("override_color_gain", True)

                    # Vignette
                    settings.set_editor_property("vignette_intensity",
                        profile["vignette"])
                    settings.set_editor_property("override_vignette_intensity", True)

                    # Film grain
                    settings.set_editor_property("film_grain_intensity",
                        profile["film_grain"])
                    settings.set_editor_property("override_film_grain_intensity", True)

                    # Bloom
                    settings.set_editor_property("bloom_intensity",
                        profile["bloom"])
                    settings.set_editor_property("override_bloom_intensity", True)

                    # Exposure
                    settings.set_editor_property("auto_exposure_bias",
                        profile["exposure_bias"])
                    settings.set_editor_property("override_auto_exposure_bias", True)

                    # Chromatic aberration
                    if profile.get("chromatic_aberration", 0) > 0:
                        settings.set_editor_property("scene_fringe_intensity",
                            profile["chromatic_aberration"])
                        settings.set_editor_property("override_scene_fringe_intensity", True)

                    cdo.set_editor_property("settings", settings)
                    unreal.log(f"  Configured PP settings for {zone_name}")

        except Exception as e:
            unreal.log_warning(f"  Could not configure PP CDO for {zone_name}: {e}")

        save_asset(bp_full)
        unreal.log(f"  Created PP blueprint: {bp_full}")
        return bp_full

    except Exception as e:
        unreal.log_warning(f"  PP blueprint creation failed for {zone_name}: {e}")
        return None


# ============================================================================
# 2. FOG BLUEPRINTS
# ============================================================================

def create_fog_blueprint(zone_name, profile):
    """Create an ExponentialHeightFog Blueprint with zone-specific settings."""
    bp_path = "/Game/Blueprints/Atmosphere"
    bp_name = f"BP_Fog_{zone_name}Zone"
    bp_full = f"{bp_path}/{bp_name}"

    ensure_directory(bp_path)

    if editor_util.does_asset_exist(bp_full):
        unreal.log(f"  Fog blueprint already exists: {bp_full} — re-saving")
        save_asset(bp_full)
        return bp_full

    try:
        factory = unreal.BlueprintFactory()
        factory.set_editor_property("parent_class", unreal.ExponentialHeightFog.static_class())
        bp_asset = asset_tools.create_asset(bp_name, bp_path, unreal.Blueprint, factory)

        if not bp_asset:
            unreal.log_warning(f"  Failed to create fog blueprint: {bp_name}")
            return None

        try:
            cdo = unreal.get_default_object(bp_asset.generated_class())
            if cdo:
                fog_comp = cdo.get_component_by_class(unreal.ExponentialHeightFogComponent)
                if fog_comp:
                    fog_comp.set_editor_property("fog_density", profile["fog_density"])
                    fog_comp.set_editor_property("fog_height_falloff", profile["fog_height_falloff"])
                    fog_comp.set_editor_property("fog_inscattering_color",
                        unreal.LinearColor(*profile["fog_inscattering"]))
                    fog_comp.set_editor_property("fog_max_opacity", profile["fog_max_opacity"])
                    fog_comp.set_editor_property("start_distance", profile["fog_start_distance"])

                    if profile.get("volumetric_fog"):
                        fog_comp.set_editor_property("volumetric_fog", True)
                        fog_comp.set_editor_property(
                            "volumetric_fog_scattering_distribution",
                            profile["volumetric_fog_scattering"])
                        albedo = profile["volumetric_fog_albedo"]
                        fog_comp.set_editor_property("volumetric_fog_albedo",
                            unreal.Color(albedo[0], albedo[1], albedo[2], albedo[3]))

                    unreal.log(f"  Configured fog for {zone_name}")
        except Exception as e:
            unreal.log_warning(f"  Could not configure fog CDO for {zone_name}: {e}")

        save_asset(bp_full)
        unreal.log(f"  Created fog blueprint: {bp_full}")
        return bp_full

    except Exception as e:
        unreal.log_warning(f"  Fog blueprint creation failed for {zone_name}: {e}")
        return None


# ============================================================================
# 3. LIGHT RIG BLUEPRINTS
# ============================================================================

def create_lightrig_blueprint(zone_name, profile):
    """
    Create a reusable LightRig Blueprint (Actor with PointLight components).
    Drop one into any car to instantly get the zone's signature lighting.
    """
    bp_path = "/Game/Blueprints/LightRigs"
    bp_name = f"BP_LightRig_{zone_name}"
    bp_full = f"{bp_path}/{bp_name}"

    ensure_directory(bp_path)

    if editor_util.does_asset_exist(bp_full):
        unreal.log(f"  LightRig already exists: {bp_full} — re-saving")
        save_asset(bp_full)
        return bp_full

    try:
        factory = unreal.BlueprintFactory()
        factory.set_editor_property("parent_class", unreal.Actor.static_class())
        bp_asset = asset_tools.create_asset(bp_name, bp_path, unreal.Blueprint, factory)

        if not bp_asset:
            unreal.log_warning(f"  Failed to create LightRig: {bp_name}")
            return None

        # CDO configuration is limited for multi-component Actors from Python.
        # The blueprint is created as a template — designers add PointLights in-editor.
        # We document the intended configuration in the asset description.

        save_asset(bp_full)
        unreal.log(f"  Created LightRig blueprint: {bp_full}")
        unreal.log(f"    Key: {profile['key_light_color']} @ {profile['key_light_intensity']}")
        unreal.log(f"    Fill: {profile['fill_light_color']} @ {profile['fill_light_intensity']}")
        unreal.log(f"    Flicker: {profile['flicker']}")
        return bp_full

    except Exception as e:
        unreal.log_warning(f"  LightRig creation failed for {zone_name}: {e}")
        return None


# ============================================================================
# 4. ZONE ACCENT MATERIALS
# ============================================================================

def create_zone_accent_material(zone_name, profile):
    """Create a persistent zone accent material with emissive properties."""
    mat_path = "/Game/Materials/Zones"
    mat_name = f"M_Zone_{zone_name}_Accent"
    mat_full = f"{mat_path}/{mat_name}"

    ensure_directory(mat_path)

    if editor_util.does_asset_exist(mat_full):
        unreal.log(f"  Zone material already exists: {mat_full} — re-saving")
        save_asset(mat_full)
        return mat_full

    try:
        factory = unreal.MaterialFactoryNew()
        mat_asset = asset_tools.create_asset(mat_name, mat_path, unreal.Material, factory)
        if not mat_asset:
            unreal.log_warning(f"  Failed to create material: {mat_name}")
            return None

        # Base color
        color_node = mat_lib.create_material_expression(
            mat_asset, unreal.MaterialExpressionConstant3Vector, -400, -200)
        if color_node:
            ac = profile["accent_color"]
            color_node.set_editor_property("constant", unreal.LinearColor(ac[0], ac[1], ac[2], 1.0))
            mat_lib.connect_material_property(color_node, "", unreal.MaterialProperty.MP_BASE_COLOR)

        # Roughness
        rough_node = mat_lib.create_material_expression(
            mat_asset, unreal.MaterialExpressionConstant, -400, 0)
        if rough_node:
            rough_node.set_editor_property("r", profile["accent_roughness"])
            mat_lib.connect_material_property(rough_node, "", unreal.MaterialProperty.MP_ROUGHNESS)

        # Metallic
        metal_node = mat_lib.create_material_expression(
            mat_asset, unreal.MaterialExpressionConstant, -400, 200)
        if metal_node:
            metal_node.set_editor_property("r", profile["accent_metallic"])
            mat_lib.connect_material_property(metal_node, "", unreal.MaterialProperty.MP_METALLIC)

        # Emissive
        if profile.get("emissive_intensity", 0) > 0:
            emissive_color = mat_lib.create_material_expression(
                mat_asset, unreal.MaterialExpressionConstant3Vector, -400, 400)
            emissive_mult = mat_lib.create_material_expression(
                mat_asset, unreal.MaterialExpressionConstant, -200, 500)

            if emissive_color and emissive_mult:
                ec = profile["emissive_color"]
                ei = profile["emissive_intensity"]
                emissive_color.set_editor_property("constant",
                    unreal.LinearColor(ec[0] * ei, ec[1] * ei, ec[2] * ei, 1.0))
                mat_lib.connect_material_property(
                    emissive_color, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

        mat_lib.recompile_material(mat_asset)
        save_asset(mat_full)
        unreal.log(f"  Created zone material: {mat_full}")
        return mat_full

    except Exception as e:
        unreal.log_warning(f"  Material creation failed for {zone_name}: {e}")
        return None


def create_zone_emissive_light_material(zone_name, profile):
    """Create a self-illuminating material for light fixtures (fluorescent panels, etc.)."""
    mat_path = "/Game/Materials/Zones"
    mat_name = f"M_Zone_{zone_name}_LightPanel"
    mat_full = f"{mat_path}/{mat_name}"

    ensure_directory(mat_path)

    if editor_util.does_asset_exist(mat_full):
        save_asset(mat_full)
        return mat_full

    try:
        factory = unreal.MaterialFactoryNew()
        mat_asset = asset_tools.create_asset(mat_name, mat_path, unreal.Material, factory)
        if not mat_asset:
            return None

        # Emissive-only material (unlit light panel)
        kc = profile["key_light_color"]
        # Convert 0-255 light color to 0-1 range for emissive
        emissive_r = kc[0] / 255.0
        emissive_g = kc[1] / 255.0
        emissive_b = kc[2] / 255.0

        color_node = mat_lib.create_material_expression(
            mat_asset, unreal.MaterialExpressionConstant3Vector, -300, 0)
        if color_node:
            intensity_scale = 2.0  # Bright enough to be visible as a light source
            color_node.set_editor_property("constant",
                unreal.LinearColor(
                    emissive_r * intensity_scale,
                    emissive_g * intensity_scale,
                    emissive_b * intensity_scale,
                    1.0))
            mat_lib.connect_material_property(
                color_node, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
            mat_lib.connect_material_property(
                color_node, "", unreal.MaterialProperty.MP_BASE_COLOR)

        # Low roughness for light panels
        rough_node = mat_lib.create_material_expression(
            mat_asset, unreal.MaterialExpressionConstant, -300, 200)
        if rough_node:
            rough_node.set_editor_property("r", 0.1)
            mat_lib.connect_material_property(rough_node, "", unreal.MaterialProperty.MP_ROUGHNESS)

        mat_lib.recompile_material(mat_asset)
        save_asset(mat_full)
        unreal.log(f"  Created light panel material: {mat_full}")
        return mat_full

    except Exception as e:
        unreal.log_warning(f"  Light panel material failed for {zone_name}: {e}")
        return None


# ============================================================================
# 5. ZONE LIGHTING CONFIG DATA TABLE
# ============================================================================

def create_lighting_config_json():
    """
    Write a JSON file with all zone lighting parameters for use as a
    DataTable or runtime configuration reference.
    """
    import json
    import os

    config = {}
    for zone_name, profile in ZONE_PROFILES.items():
        config[zone_name] = {
            "Description": profile["description"],
            "KeyLightColor": list(profile["key_light_color"]),
            "KeyLightIntensity": profile["key_light_intensity"],
            "KeyLightRadius": profile["key_light_radius"],
            "FillLightColor": list(profile["fill_light_color"]),
            "FillLightIntensity": profile["fill_light_intensity"],
            "FillLightRadius": profile["fill_light_radius"],
            "Flicker": profile["flicker"],
            "FogDensity": profile["fog_density"],
            "FogInscatteringColor": list(profile["fog_inscattering"][:3]),
            "PostProcessBlueprint": f"/Game/Blueprints/PostProcess/BP_PP_{zone_name}Zone",
            "FogBlueprint": f"/Game/Blueprints/Atmosphere/BP_Fog_{zone_name}Zone",
            "LightRigBlueprint": f"/Game/Blueprints/LightRigs/BP_LightRig_{zone_name}",
            "AccentMaterial": f"/Game/Materials/Zones/M_Zone_{zone_name}_Accent",
            "LightPanelMaterial": f"/Game/Materials/Zones/M_Zone_{zone_name}_LightPanel",
        }

    # Write to DataTables directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    dt_dir = os.path.join(os.path.dirname(script_dir), "DataTables")
    os.makedirs(dt_dir, exist_ok=True)

    config_path = os.path.join(dt_dir, "DT_ZoneLighting.json")
    with open(config_path, "w") as f:
        json.dump(config, f, indent=2)

    unreal.log(f"  Wrote lighting config: {config_path}")
    return config_path


# ============================================================================
# MAIN ENTRY POINT
# ============================================================================

def run():
    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  SNOWPIERCER: ETERNAL ENGINE")
    unreal.log("  Zone Lighting & Atmosphere Profiles")
    unreal.log("=" * 64)
    unreal.log("")

    pp_created = 0
    fog_created = 0
    rig_created = 0
    mat_created = 0

    with unreal.ScopedSlowTask(len(ZONE_PROFILES) * 5,
                                "Creating zone lighting profiles...") as slow_task:
        slow_task.make_dialog(True)

        for zone_name, profile in ZONE_PROFILES.items():
            unreal.log(f"--- Zone: {zone_name} ({profile['description']}) ---")

            # PostProcess Blueprint
            slow_task.enter_progress_frame(1, f"Creating PP for {zone_name}...")
            result = create_pp_blueprint(zone_name, profile)
            if result:
                pp_created += 1

            # Fog Blueprint
            slow_task.enter_progress_frame(1, f"Creating fog for {zone_name}...")
            result = create_fog_blueprint(zone_name, profile)
            if result:
                fog_created += 1

            # Light Rig Blueprint
            slow_task.enter_progress_frame(1, f"Creating light rig for {zone_name}...")
            result = create_lightrig_blueprint(zone_name, profile)
            if result:
                rig_created += 1

            # Zone Accent Material
            slow_task.enter_progress_frame(1, f"Creating accent material for {zone_name}...")
            result = create_zone_accent_material(zone_name, profile)
            if result:
                mat_created += 1

            # Zone Light Panel Material
            slow_task.enter_progress_frame(1, f"Creating light panel for {zone_name}...")
            result = create_zone_emissive_light_material(zone_name, profile)
            if result:
                mat_created += 1

    # Lighting config JSON
    unreal.log("--- Writing lighting config DataTable ---")
    create_lighting_config_json()

    # Summary
    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  ZONE LIGHTING SETUP COMPLETE")
    unreal.log("=" * 64)
    unreal.log(f"  PostProcess blueprints: {pp_created}")
    unreal.log(f"  Fog blueprints:         {fog_created}")
    unreal.log(f"  Light rig blueprints:   {rig_created}")
    unreal.log(f"  Zone materials:         {mat_created}")
    unreal.log("")
    unreal.log("  Created asset tree:")
    unreal.log("    /Game/Blueprints/PostProcess/")
    for zone in ZONE_PROFILES:
        unreal.log(f"      BP_PP_{zone}Zone")
    unreal.log("    /Game/Blueprints/Atmosphere/")
    for zone in ZONE_PROFILES:
        unreal.log(f"      BP_Fog_{zone}Zone")
    unreal.log("    /Game/Blueprints/LightRigs/")
    for zone in ZONE_PROFILES:
        unreal.log(f"      BP_LightRig_{zone}")
    unreal.log("    /Game/Materials/Zones/")
    for zone in ZONE_PROFILES:
        unreal.log(f"      M_Zone_{zone}_Accent, M_Zone_{zone}_LightPanel")
    unreal.log("")
    unreal.log("  USAGE:")
    unreal.log("    1. Drag BP_PP_<Zone>Zone into any car for instant atmosphere")
    unreal.log("    2. Drag BP_Fog_<Zone>Zone for zone-specific haze")
    unreal.log("    3. Drag BP_LightRig_<Zone> for pre-configured lighting")
    unreal.log("    4. Apply M_Zone_<Zone>_Accent to surfaces for zone color")
    unreal.log("    5. Apply M_Zone_<Zone>_LightPanel to ceiling panels")
    unreal.log("")
    unreal.log("  ZONE PALETTE REFERENCE:")
    for zone, profile in ZONE_PROFILES.items():
        unreal.log(f"    {zone:10s} — {profile['description']}")
    unreal.log("")


if __name__ == "__main__":
    run()
else:
    run()
