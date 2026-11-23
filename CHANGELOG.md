# Changelog

All notable changes to Osiris Better are documented here.

## [Unreleased]

### Added
- **Grenade Timer** — visual countdown for active molotov/incendiary grenades (`C_Inferno` entity tracking)
- **Health Bar** — colored health bar in player ESP panels
- **Player Name** — name label in player ESP panels
- **Player Distance** — distance readout in player ESP panels
- **Radar Hack** — configurable radar reveal for all players
- **Visuals Tab** — new Panorama UI tab organizing all visual features
- **Model Glow Color Modes** — per-team and health-based color mode options for player model glow
- **Outline Glow Color Modes** — per-team and health-based color mode options for player outline glow

### Changed
- Bomb Timer panel reworked with improved layout and site indicator
- Bomb Timer panel factory updated for new panel structure
- Player info in-world panel factory expanded to support new sub-panels (ESP box, health bar, name, distance)
- Config schema extended with new variable types for color modes and visual features
- `CreateGUI.js` refactored with updated Panorama UI layout and new feature controls
- `RenderingHookEntityLoop` updated to process `C_Inferno` entities for grenade timer

### Removed
- CI workflow files (`.github/workflows/`) — not needed for this fork
- `.clang-tidy` configuration
- `.gitattributes`

## [Base] — Osiris upstream

Based on [danielkrupinski/Osiris](https://github.com/danielkrupinski/Osiris) at commit `12aa94bb` (Fix for the latest game update on Linux).

### Existing Features (from upstream)
- Sound visualization (footsteps, bomb beep, plant, defuse, reload, scope)
- Model glow (player, weapon, grenade, bomb, defuse kit)
- Outline glow (player, weapon, grenade, bomb, hostage, defuse kit)
- Player info in-world (health, weapon icon, ammo, position arrow, bomb icon)
- HUD features (bomb timer, defusing alert, bomb plant alert, post round timer, killfeed preserver)
- Combat features (no-scope inaccuracy visualization)
- In-game Panorama UI configuration
- Auto-save configuration system
- Cross-platform support (Windows / Linux)
