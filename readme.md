# Drone Simulator SDK for Unreal Engine

A physics-first SDK for simulating multirotor drones inside Unreal Engine.
Use it as a reusable C++/Blueprint toolkit to build your own simulators, tools, and games.

> This plugin is designed to live inside your Unreal project as an engine- and editor-first SDK, not as a ready-made arcade drone game.

## Architecture

This project consists in multiple parts:
- `Content` contains demo Blueprints implementation and built-in drone parts.
- `aero_table` consists in Python scripts to build the airfoil lift and drag tables using Xfoil.
- `Source` contains the C++ source code modules:
  - `DroneSimulatorCore` is the simulator itself, it mainly include the physics core
  - `DroneSimulatorInput` allows the support of non-standard gamepads, such as EdgeTX and DJI
  - `DroneSimulatorGame` wraps the physics core with Unreal Engine classes, to make it available as Pawn, MovementComponent, assets, etc
  - `DroneSimulatorEditor` provides editor utilities, such as session replay, and asset editor

## Features

- 🔨**Modular drone parts**: pick your own frame, battery, propellers, and motors, using assets.
- 🚁 Simulate **high-fidely physics (BEMT)** or **arcade physics**.
- 🕹️ A **PID controller** that can be tuned.
- 🚀 **High-frequency physics** pipeline for multirotor drones.
- 🛸 Multiple flight modes: **race/freestyle** or **stabilized**
- 🏗️ Built for **extensibility**: flight modes, physics models and controllers are classes, that you can swap or extend.
- 🎮 **Input and calibration** support for drone RC transmitters (EdgeTX, DJI) and other HID devices.
- 👩‍💻 **Editor tooling** for authoring airfoil data and inspecting flight logs.

You drop the plugin into an existing Unreal project and compose the pieces you need.

## Who is it for?

- Game developers who want believable drones without writing all the physics from scratch.
- Researchers and engineers building training tools, evaluation environments or hardware-in-the-loop setups.
- Robotics and autonomy teams needing a controllable virtual drone embedded in Unreal.
- Educators who want to demonstrate multirotor dynamics in an interactive 3D environment.

## What can you build?

Typical use cases include:

- Evaluation environments for flight modes and control algorithms.
- Editor tools and dashboards for visualizing flight logs and telemetry.
- Educational playgrounds for learning multirotor dynamics.
- Drone-focused games built on top of a reusable physics stack.

## Quick start 🚀

1. **Install the plugin**
- Clone this repository into your project's `Plugins/` folder, e.g. `YourProject/Plugins/DroneSimulator/`.
- Open the project in Unreal and enable:
  - `Drone Simulator`
- Restart the editor if prompted.

2. **Spawn a drone**
- Add an `ADronePawn` to your level.
- On the pawn, configure a `UDroneMovementComponent` with frame, motor, propeller and battery assets.
- Select a propulsion model and one or more flight modes.

3. **Connect input (optional)**
- Open your Drone Input project settings and register a HID device.
- Map device axes to drone axes and run through calibration.
- In code or Blueprints, read `FDronePlayerInput` and feed it into your chosen flight mode.

From here you can start flying in PIE and iterate on your setup.

## License

This project is distributed under the GPL v3 license. For inquiries about alternative licensing options, please reach out to me at hutteau.b@gmail.com .

## Learn more

- For a deeper look at the modules and data flow, see [architecture.md](architecture.md).
- For notes about referenced brands and models, see [disclaimer.md](disclaimer.md).
