# Resource Types

All tradeable/craftable resource types in Snowpiercer: Eternal Engine.

## Resource Registry

| ID | Resource | Category | Primary Locations | Rarity | Stack Limit | Weight (kg) | Degrades? |
|----|----------|----------|-------------------|--------|-------------|-------------|-----------|
| `metal_scrap` | Metal Scrap | Material | Tail, Third Class workshops, wreckage | Common | 50 | 0.5 | No |
| `cloth` | Cloth | Material | Third Class, Tailor's cars, looted clothing | Common | 30 | 0.2 | Yes (moisture) |
| `protein_blocks` | Protein Blocks | Consumable | Tail (rations), Third Class kitchens | Common | 20 | 0.3 | Yes (spoilage) |
| `fresh_food` | Fresh Food | Consumable | Agriculture cars, First Class dining | Uncommon | 10 | 0.5 | Yes (fast spoil) |
| `medicine` | Medicine | Consumable | Infirmary cars, Bureaucracy medical | Rare | 10 | 0.1 | Yes (expiry) |
| `kronole` | Kronole | Consumable | Kronole Network dealers, hidden labs | Rare | 5 | 0.05 | No |
| `electronics` | Electronics | Material | Spine infrastructure, Engine support | Uncommon | 15 | 0.3 | No |
| `chemicals` | Chemicals | Material | Chemistry cars, Spine maintenance | Uncommon | 10 | 0.4 | Yes (volatility) |
| `luxury_items` | Luxury Items | Trade Good | First Class, black market | Rare | 5 | 0.5 | No |
| `ammo` | Ammunition | Combat | Armory, Jackboot barracks, hidden caches | Rare | 30 | 0.1 | No |
| `blueprints` | Blueprints | Knowledge | Loot, quest rewards, merchants | Very Rare | 1 | 0.05 | No |
| `engine_parts` | Engine Parts | Material | Engine car, Spine workshops, salvage | Very Rare | 5 | 2.0 | No |

## Location Distribution by Train Zone

### Tail Section
- **Abundant**: Metal Scrap, Protein Blocks
- **Scarce**: Cloth (poor quality)
- **Rare**: Improvised chemicals, stolen goods
- **Absent**: Fresh Food, Luxury Items, Engine Parts

### Third Class
- **Abundant**: Metal Scrap, Cloth, Protein Blocks
- **Moderate**: Chemicals, Electronics (workshops)
- **Scarce**: Medicine, Ammo
- **Absent**: Luxury Items

### Spine / Infrastructure
- **Abundant**: Electronics, Chemicals
- **Moderate**: Metal Scrap, Engine Parts (maintenance)
- **Scarce**: All consumables (staff only)

### Second Class
- **Moderate**: Cloth, Medicine, Electronics
- **Scarce**: Most materials (not a production zone)
- **Present**: Kronole (discreet dealers)

### First Class
- **Abundant**: Fresh Food, Luxury Items, Medicine
- **Moderate**: Cloth (fine fabrics)
- **Scarce**: Metal Scrap, Ammo (hidden stashes)
- **Present**: Kronole (high-end dealers)

### Engine Section
- **Abundant**: Engine Parts, Electronics
- **Moderate**: Metal Scrap, Chemicals
- **Restricted**: Everything else (limited access)

## Degradation Rules

Resources that degrade lose quality over time unless stored properly.

| Resource | Degradation Rate | Full Decay Time | Prevention |
|----------|-----------------|-----------------|------------|
| Cloth | 1% per in-game day | 100 days | Waterproof container |
| Protein Blocks | 2% per day | 50 days | Cold storage (automatic in Tail) |
| Fresh Food | 10% per day | 10 days | Refrigeration car only |
| Medicine | 1% per day | 100 days | Medical cabinet |
| Chemicals | 3% per day | ~33 days | Sealed container |

Degraded resources:
- **75-100%**: Full effectiveness
- **50-74%**: Reduced effectiveness (75% potency for consumables, -1 tier for crafting)
- **25-49%**: Significantly reduced (50% potency, crafting produces lower-quality output)
- **1-24%**: Barely usable (25% potency, high crafting failure chance)
- **0%**: Destroyed / unusable

## Scavenging & Respawn

Resources respawn in containers and environmental sources on a per-zone timer:
- **Tail**: 2 in-game days (scrounging lifestyle)
- **Third Class**: 3 days (regular resupply)
- **Spine**: 5 days (controlled access)
- **Second/First Class**: 7 days (abundant but guarded)
- **Engine**: 10 days (critical parts, slow manufacture)

Quest-specific resource nodes do not respawn.
