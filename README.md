# Prog4Project
Howest DAE programming 4 Minigin Engine Game Project


### Engine Systems

| System | Description |
|--------|-------------|
| **GameObject / Component** | Entity-component model. A `GameObject` owns a list of `Component`s; each component is independently updatable and renderable. |
| **SceneManager** | Manages named scenes; only one scene is active at a time. Scenes own their GameObjects. |
| **InputManager** | Abstraction over keyboard and up to 4 Xbox controllers via XInput. Binds actions to `Command` objects per input type. |
| **EventQueue** | Decoupled publish/subscribe system. Events are enqueued each frame and dispatched at the end of the update loop, preventing re-entrant modification. |
| **ServiceLocator** | Provides a `SoundSystem` interface. At startup the concrete `SDLSoundSystem` is registered; a `NullSoundSystem` is used as fallback. |
| **ResourceManager** | Loads and caches fonts and textures. |
| **EngineTime** | Central delta-time source; updated once per frame. |

---

## Game — BurgerTime

A faithful recreation of the 1982 arcade game **BurgerTime** supporting three game modes:

- **Single Player** — classic one-player experience
- **Co-op** — two players share lives working together
- **Versus** — one player controls Peter Pepper, the second controls a sausage enemy

---

## Design Choices

### 1. State Machine (Player, Enemy, PlayerDog)

All actors are driven by a **polymorphic state machine** using `std::unique_ptr<State>`. Each state returns a new state pointer to trigger a transition, or `nullptr` to stay. `OnEnter` / `OnExit` handle setup and teardown.

- **Player**: `Idle → Walking → Climbing → Dead → Victory`
- **Enemy (NormalState)**: four internal phases — `Spawn → WalkToLadder → Climbing → Chase` — so the enemy first pathfinds to a ladder, climbs it, then chases the player on the destination platform
- **PlayerDog**: `Normal → Stunned (0.5s) → Crushed → Frozen`

This avoids flag-soup and keeps per-state logic isolated.

### 2. Command Pattern (Input)

All input actions are encapsulated as `Command` objects (`MoveCommand`, `FuncCommand`, `DamageCommand`, …). `InputManager` maps controller buttons and keyboard keys to commands; the game never polls hardware directly. Adding a new action is one `BindControllerCommand` / `BindKeyboardCommand` call.

### 3. Observer / Event Queue

Score updates, enemy kills, and burger completion broadcast `Event` objects through `dae::EventQueue`. Display components listen for these events and update themselves without needing a direct reference to game logic components. This keeps `ScoreComponent` and `LivesDisplayComponent` fully decoupled from gameplay code.

### 4. Service Locator (Sound)

`dae::ServiceLocator` provides the active `SoundSystem`. `SDLSoundSystem` runs an async worker thread that processes a `std::queue<SoundEvent>` so audio loading and playback never stall the main thread. Muting and volume control are thread-safe via `std::atomic`.

### 5. Section-Based BFS Pathfinding (Enemy AI)

The level is divided into **platform sections** connected by **ladders** (a graph). Enemy pathfinding runs BFS over this graph to find the shortest ladder sequence from the enemy's section to the player's section. The path is rebuilt whenever the player changes section, and is resolved correctly even when the player is mid-ladder (using `GetNearestSectionExcluding` to find the other end of the ladder the player is on).

### 6. Burger Crush Chain

When a burger piece walks off a platform it enters a falling state. Enemies standing below are crushed and scored. Multiple pieces stacked in a single drop multiply the score. Burger completion (all pieces on the tray) triggers the level-complete sequence.
