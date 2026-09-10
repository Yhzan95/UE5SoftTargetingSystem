# Soft Targeting for Unreal Engine 5

![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.8-0E1128?logo=unrealengine&logoColor=white)
![C++ and Blueprint](https://img.shields.io/badge/C%2B%2B%20%2B%20Blueprint-Ready-00599C?logo=cplusplus&logoColor=white)
![Version](https://img.shields.io/badge/version-1.1.0-2ea44f)
![License](https://img.shields.io/badge/license-MIT-blue)

**Directional melee target selection for Unreal Engine 5.8.**

Soft Targeting automatically selects the enemy the player most likely intends to attack. It combines movement input, character facing, distance, camera framing, target priority, visibility, and target persistence without forcing a permanent camera lock.

[Download the latest release](https://github.com/Yhzan95/UE5SoftTargetingSystem/releases/latest) · [Open the Wiki](https://github.com/Yhzan95/UE5SoftTargetingSystem/wiki) · [Report an issue](https://github.com/Yhzan95/UE5SoftTargetingSystem/issues) · [View the changelog](CHANGELOG.md)

## Why Soft Targeting?

In melee combat with several nearby enemies, selecting only the closest actor is rarely enough. The closest enemy may be behind the player while the movement input, character orientation, and camera are clearly pointing toward another target.

Soft Targeting evaluates all valid candidates and returns the most intentional choice for the next attack. It only provides the selected target and useful targeting data; your combat system remains in control of movement, rotation, attacks, damage, animation, hit detection, and camera behavior.

## Key Features

| Feature | Description |
|---|---|
| Directional scoring | Combines character facing, movement input, camera direction, distance, and designer-authored target priority. |
| Per-attack queries | Give every punch, kick, sweep, grab, or dash its own range, selection cone, and range tolerance. |
| Stable target persistence | Prevents small score differences from constantly switching the target. Deliberate input can override persistence. |
| Line of sight | Rejects enemies hidden by level geometry, with a configurable grace period to prevent wall-edge flicker. |
| Target locking | Keeps the selected enemy stable during an attack without locking or recentering the camera. |
| Forced targets | Pins a target for grabs, throws, finishers, and scripted sequences. |
| Flexible filtering | Supports collision types, actor classes, Actor Tags, Gameplay Tags, ignored actors, and an optional targetable interface. |
| Blueprint events | Reports target, candidate, visibility, and attack-range transitions. |
| Motion Warping helper | Calculates a target-facing transform and safe stopping position. |
| Layered debug visualization | Independently toggle radii, attack cone, directions, target lines, target points, score labels, and state text. |
| Optimized evaluation | Uses a candidate cap, line-trace budget, cached target locations, and configurable refresh interval. |

## Soft Targeting vs. Traditional Lock-On

| Soft Targeting | Traditional Lock-On |
|---|---|
| Chooses a target for an attack | Keeps one target selected continuously |
| Does not rotate or recenter the camera | Often controls or recenters the camera |
| Supports fast directional changes | Usually requires explicit target switching |
| Works naturally in enemy crowds | Focuses combat on one opponent |
| Can remain completely invisible to the player | Commonly uses an on-screen lock-on marker |

Both systems can coexist. Soft Targeting can handle normal melee attacks while a separate hard lock-on system handles focused encounters, bosses, or ranged combat.

## How Target Selection Works

```text
Sphere overlap
    -> Target validation and filters
    -> Attack range and cone
    -> Directional scoring
    -> Candidate sorting
    -> Line-of-sight checks
    -> Target persistence
    -> Best target
```

The default score weights are:

| Criterion | Weight |
|---|---:|
| Movement input | 0.40 |
| Character facing | 0.35 |
| Distance | 0.20 |
| Camera framing | 0.05 |
| Target priority | 0.00 |

Only enabled criteria participate in normalization. If the movement stick is inside the dead zone, input direction is removed from the scoring formula instead of reducing every candidate's score.

## Requirements

- Unreal Engine 5.8
- C++ or Blueprint project
- Enhanced Input only if you use the optional `SoftTargetingExamples` module
- Designed for single-player gameplay

Networking, replication, and authority handling are intentionally outside the current plugin scope.

## Installation

1. Download or clone this repository.
2. Copy the plugin folder into:

   ```text
   YourProject/Plugins/SoftTargeting/
   ```

3. If your project uses C++, regenerate project files and build the project.
4. Open Unreal Engine and enable **Soft Targeting** in the Plugins window.
5. Add a **Soft Targeting** component to your player character.

For a C++ game module that uses the runtime component directly, add:

```csharp
PublicDependencyModuleNames.Add("SoftTargeting");
```

Blueprint-only projects do not need to modify a Build.cs file.

## Quick Start

The plugin intentionally ships without a test map, mannequin, or placeholder target mesh. It can be integrated directly into Unreal Engine's standard Third Person template.

1. Add a **Soft Targeting** component to the player character.
2. Add `Enemy` to **Required Actor Tags** on the component.
3. Give target actors the `Enemy` Actor Tag.
4. Make sure their collision object type is included in **Target Object Types** and allows overlap queries.
5. At attack input time, call **Find Best Target** with values matching that attack.
6. Use the returned actor for rotation, Motion Warping, attack logic, or hit detection.

For a quick Third Person test, create a Blueprint child of `BrawlerTargetDummy`, assign Manny or Quinn to its inherited Mesh component, place several instances in `Lvl_ThirdPerson`, and enable **Debug Targeting**.

See the [Wiki](https://github.com/Yhzan95/UE5SoftTargetingSystem/wiki) for the complete setup.

## Basic Attack Flow

```text
Attack Input
    -> Make Default Request
    -> Set attack-specific range, angle and tolerance
    -> Find Best Target
    -> Is Valid?
        -> Lock Current Target
        -> Rotate or Motion Warp toward the target
        -> Play the attack montage
        -> Perform hit detection
        -> Unlock Current Target when the attack ends
```

### C++ Example

```cpp
FSoftTargetingRequest Request = SoftTargeting->MakeDefaultRequest();
Request.MaxRange = 180.0f;
Request.MaxAngle = 70.0f;
Request.RangeTolerance = 60.0f;

if (AActor* Target = SoftTargeting->FindBestTarget(Request))
{
    SoftTargeting->LockCurrentTarget();
    StartAttack(Target);
}
```

Always call `UnlockCurrentTarget()` when the attack or montage ends, including interrupted attacks.

## Per-Attack Targeting

`FSoftTargetingRequest` lets every attack use a different selection shape.

| Attack example | Range | Half-angle | Tolerance |
|---|---:|---:|---:|
| Light punch | 180 cm | 70° | 40–60 cm |
| Roundhouse kick | 250 cm | 120° | 60–100 cm |
| Heavy attack | 220 cm | 80° | 60 cm |
| Sweep | 220 cm | 180° | 50 cm |
| Dash attack | 450 cm | 45° | 100–150 cm |
| Grab | 160 cm | 60° | 0–30 cm |

`MaxAngle` is a **half-aperture**: `180°` accepts every direction, `90°` accepts the entire front half, and `45°` creates a narrow forward cone.

`RangeTolerance` allows a near-miss target to remain selectable for rotation or Motion Warping while still reporting `InAttackRange = false` until it enters the attack's real range.

## Target Filtering

Targets can be accepted or rejected through:

- collision object types;
- required target classes;
- required and blocked Actor Tags;
- required and blocked Gameplay Tags;
- a runtime ignored-actor list;
- the optional `SoftTargetableInterface`.

Implement `SoftTargetableInterface` when an enemy needs to expose gameplay state. It can provide whether the actor can currently be targeted, whether it is alive, the exact world-space point to aim at, and a normalized priority value.

Plain actors do not need the interface. Correct collision plus the `Enemy` Actor Tag is enough for the minimum setup.

## Blueprint Events

| Event | Fired when |
|---|---|
| `OnTargetChanged` | The current target changes, including transitions to or from `None`. |
| `OnTargetFound` | A valid actor becomes the current target. |
| `OnTargetLost` | The previous target stops being current. |
| `OnCandidateEntered` | An actor becomes a valid background candidate. |
| `OnCandidateExited` | An actor stops being a valid background candidate. |
| `OnTargetBecameObstructed` | The current target remains hidden longer than the configured visibility grace time. |
| `OnTargetBecameVisible` | An obstructed current target becomes visible again. |
| `OnTargetLeftAttackRange` | The current target leaves the default attack range. |
| `OnTargetBackInRange` | The current target returns to the default attack range. |

Candidate enter/exit events come from the background refresh, not from narrow per-attack queries.

## Debug Visualization

Enable **Debug Targeting** to inspect the system in real time. The debug view can display:

- detection radius;
- default attack range and attack cone;
- character-facing and movement-input directions;
- lines to evaluated candidates;
- actual target points;
- candidate score labels and full score breakdowns;
- a stable on-screen targeting summary.

Target evaluation remains throttled at the configured refresh interval while debug rendering updates every frame for smooth visualization. Debug drawing is excluded from Shipping builds.

## Included Modules

### `SoftTargeting`

Asset-independent runtime module containing:

- `USoftTargetingComponent`
- `FSoftTargetingRequest`
- `FSoftTargetData`
- `ISoftTargetableInterface`
- automated scoring and API tests

### `SoftTargetingExamples`

Optional C++ integration examples containing:

- `ABrawlerCharacter`
- `UBrawlerCombatComponent`
- `ABrawlerTargetDummy`

The examples module does not include a playable map or mannequin assets. Remove `SoftTargetingExamples` from the plugin descriptor if you do not want the example classes in your final project.

## Performance

The plugin is designed for one player facing approximately **3–30 enemies**.

A typical background refresh performs one sphere overlap and usually one visibility trace. Candidate sorting is capped by `MaxTrackedTargets`, while hidden leaders are checked within the `MaxLineOfSightChecks` budget.

The default refresh interval is `0.05` seconds, or 20 evaluations per second. Debug drawing can still update every rendered frame.

## Plugin Scope

Soft Targeting deliberately does **not** provide:

- damage or health systems;
- attack hit detection;
- combos or attack state machines;
- animation montages;
- character or camera movement;
- enemy AI;
- network replication.

These systems remain project-specific and consume the target selected by the plugin.

## Compatibility

- Unreal Engine 5.8
- Full C++ and Blueprint API
- Designed for single-player gameplay
- Current release package built and tested on Win64
- Plugin modules allow Win64, macOS, Linux, iOS, and Android
- No third-party libraries
- No runtime dependency on project content, UI, animation, or Motion Warping

## Documentation

- [GitHub Wiki](https://github.com/Yhzan95/UE5SoftTargetingSystem/wiki)
- [Getting Started](Documentation/GettingStarted.md)
- [Blueprint API](Documentation/BlueprintAPI.md)
- [Changelog](CHANGELOG.md)

## Support

When reporting a problem, please include:

- Unreal Engine version;
- plugin version;
- Blueprint or C++ setup;
- relevant collision and tag settings;
- a screenshot of the debug view;
- exact reproduction steps.

Use [GitHub Issues](https://github.com/Yhzan95/UE5SoftTargetingSystem/issues) for reproducible bugs and feature requests.

## License

Released under the [MIT License](LICENSE).
