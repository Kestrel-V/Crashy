# Crashy - Project Structure (v1.1)

> Dual-core STM32 FPV drone firmware architecture (Cortex-M7 + Cortex-M4).

## 1. Overview

This document defines the target repository layout, firmware architecture, and directory responsibilities for Crashy. For build, CI, and tooling instructions see [development-guide.md](development-guide.md).

The firmware is organized into three primary layers:

- **Application:** Core-specific initialization, task creation, and scheduling.
- **Modules:** Flight-control functionality, algorithms, sensors, safety, and communication.
- **Platform:** Hardware-specific implementation and STM32 integration.

Portable algorithms (controls, estimation, math) live in `modules/` alongside the other modules.

### Current Status

The repository is at the build-system bring-up stage. Sections 2-6 describe the **target** architecture; only the following exists today:

| Area | State |
|------|-------|
| `firmware/app/cm7/`, `firmware/app/cm4/` | Implemented: stub `cm7_app_init/step` and `cm4_app_init/step` (no tasks yet) |
| `firmware/platform/stm32/cubemx/` | Implemented: CubeMX-generated CM7 and CM4 projects (HAL, CMSIS, startup, linker scripts) |
| Root `CMakeLists.txt` + `CMakePresets.json` | Implemented: Debug/Release presets build both cores as separate ExternalProjects |
| `.github/workflows/` | Implemented: Firmware Build (clang-format check, Debug + Release builds of both cores) |
| `docs/` | `architecture.md`, `development-guide.md` |
| `firmware/modules/`, `firmware/platform/{api,board}`, `firmware/platform/stm32/{src,CMakeLists.txt}`, `cmake/`, `tests/`, `tools/`, `hardware/`, `software/`, `third_party/` | **Planned**, not yet created |

---

## 2. Directory Layout

```text
Crashy/
│
├── .github/
│   └── workflows/             # CI/CD
├── .vscode/                   # VS Code configuration
├── cmake/                     # Build helpers and toolchains (planned)
│
├── firmware/
│   ├── app/
│   │   ├── cm7/               # Flight control + FreeRTOS tasks
│   │   │   ├── inc/cm7/
│   │   │   └── src/
│   │   │       └── tasks/
│   │   └── cm4/               # Background services + FreeRTOS tasks
│   │       ├── inc/cm4/
│   │       └── src/
│   │           └── tasks/
│   │
│   ├── modules/
│   │   ├── flight/            # Flight-loop coordination
│   │   ├── safety/            # Arming, failsafe, fault handling
│   │   ├── devices/
│   │   │   ├── imu/
│   │   │   └── esc/
│   │   ├── controls/          # PID controllers and control algorithms
│   │   ├── estimation/        # State estimation and sensor fusion
│   │   ├── math/              # Math utilities
│   │   ├── receiver/          # RC input / CRSF decoding
│   │   ├── telemetry/         # Telemetry processing
│   │   ├── configuration/     # Settings management
│   │   └── intercore/         # CM7 <-> CM4 communication
│   │
│   └── platform/
│       ├── api/
│       │   └── inc/platform/
│       ├── board/
│       │   ├── inc/board/
│       │   └── src/
│       └── stm32/
│           ├── cubemx/        # Generated STM32 files
│           ├── src/
│           │   ├── cm7/
│           │   ├── cm4/
│           │   └── ipc/
│           └── CMakeLists.txt
│
├── hardware/                  # PCB, schematics, BOM
├── software/                  # Ground station / PC applications
├── tests/                     # Unit and integration tests
├── tools/                     # Developer scripts
├── docs/                      # Project documentation
├── third_party/               # External dependencies
│
├── .clang-format
├── .clang-format-ignore       # Excludes CubeMX/third-party code from formatting
├── .clangd                    # Per-core clangd compilation database selection
├── .gitignore
├── CMakeLists.txt
├── CMakePresets.json
└── README.md
```

---

## 3. Firmware Architecture

### 3.1 Application Layer — `firmware/app/`

Responsible for core-specific application initialization, FreeRTOS task creation, and scheduling.

| Directory | Responsibility |
|-----------|----------------|
| `cm7/` | Flight control and time-critical execution |
| `cm4/` | Background services and supporting tasks |

Each core owns its application entry point and tasks.

### 3.2 Module Layer — `firmware/modules/`

Contains the functional components of the flight controller.

| Module | Responsibility |
|--------|----------------|
| `flight/` | Flight-loop coordination |
| `safety/` | Arming, failsafe, and fault handling |
| `controls/` | PID controllers and other control algorithms |
| `estimation/` | State estimation and sensor fusion |
| `math/` | Mathematical utilities and supporting functions |
| `devices/` | Device-level sensor and actuator drivers |
| `receiver/` | RC input and CRSF decoding |
| `telemetry/` | Telemetry collection and processing |
| `configuration/` | Configuration coordination and persistence |
| `intercore/` | CM7 <-> CM4 communication |

**Module conventions:**
- Each module owns its implementation and public interfaces.
- Public headers are organized under `inc/<module>/`.
- Module-internal details remain private.
- Modules communicate through direct API calls or targeted FreeRTOS mechanisms.

### 3.3 Platform Layer — `firmware/platform/`

Isolates hardware-dependent implementations from higher-level firmware.

| Directory | Responsibility |
|-----------|----------------|
| `api/` | Hardware abstraction interfaces |
| `board/` | PCB-specific configuration and pin mappings |
| `stm32/cubemx/` | STM32CubeMX-generated files |
| `stm32/src/` | STM32 HAL/LL-based implementations |
| `stm32/src/ipc/` | Shared memory and hardware semaphore backend |

**Design principle:** Hardware-specific details should be kept out of portable modules wherever practical.

---

## 4. Repository Directories

| Directory | Responsibility |
|-----------|----------------|
| `hardware/` | PCB schematics, layouts, BOM, and pinouts |
| `software/` | Ground station and PC-side applications |
| `tests/` | Unit, integration, and simulation tests |
| `tools/` | Flashing, calibration, and debugging scripts |
| `docs/` | Architecture decisions and developer documentation |
| `third_party/` | External libraries and dependencies |
| `cmake/` | ARM toolchain configuration and reusable build logic |
| `.github/` | CI/CD workflows and repository automation |
| `.vscode/` | Recommended extensions and build/debug settings |

---

## 5. Project Configuration Files

| File | Purpose |
|------|---------|
| `.clang-format` | C/C++ formatting rules (CI pins clang-format 21.1.8) |
| `.clang-format-ignore` | Excludes generated and third-party code from formatting |
| `.clangd` | Selects the CM7 or CM4 compilation database by source path |
| `.gitignore` | Files excluded from Git |
| `CMakeLists.txt` | Root CMake build; builds CM7 and CM4 as separate ExternalProjects |
| `CMakePresets.json` | Debug/Release presets, each building both cores |
| `README.md` | Project introduction and setup instructions |

---

## 6. Architectural Summary

- **Dual-core design:** Cortex-M7 and Cortex-M4 have separate application entry points and task ownership.
- **Modular firmware:** Functionality is divided into self-contained modules with clear interfaces.
- **Hardware abstraction:** STM32-specific implementation is separated from portable application logic.
- **Module-owned interfaces:** Each module defines and exposes its own public API.
- **Module-owned configuration:** Modules own their settings, with central coordination for persistence and initialization.
- **Communication:** Direct calls are preferred within a core; FreeRTOS communication mechanisms are used where needed.
- **Portable algorithms:** Controls, estimation, and math modules remain independent of the target hardware.
