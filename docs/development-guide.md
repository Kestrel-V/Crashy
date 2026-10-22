
# Crashy Development Guide

Crashy targets the **STM32H755ZIT6** (Cortex-M7 + Cortex-M4). Development is supported on **Ubuntu Linux with VS Code**. Unless noted, run commands from the repository root.

## 1. Requirements

| Tool | Requirement | Initially tested |
| --- | --- | --- |
| CMake | 3.22+ | 4.2.3 |
| Ninja | Installed and on `PATH` | 1.13.2 |
| ARM GNU Toolchain | `arm-none-eabi-gcc` on `PATH` | GCC 14.2.1 |
| Python | Python 3 | System installation |
| clangd / clang-format | Recommended for editing / formatting | 21.1.8 |
| GDB Multiarch / OpenOCD | Only needed for hardware debugging | 17.1 / 0.12.0 |

Verify the build tools:

```bash
cmake --version
ninja --version
arm-none-eabi-gcc --version
python3 --version
```

STM32CubeMX is required to *regenerate* MCU configuration, not to build existing firmware; generated HAL/CMSIS sources are committed.

## 2. Build Firmware

```bash
# Debug — both cores
cmake --preset Debug
cmake --build --preset Debug

# Release — both cores
cmake --preset Release
cmake --build --preset Release
```

Each preset configures and builds **both CM7 and CM4**. Ninja rebuilds changed files incrementally. Reconfigure after changing CMake configuration.

| Configuration | CM7 ELF | CM4 ELF |
| --- | --- | --- |
| Debug | `build/Debug/CM7/cubemx_CM7.elf` | `build/Debug/CM4/cubemx_CM4.elf` |
| Release | `build/Release/CM7/cubemx_CM7.elf` | `build/Release/CM4/cubemx_CM4.elf` |

`build/` is Git-ignored. Compilation proves the firmware links, **not** that it runs correctly on hardware.

To rebuild from a clean Debug configuration:

```bash
rm -rf build/Debug
cmake --preset Debug
cmake --build --preset Debug
```

## 3. VS Code and clangd

Open the repository with `code .`. Install the extensions recommended in `.vscode/extensions.json`:

- **CMake Tools** (`ms-vscode.cmake-tools`)
- **clangd** (`llvm-vs-code-extensions.vscode-clangd`)
- **Cortex-Debug** (`marus25.cortex-debug`)

CMake Tools uses the same Debug/Release presets as the terminal. Select matching configure/build presets, then run **CMake: Configure** and **CMake: Build**.

Run a **Debug build first** to generate `build/Debug/CM7/compile_commands.json` and `build/Debug/CM4/compile_commands.json`. The root `.clangd` file selects the correct database by source path; shared CubeMX sources default to CM7 analysis. clangd queries ARM GCC for target headers using `--query-driver=**/arm-none-eabi-gcc`. Microsoft's C/C++ IntelliSense is disabled to avoid duplicate diagnostics.

## 4. Formatting

Use the repository's `.clang-format` rules (CI uses **clang-format 21.1.8**). Format a handwritten file with `clang-format -i path/to/file.c` or VS Code's **Format Document**.

Check *all tracked handwritten* C/header files before a PR:

```bash
git ls-files -z -- '*.c' '*.h' |
  grep -zEv '^(firmware/platform/stm32/cubemx/|third_party/)' |
  xargs -0 -r clang-format --dry-run --Werror
```

Do not format CubeMX-generated or third-party code. `.clang-format-ignore` excludes these directories, and `firmware/platform/stm32/cubemx/.clang-format` disables formatting for generated files in the editor. Automatic Format on Save/Type is disabled for C/C++.

## 5. Branches, Pull Requests, and Reviews

Work in short-lived branches; do **not** develop directly on protected `main`. Use descriptive names such as `feat/imu-driver`, `fix/uart-init`, `ci/format-check`, or `docs/dev-guide`.

```bash
git switch main
git pull --ff-only
git switch -c feat/example

# After making and testing changes
git status --short
git diff --check
git add path/to/changed-file
git diff --cached --check
git diff --cached
git commit -m "feat: describe the change"
git push -u origin feat/example
```

Open a PR **into `main`**. Briefly explain *what changed, why, and how it was tested*. Request a teammate's review; respond to feedback by pushing updates to the same branch.

Merging requires **one teammate approval** and a passing **Firmware Build** status check. Use **Squash and merge** once both requirements are met. GitHub automatically deletes eligible merged remote branches. You may prune deleted remote references locally with `git fetch --prune`.

## 6. GitHub Actions CI

`.github/workflows/firmware-build.yml` runs for PRs targeting `main` and pushes to `main`. On Ubuntu 24.04, it:

1. Restores or installs **ARM GNU Toolchain 14.2.Rel1 (GCC 14.2.1)** and verifies the build tools.
2. Installs **clang-format 21.1.8** and checks tracked handwritten C/header files without modifying them.
3. Builds **Debug and Release** for both CM7 and CM4 using the CMake presets above.
4. Fails if any build fails or any of the **four ELF files** is missing or empty.

If CI fails, open the failed GitHub Actions step, read its logs, reproduce the relevant command locally, fix the issue, and push another commit to the PR. CI does not flash hardware or run hardware-in-the-loop tests.

## 7. Hardware Debugging (Not Yet Validated)

Planned stack: **VS Code → Cortex-Debug → GDB Multiarch → OpenOCD → ST-LINK → STM32H755 (SWD)**. GDB can read both Debug ELF files, but physical flashing, reset behavior, and simultaneous core debugging remain untested.

Proposed OpenOCD command (requires hardware validation):

```bash
openocd \
  -f interface/stlink-dap.cfg \
  -c "transport select dapdirect_swd" \
  -c "set DUAL_CORE 1" \
  -f target/stm32h7x_dual_bank.cfg
```

A shared `.vscode/launch.json` will be added after hardware bring-up confirms the debug configuration.

## 8. CubeMX and Troubleshooting

- MCU configuration: `firmware/platform/stm32/cubemx/cubemx.ioc`; core projects: `CM7/` and `CM4/` beneath that directory.
- Keep handwritten application/module code outside CubeMX-generated directories. Preserve application changes to generated `main.c` inside **USER CODE** regions.
- Per-core `CMakeLists.txt` files contain the application's build integration. Avoid unnecessary edits to `mx-generated.cmake`, HAL, and startup sources.
- **Compiler not found:** Check `arm-none-eabi-gcc --version` and `PATH`.
- **clangd missing headers/wrong flags:** Build Debug, confirm both compilation databases exist, check `.clangd`, then restart clangd.
- **VS Code missing presets:** Open the repository root and verify that CMake Tools is installed.
- **Unexpected generated-code changes:** Inspect formatting exclusions and staged diff before committing.
