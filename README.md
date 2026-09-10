# Soft Targeting

![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.8-0E1128?logo=unrealengine&logoColor=white)
![C++ and Blueprint](https://img.shields.io/badge/C%2B%2B%20%2B%20Blueprint-Ready-00599C?logo=cplusplus&logoColor=white)
![Version](https://img.shields.io/badge/version-1.1.0-2ea44f)

**Directional melee target selection for Unreal Engine.**

Soft Targeting automatically finds the enemy the player most likely intends to attack. It combines
movement input, character facing, distance, camera framing, target priority, visibility and target
persistence without forcing a permanent camera lock.

[Download the latest release](../../releases/latest) · [Open the Wiki](../../wiki) ·
[Report an issue](../../issues) · [View the changelog](CHANGELOG.md)

## Overview

In a melee fight with several nearby enemies, choosing the closest actor is rarely enough. The
closest enemy may be behind the player while the stick, character and camera are clearly pointing
toward somebody else.

Soft Targeting evaluates every valid candidate and returns the most intentional choice for the next
attack. The plugin only provides the target and useful targeting data. Your own combat system stays
in control of movement, rotation, attacks, damage, animation and camera behavior.

## Key features

| Feature | Description |
|---|---|
| Directional scoring | Combines character facing, movement input, camera direction, distance and designer-authored priority. |
| Per-attack queries | Give every punch, kick, sweep, grab or dash its own range, cone and range tolerance. |
| Stable target persistence | Prevents small score changes from causing constant target switching. Deliberate stick input can override it. |
| Line of sight | Rejects enemies hidden by level geometry, with a short grace period to prevent wall-edge flicker. |
| Target lock | Keeps the selected enemy stable during an attack montage without locking the camera. |
| Forced targets | Pins a target for grabs, throws, finishers and scripted actions. |
| Flexible filtering | Supports actor classes, Actor Tags, Gameplay Tags, ignored actors and an optional targetable interface. |
| Blueprint events | Reports target, candidate, visibility and attack-range transitions. |
| Motion Warping helper | Calculates a target-facing warp transform and a safe stopping position. |
| Layered debug view | Individually toggle radii, attack cone, direction arrows, target lines, target points, scores and state text. |
| Optimized evaluation | Uses a candidate cap, line-trace budget, cached target positions and a configurable refresh interval. |

## Soft targeting versus hard lock-on

| Soft Targeting | Traditional Lock-On |
|---|---|
| Selects a target when the player attacks | Keeps one target selected continuously |
| Does not rotate the camera | Often controls or recenters the camera |
| Allows fast directional changes | Usually requires explicit target switching |
| Works naturally in crowds | Focuses combat on one opponent |
| Can be invisible to the player | Commonly displays a lock-on marker |

Both systems can coexist. Soft Targeting can handle normal melee attacks while a separate lock-on
system handles focused encounters.

## How selection works

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

Only enabled criteria participate in normalization. When the stick is inside the dead zone, the
input criterion is removed from the formula instead of reducing every candidate's score.

## Recommended use cases

- Third-person brawlers and beat 'em ups
- Action-adventure melee combat
- Hand-to-hand combat systems
- Swords and short-range weapons
- Directional attacks in enemy crowds
- Grabs, throws and finishers
- Root-motion attacks and Motion Warping
- Combat systems where hard lock-on would feel too restrictive

## Quick start

The plugin intentionally ships without a test map, mannequin or placeholder target mesh. It can be
integrated directly into the standard Third Person template.

1. Copy `SoftTargeting` into `YourProject/Plugins/`.
2. Enable **Soft Targeting** and restart Unreal Engine.
3. Add a **Soft Targeting** component to the player character.
4. Add `Enemy` to **Required Actor Tags** on the component.
5. Give target actors the `Enemy` Actor Tag and a query-enabled Pawn collision object.
6. Call **Find Best Target** when an attack input is pressed.
7. Use the returned actor in your combat, rotation or Motion Warping logic.

For a quick Third Person test, create a Blueprint child of `BrawlerTargetDummy`, assign Manny or
Quinn to its inherited Mesh component, and place several instances in `Lvl_ThirdPerson`.

The complete setup tutorial will be maintained in the [Wiki](../../wiki).

## Basic attack flow

```text
Attack Input
    -> Make Default Request
    -> Set attack-specific range, angle and tolerance
    -> Find Best Target
    -> Is Valid?
        -> Lock Current Target
        -> Rotate or Motion Warp toward the target
        -> Play the attack montage
        -> Perform your hit detection
        -> Unlock Current Target when the attack ends
```

Example C++ query:

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

Add the runtime module to any C++ module that uses the component directly:

```csharp
PublicDependencyModuleNames.Add("SoftTargeting");
```

Blueprint-only projects can add the component from the Components panel without changing a build
file.

## Per-attack targeting

An `FSoftTargetingRequest` lets every attack use a different selection shape.

| Attack example | Range | Half-angle | Tolerance |
|---|---:|---:|---:|
| Light punch | 180 cm | 70° | 40–60 cm |
| Roundhouse kick | 250 cm | 120° | 60–100 cm |
| Heavy attack | 220 cm | 80° | 60 cm |
| Sweep | 220 cm | 180° | 50 cm |
| Dash attack | 450 cm | 45° | 100–150 cm |
| Grab | 160 cm | 60° | 0–30 cm |

`MaxAngle` is a half-aperture: `180°` accepts every direction, `90°` accepts the entire front half,
and `45°` creates a narrow forward cone.

`RangeTolerance` keeps a near-miss target selectable for rotation or Motion Warping while reporting
that it is not yet inside the attack's real range.

## Target filtering

Targets can be accepted or rejected through:

- collision object types;
- required target classes;
- required and blocked Actor Tags;
- required and blocked Gameplay Tags;
- a runtime ignore list;
- the optional `SoftTargetableInterface`.

Implement `SoftTargetableInterface` when an enemy needs to expose its gameplay state. It can provide:

- whether it can currently be targeted;
- whether it is alive;
- the exact world-space point to aim at;
- a normalized target priority.

Plain actors do not need the interface. An actor with the correct collision and `Enemy` tag is
enough for the minimum setup.

## Blueprint events

| Event | Fired when |
|---|---|
| `OnTargetChanged` | The current target changes, including transitions to or from `None`. |
| `OnTargetFound` | A valid actor becomes the current target. |
| `OnTargetLost` | The previous target stops being current. |
| `OnCandidateEntered` | An actor becomes a valid background candidate. |
| `OnCandidateExited` | An actor stops being a valid background candidate. |
| `OnTargetBecameObstructed` | The current target remains hidden longer than the visibility grace time. |
| `OnTargetBecameVisible` | An obstructed current target becomes visible again. |
| `OnTargetLeftAttackRange` | The current target leaves the default attack range. |
| `OnTargetBackInRange` | The current target returns to the default attack range. |

## Debug visualization

Enable **Debug Targeting** to inspect the system in real time. The debug view can display:

- the detection radius;
- default attack range and attack cone;
- character-facing and movement-input directions;
- colored lines to candidates;
- target points and score labels;
- the full score breakdown;
- a stable on-screen targeting summary.

Target evaluation remains throttled at the configured refresh rate while debug drawing updates every
rendered frame, preventing visible jitter during character movement.

Every debug layer can be disabled independently. Debug drawing is excluded from Shipping builds.

## Included source modules

### `SoftTargeting`

The asset-independent runtime module containing:

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

The example module does not include a playable map or mannequin assets. Remove it from the plugin
descriptor if the example classes are not wanted in a final project.

## Performance

The plugin is designed for one player facing approximately 3–30 enemies. A typical background
refresh performs one sphere overlap and one visibility trace. Candidate sorting is capped by
`MaxTrackedTargets`, and hidden leaders are checked within the `MaxLineOfSightChecks` budget.

The default refresh interval is `0.05` seconds, or 20 evaluations per second. This keeps target
selection responsive while the debug display remains visually smooth at the rendered frame rate.

## Scope

Soft Targeting deliberately does not provide:

- damage or health systems;
- attack hit detection;
- combos or attack state machines;
- animation montages;
- character or camera movement;
- enemy AI;
- network replication.

These systems stay project-specific and consume the target selected by the plugin.

## Compatibility

- Unreal Engine 5.8
- Full C++ and Blueprint API
- Designed for single-player gameplay
- Current release package built and tested on Win64
- No third-party libraries
- No runtime dependency on project content, UI, animation or Motion Warping

## Documentation

- [Getting Started](Documentation/GettingStarted.md)
- [Blueprint API](Documentation/BlueprintAPI.md)
- [Complete French Configuration Reference](Documentation/ConfigurationReference_FR.md)
- [Changelog](CHANGELOG.md)
- [GitHub Wiki](../../wiki) — tutorials, Blueprint screenshots and integration guides

Suggested Wiki sections:

1. Installation
2. Third Person template setup
3. Creating targetable enemies
4. Performing an attack query
5. Understanding score weights
6. Target locking and forced targets
7. Motion Warping integration
8. Debug visualization
9. Troubleshooting and FAQ

## Support

When reporting a problem, include:

- Unreal Engine version;
- plugin version;
- Blueprint or C++ setup;
- relevant collision and tag settings;
- a screenshot of the debug view;
- the steps needed to reproduce the issue.

Use [GitHub Issues](../../issues) for reproducible bugs and feature requests.
