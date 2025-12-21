# Drone Simulator Architecture

This document describes how the Drone Simulator SDK is structured internally and how the main systems interact.

At a high level, the plugin is split into three main Unreal modules plus a placeholder launcher module:

- `DroneSimulator` – runtime simulation, physics, gameplay integration and recording.
- `DroneSimulatorInput` – device discovery, axis mapping and calibration for HID devices.
- `DroneSimulatorEditor` – editor‑only asset types, importers and flight analysis tools.
- `DroneSimulatorLauncher` – currently an empty shell reserved for future launcher/packaging logic.

The sections below walk through each area and the typical data flow from player input to physics and back.

---

## Runtime module: `DroneSimulator`

Source: `Source/DroneSimulator`

The runtime module owns the core simulation and gameplay integration.

### Data assets (`Assets/`)

Directory: `Source/DroneSimulator/Assets`

These assets describe the physical configuration of a drone:

- `UDroneFrameAsset` – geometry, mass and drag coefficients for the frame.
- `UDroneMotorAsset` – motor constants such as KV and mass.
- `UDronePropellerAsset` – propeller geometry and metadata, including links to airfoil data.
- `UDroneBatteryAsset` – battery properties (voltage, capacity, mass).
- `UDroneAirfoilAssetTable` / `UDroneAirfoilAssetSimplified` – airfoil coefficient tables and simplified approximations used by the rotor model.
- `UFlightRecordAsset` – stores recorded telemetry for later playback in the editor.

Assets are authored in the editor and referenced by runtime components such as `UDroneMovementComponent`.

### Simulation core (`Simulation/`)

Directory: `Source/DroneSimulator/Simulation`

This namespace contains a small, self‑contained physics layer:

- `FSubstepBody` and `FSimulationWorld` – represent bodies and the sub‑stepped world used to integrate forces at a high rate.
- `FStructural` / `Structural.h` – builds structural models from the configured frame and propellers.
- `FInertia`, `FLinearDrag`, `FRotationalDrag` – compute inertial and drag properties.
- `Math.*` – numerical helpers plus `MathTests.cpp` with Unreal automation tests.
- `LogDebug.*` – utilities for capturing debug information during a simulation step.

`UDroneMovementComponent` uses these types to perform custom physics updates at a configurable tick rate (400 Hz by default).

### Rotor and propulsion models (`RotorModel/`, `PropulsionModel/`)

Directories:

- `Source/DroneSimulator/RotorModel`
- `Source/DroneSimulator/RotorModel/Bemt`
- `Source/DroneSimulator/PropulsionModel`

Key concepts:

- `URotorModelBase` – abstract base class for rotor models.
- `URotorModelBemt` – Blade‑Element Momentum Theory implementation; computes thrust/torque based on airfoil data, freestream velocity and blade geometry. Includes `ComputePropellerThrust.*`, `AirfoilCoefficients.*` and `PropellerThrust.*` plus tests.
- `UPropulsionModel` – high‑level abstraction for mapping controller output to per‑motor forces.
- `UPropulsionModelDynamics` – dynamics‑based propulsion model that uses the rotor model.
- `UPropulsionModelDirectSetpoint` – simpler model that applies setpoints more directly.

The propulsion model is owned by `UDroneMovementComponent` and is instantiated as an instanced sub‑object on the pawn.

### Control stack (`Controller/`)

Directory: `Source/DroneSimulator/Controller`

The control stack breaks down into three layers:

- **Player input**
  - `FDronePlayerInput` – logical drone axes (throttle, roll, pitch, yaw) coming from the input subsystem.
  - `FDroneControllerInput` – internal representation used inside flight modes.

- **Flight modes**
  - `UFlightModeBase` – abstract base class that turns `FDronePlayerInput` + `FFlightModeState` into a `FDroneSetpoint`.
  - `UFlightModeAngle`, `UFlightModeVelocity`, `UFlightModeAir` – concrete flight modes with different control schemes.
  - `FFlightModeState` – current linear/angular velocity, rotation and delta time used by a flight mode.

- **Controllers**
  - `UDroneController` – abstract controller that maps a setpoint and current angular velocity to `FPropellerSetThrottle`.
  - `UPidDroneController`, `UBasicDroneController` – example implementations based on PID or simpler logic.
  - `FPropellerSetThrottle` / `Throttle.*` – per‑motor throttle outputs.

`UDroneMovementComponent` owns a map of `UFlightModeBase` instances and a `UPropulsionModel`. Each sub‑step:

1. Reads `FDronePlayerInput`.
2. Builds an `FFlightModeState` from the current body state.
3. Asks the active `UFlightModeBase` to compute a `FDroneSetpoint`.
4. Passes the setpoint and angular velocity into `UDroneController::tick_controller`.
5. Hands the resulting `FPropellerSetThrottle` to the propulsion model, which talks to the rotor model and simulation world.

### Gameplay integration (`Gameplay/`)

Directory: `Source/DroneSimulator/Gameplay`

The core gameplay types are:

- `ADronePawn` – pawn that owns a `UDroneMovementComponent` and binds it into the normal Unreal pawn lifecycle.
- `UDroneMovementComponent` – movement component responsible for:
  - Converting assets into runtime structs (`FDroneFrame`, `FDroneMotor`, etc.).
  - Scheduling custom physics via `FCalculateCustomPhysics`.
  - Running the sub‑stepped simulation (thrust + drag) at the configured tick rate.
  - Managing flight modes, controllers and player input.
  - Exposing debug information through `FPropulsionInfo` and other structs.
- `ADronePlayerController` – optional player controller glue for routing input.
- `ADroneGameState` – game‑level state for more complex setups.

#### Recording (`Gameplay/Recording`)

Directory: `Source/DroneSimulator/Gameplay/Recording`

- `UDroneFlightRecordingManager` – orchestrates recording of per‑frame telemetry.
- `FFlightRecord` / `FlightRecord.*` – raw recording data structure.
- `FPropulsionInfo` – detailed per‑propeller debug state captured during simulation.

Recordings are saved into `UFlightRecordAsset` assets, which are later consumed by the editor playback tools.

#### In‑game UI (`Gameplay/UI`)

- `SDroneControllerCalibrationWidget` – Slate widget used for in‑editor/in‑game controller calibration flows built on top of the input subsystem.

---

## Input module: `DroneSimulatorInput`

Source: `Source/DroneSimulatorInput`

This module encapsulates device handling and maps physical inputs to logical drone axes.

Key types:

- `UDroneInputSubsystem` – a `UEngineSubsystem` and `FTickableGameObject` that:
  - Registers input devices and maintains their state.
  - Maps device‑specific axis names to logical axes such as throttle/roll/pitch/yaw.
  - Stores calibration data per device and axis.
  - Provides Blueprint-callable getters for raw values and calibration state.

- `UDroneInputLocalPlayerSubsystem` - a `ULocalPlayerSubsystem` that:
  - Holds per-player precision/throttle configuration.
  - Dispatches calibrated axes into Unreal input for the owning local player.
- `FDroneInputProcessor` – processes raw events and forwards them to `UDroneInputSubsystem`.
- `FHIDInputManager` and `FHIDDevice` (in `Private/Windows`) – low‑level HID polling and enumeration.
- `FDroneInputCalibrator` / `FDroneInputProfileManager` – helpers for calibration flows and saving/loading profiles.
- `FDroneInputDevice`, `EDroneInputAxis`, `FInputAxisMapping`, `FAxisCalibrationData` – core data types defined in `DroneInputTypes.h`.

Typical flow:

1. The HID manager discovers devices and reports raw axis/button events.
2. `FDroneInputProcessor` normalizes events and calls into `UDroneInputSubsystem` APIs like `handle_raw_analog_input`.
3. `UDroneInputSubsystem` applies mappings and calibration, storing per-device axis state.
4. `UDroneInputLocalPlayerSubsystem` injects calibrated axes into Unreal input for each local player.
5. Game code builds `FDronePlayerInput` from Enhanced Input and passes it to the active flight mode.

---

## Editor module: `DroneSimulatorEditor`

Source: `Source/DroneSimulatorEditor`

This module provides Unreal Editor tooling around authoring assets and inspecting recorded flights.

Main responsibilities:

- **Asset type integration**
  - `FAssetTypeActions_*` classes for frames, motors, propellers, batteries, airfoils and flight records.
  - Makes assets appear in the Content Browser with custom categories and menus.

- **Factories and import pipeline (`Private/Factories`)**
  - `UDroneAirfoilFactory`, `UDroneAirfoilSimplifiedFactory`, `UDroneMotorFactory`, etc.
  - CSV/XFoil‑style parsing helpers in `AeroTableCSVParsing.h` and `AirfoilParsing.h`.
  - Create `UDroneAirfoilAssetTable` and related assets from aero‑tables generated by external tooling.

- **Detail customizations (`Private/Details`)**
  - `FDroneAirfoilAssetDetails` customizes the details panel for airfoil assets to make editing more ergonomic.

- **Flight playback (`Private/Playback`)**
  - `UFlightPlaybackManager` – wraps loading and stepping through `UFlightRecordAsset` data.
  - `AFlightPlaybackVisualizationActor` – optional actor that visualizes recorded data in the level.

- **Timeline & visualization widgets (`Private/Widgets`)**
  - `STimelinePanel`, `STimelineTrack` – main “Drone Timeline” editor tab for scrubbing, play/pause and time navigation.
  - `SAirfoilGraphWidget` – plots airfoil coefficient curves.
  - `SFlightDataWidget`, `SPropellerDisplayWidget`, `SSinglePropellerWidget` – various telemetry and rotor visualization widgets.

The module entry point `FDroneSimulatorEditorModule` wires everything up:

- Registers custom asset actions and detail layouts in `StartupModule()`.
- Registers the “Drone Timeline” tab with the Level Editor.
- Hooks into PIE begin/end delegates to automatically open recordings in the timeline.

---

## Launcher module: `DroneSimulatorLauncher`

Source: `Source/DroneSimulatorLauncher`

This module is currently just a directory stub and does not contain code.  
It is reserved for potential future launcher / standalone‑simulator glue. You can safely ignore it for now.

---

## Per‑frame data flow

Putting it all together, a typical simulation step looks like:

1. **Input**
   - HID devices report events to `UDroneInputSubsystem`.
   - The device subsystem stores calibrated axis state per device.
   - `UDroneInputLocalPlayerSubsystem` dispatches input for each local player.
   - Enhanced Input receives calibrated logical axes.

2. **Flight mode**
   - `UDroneMovementComponent` reads `FDronePlayerInput`.
   - It builds `FFlightModeState` from the current body state.
   - The active `UFlightModeBase` returns a target `FDroneSetpoint`.

3. **Controller**
   - `UDroneController::tick_controller` computes `FPropellerSetThrottle` based on the setpoint and angular velocity.

4. **Propulsion & rotor model**
   - The active `UPropulsionModel` turns throttle values into forces/torques via `URotorModelBase` (often `URotorModelBemt`).
   - Forces are applied to the `FSubstepBody` in the simulation world.

5. **Integration & recording**
   - The simulation world integrates motion for the current sub‑step.
   - `UDroneMovementComponent` updates the pawn’s transform and records telemetry into `UDroneFlightRecordingManager`.

6. **Editor analysis (offline)**
   - Recorded `UFlightRecordAsset` instances are opened in the Drone Timeline for inspection and debugging.

Use this flow as a starting point when extending the SDK with new flight modes, controllers, rotor models or editor tooling.

