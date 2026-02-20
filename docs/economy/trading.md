# Barter Economy & Trading

No universal currency. All trade is barter — item values are zone-relative.

## Core Principle: Zone-Specific Value

The same item has different value depending on where you trade it.
Scarcity drives value — resources abundant in a zone are cheap there, expensive elsewhere.

### Value Multipliers by Zone

Base value = 1.0. Multipliers adjust per zone.

| Resource | Tail | Third Class | Spine | Second Class | First Class | Engine |
|----------|------|-------------|-------|-------------|-------------|--------|
| Metal Scrap | 0.5 | 0.7 | 1.0 | 1.5 | 2.0 | 0.8 |
| Cloth | 1.0 | 0.8 | 1.2 | 1.0 | 0.5 | 1.5 |
| Protein Blocks | 0.8 | 0.7 | 1.0 | 1.2 | 0.3 | 1.5 |
| Fresh Food | 3.0 | 2.0 | 1.5 | 1.0 | 0.5 | 2.5 |
| Medicine | 2.5 | 2.0 | 1.0 | 0.8 | 0.5 | 1.5 |
| Kronole | 1.5 | 1.5 | 2.0 | 1.0 | 2.5 | 3.0 |
| Electronics | 2.0 | 1.5 | 0.7 | 1.0 | 1.5 | 0.5 |
| Chemicals | 2.0 | 1.5 | 0.7 | 1.0 | 1.5 | 0.8 |
| Luxury Items | 0.3 | 0.5 | 1.0 | 1.5 | 0.5 | 3.0 |
| Ammo | 2.0 | 1.5 | 1.5 | 2.0 | 2.5 | 1.0 |
| Blueprints | 2.0 | 1.5 | 1.5 | 1.5 | 1.0 | 2.0 |
| Engine Parts | 3.0 | 2.5 | 1.5 | 2.0 | 2.5 | 0.5 |

### Trade Value Calculation

```
Effective Value = BaseValue × ZoneMultiplier × ConditionFactor × FactionModifier × HaggleResult
```

- **BaseValue**: Intrinsic item value (see resource table)
- **ZoneMultiplier**: From table above
- **ConditionFactor**: 1.0 at 100% quality, scales down with degradation
- **FactionModifier**: Based on faction standing with merchant
- **HaggleResult**: From haggling minigame (0.8 to 1.3)

## Merchant NPCs

### Merchant Types

| Type | Locations | Specialization | Inventory Refresh |
|------|-----------|----------------|-------------------|
| Scavenger | Tail, Third Class | Scrap, crude weapons, basic supplies | 2 days |
| Workshop Vendor | Third Class | Crafting materials, tools, T2 weapons | 3 days |
| Medic Trader | Spine infirmary | Medicine, chemicals, medkits | 4 days |
| Kronole Dealer | Hidden locations | Kronole, stimulants, information | 1 day |
| Black Marketeer | Varies (moves) | Stolen goods, rare items, contraband | Unique per visit |
| Arms Dealer | Near Armory | Ammo, T2-T3 weapons, armor | 5 days |
| First Class Purveyor | First Class | Luxury items, fine clothing, gourmet food | 7 days |
| Engine Specialist | Engine access | Engine parts, electronics, blueprints | 10 days |

### Merchant Behavior

- **Buy/Sell Spread**: Merchants buy at 60-80% of calculated value, sell at 100-130%.
- **Inventory Limits**: Merchants have finite stock and finite purchasing power.
- **Relationship Memory**: Repeat customers get better rates (up to -10% on purchases).
- **Faction Alignment**: Each merchant belongs to a faction. Faction standing affects prices.

### Faction Price Modifiers

| Standing | Buy Price | Sell Price |
|----------|-----------|------------|
| Hostile | Cannot trade | Cannot trade |
| Unfriendly | +30% | -20% |
| Neutral | Standard | Standard |
| Friendly | -10% | +10% |
| Allied | -20% | +15% |

## Haggling System

Trade initiates a simple haggling mechanic.

### How It Works

1. **Merchant offers initial price** (their standard rate)
2. **Player can Accept, Counter-offer, or Walk Away**
3. **Counter-offers**: Player proposes alternative item combination
4. **Merchant responds**: Accept, counter-counter, or refuse (getting annoyed)
5. **Resolution**: Deal struck or merchant refuses further negotiation

### Haggling Rules

- **3 rounds maximum** per transaction before merchant locks in final offer
- **Social stat** affects starting position (higher Social = better opening offers)
- **Faction standing** affects merchant patience (more rounds at higher standing)
- **Walking away**: 50% chance merchant calls you back with a better offer (once per visit)
- **Insulting offers** (below 50% of value): Merchant becomes hostile for that visit
- **Kronole bribe**: Offering Kronole to certain merchants bypasses haggling (+20% in your favor)

### Haggle Outcome Range

| Social Stat | Best Possible | Worst Possible |
|-------------|---------------|----------------|
| 1-2 | 1.0x (no discount) | 0.8x (overpay 20%) |
| 3-5 | 1.1x (10% in your favor) | 0.9x |
| 6-8 | 1.2x | 0.95x |
| 9-10 | 1.3x | 1.0x |

## Black Market

A roving merchant network that trades in stolen, contraband, and rare goods.

### Characteristics

- **Location**: Changes every 3 in-game days. Tip-offs from Kronole Network contacts.
- **Access**: Requires Kronole Network "Known" reputation or a tip from an NPC.
- **Inventory**: Unique items not found elsewhere — T3-T4 weapons, rare blueprints, stolen luxury goods.
- **Risk**: Jackboot patrols may raid the market. Getting caught = reputation hit + confiscation.
- **No haggling**: Prices are fixed but fair (no merchant markup).

### Stolen Goods

Items looted from NPCs or specific locations are flagged as "stolen."

- **Stolen goods** cannot be sold to legitimate merchants.
- **Black Marketeer** buys stolen goods at 70% of zone value.
- **Kronole Dealers** buy stolen goods at 50% but ask no questions.
- **Fence quest**: A mid-game quest unlocks a permanent fence NPC in Third Class.
- **Stolen flag clears** after holding an item for 7 in-game days (people forget).

## Special Trade Mechanics

### Information Trading

Some merchants trade in information rather than goods.

- **Rumor purchase**: Buy NPC locations, quest hints, guard patrol schedules.
- **Cost**: Varies by value. Guard schedules cost more than gossip.
- **Currency**: Information can be bartered for with Kronole, Luxury Items, or other information.

### Favor Economy

Completing tasks for merchants unlocks special inventory tiers.

- **Tier 1** (default): Standard inventory
- **Tier 2** (after 2 completed tasks): Rare items available, -5% prices
- **Tier 3** (after 5 completed tasks): Exclusive items, -10% prices, will buy stolen goods

### Bulk Trading

Trading 10+ of the same resource grants a 10% bonus value. Merchants value reliable suppliers.

### Cross-Zone Arbitrage

The player can profit by buying cheap in one zone and selling high in another.
This is intentional gameplay — the merchant caravan fantasy on a train.

Example profitable routes:
- Buy Metal Scrap cheap in Tail → Sell high in First Class
- Buy Fresh Food cheap in First Class → Sell high in Tail
- Buy Electronics cheap in Engine → Sell high in Tail
- Buy Luxury Items cheap in First Class → Sell high in Engine
