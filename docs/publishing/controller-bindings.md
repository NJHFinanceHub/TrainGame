# Controller Support — Full Gamepad Bindings

## Overview

Full controller support for Xbox, PlayStation, and Steam Deck controllers. All gameplay is accessible without keyboard/mouse. Steam Input API provides platform-agnostic binding with per-controller defaults and full remapping.

---

## Action Set: Exploration (Default)

| Action | Xbox | PlayStation | Steam Deck | Keyboard |
|--------|------|-------------|------------|----------|
| Move | Left Stick | Left Stick | Left Stick | WASD |
| Camera | Right Stick | Right Stick | Right Stick + Gyro | Mouse |
| Sprint | Left Stick Click (L3) | L3 | L3 | Left Shift |
| Crouch / Stealth | Right Stick Click (R3) | R3 | R3 | Left Ctrl |
| Jump | A | Cross | A | Space |
| Interact | X | Square | X | E |
| Toggle View (1P/3P) | D-Pad Up | D-Pad Up | D-Pad Up | V |
| Journal | D-Pad Left | D-Pad Left | D-Pad Left | J |
| Inventory | D-Pad Right | D-Pad Right | D-Pad Right | Tab |
| Map / Train Diagram | D-Pad Down | D-Pad Down | D-Pad Down | M |
| Quick Slot 1 | LB + A | L1 + Cross | L4 | 1 |
| Quick Slot 2 | LB + B | L1 + Circle | L4 + A | 2 |
| Quick Slot 3 | LB + X | L1 + Square | L4 + X | 3 |
| Quick Slot 4 | LB + Y | L1 + Triangle | L4 + Y | 4 |
| Pause Menu | Menu / Start | Options | Menu | Escape |

---

## Action Set: Combat

Activated automatically when combat is initiated. Returns to Exploration when combat ends.

| Action | Xbox | PlayStation | Steam Deck | Keyboard |
|--------|------|-------------|------------|----------|
| Move | Left Stick | Left Stick | Left Stick | WASD |
| Camera / Target | Right Stick | Right Stick | Right Stick + Gyro | Mouse |
| Light Attack | RB | R1 | R1 | Left Mouse |
| Heavy Attack (hold) | RT | R2 | R2 | Middle Mouse |
| Block / Parry | LT | L2 | L2 | Right Mouse |
| Dodge | B | Circle | B | Left Alt |
| Quick Slot 1 | D-Pad Up | D-Pad Up | D-Pad Up | 1 |
| Quick Slot 2 | D-Pad Right | D-Pad Right | D-Pad Right | 2 |
| Quick Slot 3 | D-Pad Down | D-Pad Down | D-Pad Down | 3 |
| Quick Slot 4 | D-Pad Left | D-Pad Left | D-Pad Left | 4 |
| Lock-On Target | R3 | R3 | R3 | Q |
| Switch Target | Right Stick Flick | Right Stick Flick | Right Stick Flick | Mouse Wheel |
| Companion Command | Y | Triangle | Y | F |
| Sprint (disengage) | L3 | L3 | L3 | Left Shift |

### Combat Notes
- **Heavy Attack**: Hold RT/R2 to charge, release to strike. Charge level indicated by controller vibration intensity ramp.
- **Block**: Hold for sustained block. Precise timing triggers parry (within 200ms window).
- **Dodge**: i-frames during dodge roll. Direction controlled by Left Stick.
- **Lock-On**: Toggles target lock. Right Stick flick switches between visible targets.

---

## Action Set: Stealth

Activated when crouching/entering stealth mode. Extends Exploration with stealth-specific actions.

| Action | Xbox | PlayStation | Steam Deck | Keyboard |
|--------|------|-------------|------------|----------|
| Move (slow) | Left Stick | Left Stick | Left Stick | WASD |
| Camera / Peek | Right Stick | Right Stick | Right Stick + Gyro | Mouse |
| Non-Lethal Takedown | X (behind enemy) | Square (behind enemy) | X (behind enemy) | E (behind enemy) |
| Whistle / Lure | Y | Triangle | Y | C |
| Throw Distraction | RB | R1 | R1 | G |
| Peek Around Corner | LT + Right Stick | L2 + Right Stick | L2 + Right Stick | Right Mouse + Mouse |
| Enter Hiding Spot | A (near spot) | Cross (near spot) | A (near spot) | Space (near spot) |
| Enter Vent | X (near vent) | Square (near vent) | X (near vent) | E (near vent) |
| Toggle Disguise | D-Pad Up | D-Pad Up | D-Pad Up | Z |
| Stand Up (exit stealth) | R3 | R3 | R3 | Left Ctrl |

---

## Action Set: Dialogue

Activated when entering a dialogue sequence. Full controller navigation of dialogue UI.

| Action | Xbox | PlayStation | Steam Deck | Keyboard |
|--------|------|-------------|------------|----------|
| Navigate Choices | Left Stick / D-Pad | Left Stick / D-Pad | Left Stick / D-Pad | W/S / Arrow Keys |
| Select Choice | A | Cross | A | E / Enter |
| Advance Text | A | Cross | A | Space / Enter |
| Skip (held) | Y (hold 1s) | Triangle (hold 1s) | Y (hold 1s) | Escape (hold 1s) |
| View Character Info | X | Square | X | Tab |
| Back / Cancel | B | Circle | B | Escape |

### Dialogue Notes
- Skill check options show the required stat and current value inline
- Failed checks are grayed out but visible (so player sees what they missed)
- Hold-to-skip prevents accidental dialogue skipping

---

## Action Set: Menu

Active in all menu screens (pause, inventory, crafting, journal, settings).

| Action | Xbox | PlayStation | Steam Deck | Keyboard |
|--------|------|-------------|------------|----------|
| Navigate | Left Stick / D-Pad | Left Stick / D-Pad | Left Stick / D-Pad | WASD / Arrows |
| Confirm | A | Cross | A | Enter |
| Back | B | Circle | B | Escape |
| Tab Left | LB | L1 | L1 | Q |
| Tab Right | RB | R1 | R1 | E |
| Page Up | LT | L2 | L2 | Page Up |
| Page Down | RT | R2 | R2 | Page Down |
| Context Action 1 | X | Square | X | R |
| Context Action 2 | Y | Triangle | Y | F |
| Quick Save | Menu + LB | Options + L1 | Menu + L1 | F5 |
| Quick Load | Menu + RB | Options + R1 | Menu + R1 | F9 |

---

## Haptic Feedback (Controller Vibration)

### Xbox / Standard Rumble

| Event | Left Motor (Low Freq) | Right Motor (High Freq) | Duration |
|-------|----------------------|------------------------|----------|
| Light attack hit | 0.0 | 0.6 | 100ms |
| Heavy attack charge | 0.0 → 0.8 (ramp) | 0.0 | Hold duration |
| Heavy attack hit | 0.8 | 0.8 | 200ms |
| Block impact | 0.4 | 0.4 | 150ms |
| Parry (perfect block) | 0.2 | 1.0 | 80ms |
| Take damage | 0.6 | 0.3 | 250ms |
| Cold exposure tick | 0.1 | 0.0 | 500ms (pulsing) |
| Train rumble (ambient) | 0.05 | 0.0 | Continuous |
| Exterior wind | 0.0 | 0.15 | Continuous |
| Tunnel passage | 0.3 | 0.3 | Duration of tunnel |
| Achievement unlock | 0.0 | 0.5 | 300ms |
| Companion downed | 0.8 | 0.0 | 400ms |

### DualSense Adaptive Triggers (PlayStation 5)

| Context | L2 (Block) | R2 (Heavy Attack) |
|---------|------------|-------------------|
| Combat idle | Soft resistance (weapon weight) | Soft resistance |
| Blocking | Rigid resistance (force feedback) | N/A |
| Heavy charge | N/A | Increasing resistance with charge level |
| Heavy release | N/A | Snap release |
| Cold exposure | Vibrating resistance (shivering) | Vibrating resistance |
| Crawlspace | Light pulse (confined space) | Light pulse |

### DualSense Haptics

- **Train wheel rhythm**: Subtle haptic pattern matching the 72 BPM wheel-on-rail cadence
- **Zone transitions**: Distinct haptic signature per zone (rougher in Tail, smoother in First Class)
- **Stealth detection**: Increasing haptic intensity as detection meter fills

---

## Gyro Aim (Steam Deck / DualSense / Switch Pro)

| Setting | Default | Range |
|---------|---------|-------|
| Gyro Enabled | On (Steam Deck), Off (others) | On/Off |
| Gyro Sensitivity | 1.0 | 0.1 - 3.0 |
| Gyro Activation | Always On | Always On / Right Stick Touch / L2 Hold |
| Gyro Axis | Yaw + Pitch | Yaw Only / Pitch Only / Both |
| Gyro Invert Y | Off | On/Off |

### Gyro Use Cases
- **Camera control**: Fine-tune aiming and looking while right stick provides coarse movement
- **Stealth peeking**: Precise peek control around corners
- **Dialogue**: Not active during dialogue (unnecessary)

---

## Button Prompt Glyphs

The game dynamically displays controller-appropriate button prompts based on the active input device.

| Input Device | Glyph Set | Detection Method |
|-------------|-----------|-----------------|
| Keyboard/Mouse | Key names ("E", "LMB") | Last input was KB/M |
| Xbox Controller | Xbox glyphs (A/B/X/Y) | XInput device detected |
| PlayStation Controller | PS glyphs (Cross/Circle/Square/Triangle) | DualShock/DualSense detected via Steam Input |
| Steam Deck | Steam Deck glyphs | Running on SteamOS |
| Generic Controller | Xbox-style glyphs (fallback) | Unknown controller |

### Glyph Switching
- Glyphs switch instantly when input device changes (e.g., player touches mouse → KB prompts, touches controller → gamepad prompts)
- No mixed prompts — all on-screen prompts use the same glyph set
- Tutorial screens show the active device's bindings

---

## Accessibility: Controller Remapping

| Feature | Supported |
|---------|-----------|
| Full button remapping | Yes (via Steam Input + in-game) |
| Stick dead zone adjustment | Yes (0.05 - 0.50) |
| Stick response curve | Yes (Linear / Exponential / Custom) |
| Trigger dead zone | Yes (0.0 - 0.30) |
| Swap sticks | Yes |
| Invert Y axis | Yes (camera and gyro independently) |
| One-handed mode | Yes (remaps all essential actions to one side) |
| Hold vs. toggle | Configurable for Sprint, Crouch, Block, Aim |
| Vibration intensity | 0% - 100% slider |
| Adaptive trigger intensity | 0% - 100% slider (DualSense) |

---

## DefaultInput.ini — Gamepad Bindings (Legacy Format)

For immediate implementation before Enhanced Input migration:

```ini
; === GAMEPAD: EXPLORATION ===
+ActionMappings=(ActionName="Interact",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Gamepad_FaceButton_Left)
+ActionMappings=(ActionName="Jump",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Gamepad_FaceButton_Bottom)
+ActionMappings=(ActionName="ToggleView",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Gamepad_DPad_Up)
+ActionMappings=(ActionName="Journal",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Gamepad_DPad_Left)
+ActionMappings=(ActionName="Sprint",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Gamepad_LeftThumbstick)
+ActionMappings=(ActionName="Crouch",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Gamepad_RightThumbstick)
+ActionMappings=(ActionName="Pause",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Gamepad_Special_Right)

; === GAMEPAD: COMBAT ===
+ActionMappings=(ActionName="LightAttack",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Gamepad_RightShoulder)
+ActionMappings=(ActionName="Dodge",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Gamepad_FaceButton_Right)

; === GAMEPAD: MOVEMENT AXES ===
+AxisMappings=(AxisName="MoveForward",Scale=1.0,Key=Gamepad_LeftY)
+AxisMappings=(AxisName="MoveRight",Scale=1.0,Key=Gamepad_LeftX)
+AxisMappings=(AxisName="Turn",Scale=1.0,Key=Gamepad_RightX)
+AxisMappings=(AxisName="LookUp",Scale=1.0,Key=Gamepad_RightY)

; === GAMEPAD: TRIGGERS (Axis as Action) ===
+AxisMappings=(AxisName="HeavyAttack",Scale=1.0,Key=Gamepad_RightTriggerAxis)
+AxisMappings=(AxisName="Block",Scale=1.0,Key=Gamepad_LeftTriggerAxis)
```

---

## Steam Input Configuration File

The default controller configuration ships as a VDF file registered in the Steamworks partner portal.

### Official Configurations

| Config Name | Target | Description |
|------------|--------|-------------|
| `SnowpiercerEE_Xbox` | Xbox controllers | Default Xbox layout |
| `SnowpiercerEE_PS` | DualShock 4 / DualSense | PlayStation layout with adaptive trigger support |
| `SnowpiercerEE_Deck` | Steam Deck | Deck-optimized with gyro aim, back buttons for quick slots |
| `SnowpiercerEE_Generic` | Generic controllers | Xbox-style fallback |

### Community Configurations

- Community controller configurations enabled in Steamworks settings
- Players can share and vote on configurations via Steam
- Top-rated community configs highlighted in controller settings menu
