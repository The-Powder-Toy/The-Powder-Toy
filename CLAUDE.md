# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Quick Start: Build & Run

**Build with Meson:**
```bash
meson setup build
cd build
meson compile
```

**Run the game:**
```bash
./powder  # Linux/macOS
.\powder.exe  # Windows (from build directory)
```

**Build options** (pass with `-D` flag to `meson setup`):
- `-Dhttp=true`: Enable online features (saves, account sync)
- `-Dlua=auto`: Lua scripting support (`lua5.1`, `lua5.2`, `luajit`, or `none`)
- `-Dstatic=prebuilt`: Use prebuilt static libraries (cross-platform binary)
- `-Denforce_https=true`: Required for release builds
- Full options in `meson_options.txt`

## Architecture Overview

The Powder Toy is a physics sandbox with modular architecture:

**Core Simulation** (`src/simulation/`)
- Physics engine: particle updates, pressure/velocity grid (Air), gravity, heat
- Elements: 150+ distinct substances with unique properties (DUST, LIQUID, GAS, ENERGY, SPECIAL, etc.)
- Reactions: element interactions defined in `ElementClasses.cpp` + element-specific `.cpp` files
- Tools: brushes and editing tools (CREATE, SMUDGE, HEAT, etc.) in `SimTool.h`
- Rendering: SaveRenderer handles thumbnails; offline rendering for saves

**Graphics & Display** (`src/graphics/`)
- Renderer abstracts SDL/OpenGL; pixel-level drawing, color palettes
- Pixel types map elements to visual representation
- View modes: normal, heat, pressure, velocity, vorticity, etc.

**UI & Interaction** (`src/gui/`)
- GameController: input handling, simulation stepping, tool management
- GameView: viewport and rendering orchestration
- Dialogues: menus, prompts, settings
- Engine: UI framework for windows, buttons, sliders (custom, not external)
- HUD: menus and parameter displays overlaid on simulation

**Scripting** (`src/lua/`)
- Lua 5.1/5.2/LuaJIT support (configurable at build time)
- CommandInterface: exposes simulation, graphics, and UI to Lua scripts
- Custom bindings for game-specific objects (Window, Button, Checkbox, etc.)
- Scripts run in sandboxed environment; can modify simulation and create UI

**Network & Saves** (`src/client/`)
- Client: manages account login, HTTP requests to powdertoy.co.uk
- GameSave: in-memory representation (metadata + particle grid)
- SaveFile: I/O format handling (bzip2 compression, thumbnail embedding)
- Requests: HTTP layer for login, upload, search, vote, etc.

**Platform Abstraction** (`src/common/platform/`)
- Unified API for filesystem, threading, clipboard (platform-specific implementations)
- Supports Windows, Linux, macOS, Android, Emscripten

**Utilities**
- `Probability.h`: deterministic RNG (seeded for replay)
- `Format.h`: number/data formatting for display
- `Misc.h`: common helpers (vector math, bit manipulation)

## Key Files to Know

- **Entry point**: `src/PowderToy.cpp` (main game loop, startup)
- **Element definitions**: `src/simulation/ElementClasses.cpp` + per-element `.cpp` files (e.g., `ElementDust.cpp`)
- **Simulation core**: `src/simulation/Simulation.h` (update loop, particle array, tools)
- **Physics**:
  - Air/pressure: `src/simulation/Air.h` (pressure and velocity grids)
  - Heat diffusion: integrated into element update
  - Gravity: `src/simulation/orbitalparts.h`
- **Configuration**: `src/Config.h` (build-time constants), `src/SimulationConfig.h` (runtime settings)
- **Particle storage**: `src/simulation/Particle.h` (x, y, vx, vy, element type, temperature, life, tmp/tmp2/tmp3 fields)
- **Rendering**: `src/graphics/Graphics.h`, `src/simulation/SaveRenderer.h`

## Development Patterns

**Element reactions:** Element-specific logic lives in `ElementClasses.cpp` or a dedicated `.cpp` file. Reactions are triggered during simulation update via `Simulation::UpdateParticles()`. Use `Particle` struct fields (x, y, vx, vy, type, temp, life, tmp*) for state; `SimulationData` to query neighboring cells.

**Tool behavior:** Tools defined in `ToolClasses.cpp`; called from `GameController` on mouse/keyboard input. Tools modify particle array and call `Simulation` methods to create/destroy particles.

**UI windows:** Custom UI framework in `src/gui/interface/`; windows are hierarchies of Components. Use `ModalDialog` for prompts; `GameView` orchestrates all rendering.

**Lua integration:** Script entry points are commands (e.g., `/script.lua`). Use `CommandInterface` to access simulation state. Scripts can spawn windows, read/write particles, trigger reactions.

**Logging:** No unified logging framework; use `std::cerr` or Lua's `error()` for debugging. Output redirectable via command-line flags (`redirect`, `console`).

## Common Tasks

**Add a new element:**
1. Define particle type in `src/simulation/ElementNumbers.template.h` (auto-generated)
2. Add graphics properties in `src/simulation/ElementDefs.cpp`
3. Implement interactions in `src/simulation/ElementClasses.cpp` (or create `Element{Name}.cpp`)
4. Compile and test with `meson compile -C build`

**Modify reactions:**
Element reactions are in `ElementClasses.cpp` under `Element_PT_{TYPE}::Update()`. Access neighbors via `SimulationData` queries (e.g., `SimulationData::InBounds()`, `SimulationData::GetParticleAt()`); use `Particle` fields for state changes.

**Add a tool:**
Define in `ToolClasses.cpp`, register in `SimTool.h`. Implement on_tool_draw and on_tool_click callbacks to modify particles and geometry.

**Debug a physics issue:**
- Enable view modes: Shift+0 (vorticity), 0-9 (pressure, velocity, heat, etc.)
- Check `src/simulation/Air.h` pressure/velocity updates
- Verify element reactions trigger correctly in `ElementClasses.cpp`
- Use Lua to inspect particle state: `sim.partProperty(idx, "type")`, `sim.partProperty(idx, "temp")`

**Build for a specific platform:**
Use Meson's cross-compilation toolchain. Android builds go through `android/` directory. Emscripten targets `host_platform == 'emscripten'` in build config.

## Important Notes

- **No RTTI / No exceptions**: Build disables RTTI and C++ exceptions for performance; use C-style returns or status codes for error handling.
- **32-bit & 64-bit**: Code supports both; watch for integer overflows in particle indexing (grid is 1D flattened array: `idx = y * width + x`).
- **Determinism**: Simulations are deterministic given a seed for replay; avoid floating-point nondeterminism or platform-specific randomness.
- **SDL2 abstraction**: Graphics and input go through `SDLCompat.h`, not raw SDL; platform-specific code in `PowderToySDL*` files.
- **Pressure/velocity grid**: Air updates are CPU-intensive; grid size is configurable (default ~simulation_width / default_air_mode settings).

## Build Variants

- **Debug** (`-Doptimization=0`): unoptimized, all assertions enabled, useful for tracing bugs
- **Release** (`-Doptimization=3`): optimized, assertions disabled, requires `-Denforce_https=true`
- **Snapshot** (`-Dsnapshot=true`): intermediate builds with version string; used for testing
- **Mod** (`-Dmod_id=N`): community mods; have their own ID and update server
- **Static** (`-Dstatic=prebuilt`): bundles all dependencies; official releases use this

## Testing & Validation

- No integrated test suite; validation is manual (run the game, try elements/tools).
- Physics regression: save before/after changes and compare behavior via Lua or visual inspection.
- Platform validation: GitHub Actions runs builds on Windows, macOS, Linux, and Android.
- Lua scripts in `resources/` can be used to test API stability.

## Resources

- [Official wiki](https://powdertoy.co.uk/Wiki/) — element properties, tool descriptions, Lua API docs
- `README.md` — command-line arguments, in-game controls
- `meson_options.txt` — comprehensive build configuration documentation
- Git history: recent commits show physics fixes, reaction changes, performance tuning

---

**Mod development?** Change `meson_options.txt` options (`app_name`, `app_id`, etc.) for custom branding. Official mods register a `mod_id` with maintainers for auto-update support.
