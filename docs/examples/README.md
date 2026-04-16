# RiftbornAI Quick Start Examples

This folder contains example prompts to get you started with RiftbornAI.

They are intentionally illustrative, not guarantees that every request resolves in one call. Exact results depend on the current tool surface, project health, available assets, and whether the task needs follow-up verification or iteration.

## 1. Character Creation

```
Create a third-person character with:
- 100 health
- Sprint ability (1.5x speed)
- Jump ability
```

## 2. Enemy AI

```
Create an enemy NPC that:
- Patrols between waypoints
- Detects player within 1000 units
- Attacks when player is in range
- Has 50 health
```

## 3. Weapon System

```
Create a projectile weapon system:
- Fires bullets at 10 rounds per second
- Each bullet does 25 damage
- Has magazine of 30 rounds
- 2 second reload time
```

## 4. Ability System (GAS)

```
Create a fireball ability:
- Costs 20 mana
- 5 second cooldown
- Deals 50 fire damage
- Burns for 10 damage over 3 seconds
```

## 5. UI Elements

```
Create a health bar widget:
- Top-left corner
- Shows current/max health
- Red color that fades when damaged
- Animates smoothly when health changes
```

## 6. Level Design

```
Spawn 8 enemies in a circle:
- Radius 500 units
- Around player start
- Random enemy types
```

## 7. Procedural Content

```
Generate a dungeon layout:
- 10x10 grid
- 5 rooms connected by corridors
- Place treasure in dead ends
- Place enemies in main rooms
```

## Tips

1. **Be specific** - The more detail you provide, the better the result
2. **Use numbers** - Specify exact values for damage, health, cooldowns
3. **Describe behavior** - Explain what should happen, not just what exists
4. **Iterate** - Start simple, then ask for modifications

## Troubleshooting

If commands are not working:

1. Check the Output Log for errors
2. Verify the bridge health route responds
3. Make sure your MCP client is connected to the plugin
4. Try a simpler read-first request before a complex multi-step generation request
