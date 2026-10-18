# Crashy Development Guide

This guide covers building the **STM32H755ZIT6 dual-core firmware** on a development machine. Run all commands from the **Crashy repository root**.

## Requirements

- CMake **3.22 or newer**
- Ninja
- Arm GNU Toolchain (`arm-none-eabi-gcc`)

Verify that the tools are available:

```bash
cmake --version
ninja --version
arm-none-eabi-gcc --version
```

> STM32CubeMX is needed when changing and regenerating the MCU configuration, **not** for an ordinary firmware build. The generated HAL/CMSIS code is kept in the repository.

## Build firmware

### Debug

```bash
cmake --preset Debug
cmake --build --preset Debug
```

### Release

```bash
cmake --preset Release
cmake --build --preset Release
```

The first command configures the selected build; the second compiles and links **both cores**. Re-run the build command after changing source files. Re-run the configure command if you change CMake configuration.

## Build outputs

The root-level CMake build stores outputs under the Git-ignored `build/` directory:

```text
Crashy/
└── build/
    ├── Debug/
    │   ├── CM7/
    │   │   └── cubemx_CM7.elf
    │   └── CM4/
    │       └── cubemx_CM4.elf
    └── Release/
        ├── CM7/
        │   └── cubemx_CM7.elf
        └── CM4/
            └── cubemx_CM4.elf
```

These paths show the ELF files; each core's directory also contains generated CMake/Ninja and object files.

Find all firmware ELF files:

```bash
find build/ -name '*.elf'
```

- **CM7**: Cortex-M7 firmware (planned flight-control application)
- **CM4**: Cortex-M4 firmware (planned supporting services)
- **`.elf`**: Linked firmware executable containing symbols useful for debugging

The current build verifies compilation and linking. **It does not prove that the firmware runs correctly on hardware.**

## Clean / rebuild

To remove only the Debug build outputs and start fresh:

```bash
rm -rf build/Debug
cmake --preset Debug
cmake --build --preset Debug
```

Use `build/Release` instead to clean Release. Do not delete `firmware/platform/stm32/cubemx/`—that directory contains tracked generated sources and configuration.

## CubeMX configuration

- MCU configuration: `firmware/platform/stm32/cubemx/cubemx.ioc`
- Generated core projects: `firmware/platform/stm32/cubemx/CM7/` and `CM4/`
- HAL/CMSIS drivers: `firmware/platform/stm32/cubemx/Drivers/`

Keep custom application and module code **outside** the CubeMX-generated source directories. Use the **repository-root** CMake presets above for normal development; CubeMX's own standalone presets may use a different build output layout.

## Before opening a pull request

1. Confirm Debug and Release builds complete successfully.
2. Check `git status` to ensure that no build artifacts are staged.
3. Include relevant firmware and `.ioc` changes in the PR; explain any hardware assumptions that changed.
