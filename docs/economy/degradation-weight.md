# Resource Degradation, Carry Weight & Stacking

## Resource Degradation

### Overview

Perishable resources degrade over time. Condition is tracked as a float (0.0-1.0).
Degradation rates are per in-game day. Environmental factors can accelerate or prevent decay.

### Degradation Categories

| Category | Resources | Base Rate/Day | Preventable? |
|----------|-----------|---------------|--------------|
| Perishable (Fast) | Fresh Food | 10% | Refrigeration car (full stop) |
| Perishable (Slow) | Protein Blocks | 2% | Cold storage (halved) |
| Moisture-Sensitive | Cloth | 1% | Waterproof container (full stop) |
| Volatile | Chemicals | 3% | Sealed container (full stop) |
| Expiring | Medicine | 1% | Medical cabinet (full stop) |
| Durable | Metal Scrap, Electronics, Ammo, Kronole, Luxury Items, Blueprints, Engine Parts | 0% | N/A |

### Condition Effects

| Condition Range | Label | Effect on Use | Trade Value Modifier |
|-----------------|-------|---------------|---------------------|
| 100-76% | Fresh | Full effectiveness | 1.0x |
| 75-51% | Worn | 75% effectiveness | 0.75x |
| 50-26% | Degraded | 50% effectiveness | 0.4x |
| 25-1% | Ruined | 25% effectiveness | 0.1x |
| 0% | Destroyed | Cannot be used, auto-removed | 0x |

### Storage Mechanics

Special storage locations halt or slow degradation:

| Storage Type | Location | Capacity | Effect |
|-------------|----------|----------|--------|
| Cold Storage Locker | Tail (ambient cold) | 20 items | Halves food degradation |
| Refrigeration Car | Spine/Second Class | 50 items | Stops all food degradation |
| Medical Cabinet | Infirmary cars | 15 items | Stops medicine degradation |
| Waterproof Chest | Craftable (T2) | 10 items | Stops moisture degradation |
| Sealed Crate | Craftable (T2) | 10 items | Stops chemical volatility |
| Personal Stash | Player hideout (unlocked mid-game) | 100 items | No degradation prevention |

### Weapon & Armor Durability

Weapons and armor use a separate durability system (see weapons-armor.md).

| Action | Durability Cost |
|--------|----------------|
| Landing a melee hit | 2-5 (varies by tier) |
| Being blocked by enemy | 5 (extra stress on weapon) |
| Blocking with shield | 3-5 |
| Taking a hit (armor) | 2-4 (varies by hit type) |
| Environmental damage | 1-3 (rain, cold, acid) |

Durability at 0% = item breaks. Broken weapons deal 25% damage and cannot be repaired.
Broken armor provides 0% DR. Must craft a new one.

## Carry Weight

### Weight Budget

| Factor | Weight |
|--------|--------|
| Base carry capacity | 35.0 kg |
| Per Strength stat point | +3.0 kg |
| Engineer's Exo-Frame (T4 armor) | +15.0 kg |
| Companion carry bonus | +10.0 kg (per companion, up to 2) |
| Overweight penalty threshold | 100% of max |
| Immobilized threshold | 150% of max |

### Overweight Effects

| Weight % | Effect |
|----------|--------|
| 0-75% | No penalty |
| 76-100% | Mild: -10% movement speed |
| 101-125% | Moderate: -30% movement speed, -15% dodge speed, +stamina drain |
| 126-150% | Severe: -50% movement, cannot dodge, double stamina drain |
| 150%+ | Immobilized: Cannot move until items are dropped |

### Weight by Category

| Category | Typical Weight Range |
|----------|---------------------|
| Materials (scrap, cloth, etc.) | 0.05-2.0 kg per unit |
| Consumables | 0.1-0.5 kg per unit |
| Weapons (melee) | 1.0-4.0 kg |
| Weapons (ranged) | 2.0-5.0 kg |
| Armor (torso) | 3.0-10.0 kg |
| Armor (head) | 1.0-3.0 kg |
| Shields | 4.0-8.0 kg |
| Tools | 0.5-2.0 kg |
| Quest items | 0.0 kg (weightless) |

## Stacking Limits

Items stack by type. Stack limits prevent hoarding and encourage trade.

| Category | Max Stack Size | Notes |
|----------|---------------|-------|
| Metal Scrap | 50 | High stack — bulk material |
| Cloth | 30 | Medium stack |
| Protein Blocks | 20 | Moderate — takes up space |
| Fresh Food | 10 | Low — bulky and fragile |
| Medicine | 10 | Small containers |
| Kronole | 5 | Highly controlled |
| Electronics | 15 | Moderate — components |
| Chemicals | 10 | Dangerous in quantity |
| Luxury Items | 5 | Bulky valuables |
| Ammo | 30 | Compact rounds |
| Blueprints | 1 | Each is unique |
| Engine Parts | 5 | Heavy, large |
| Crafted consumables | 5-10 | Varies by type |
| Weapons | 1 | Each is unique instance |
| Armor | 1 | Each is unique instance |

### Inventory Slot System

- **Grid-based**: 6 columns x 8 rows = 48 slots
- **Small items**: 1 slot (ammo, kronole, medicine)
- **Medium items**: 2 slots (weapons, tools, food stacks)
- **Large items**: 4 slots (rifles, shields, engine parts)
- **Equipped items** do not occupy grid slots (separate equipment slots)

### Equipment Slots

| Slot | Item Types |
|------|-----------|
| Primary Weapon | Any weapon |
| Secondary Weapon | Any weapon (quick swap) |
| Head Armor | Helmets |
| Torso Armor / Disguise | Body armor or disguise outfit |
| Shield | Shields (optional) |
| Tool | Active tool (lockpick, torch, etc.) |
| Quick Use x3 | Consumables (hotbar) |
