# 01: Extract the switching decision into a pure, tested module

**What to build:** Automatic switching behaves exactly as it does today, but the decision "given the current active profile and this foreground process, what should happen" is made by a pure C++ module with no Win32 or NVAPI dependency, and that module has a console test executable that CI builds and runs. This is the prefactor that makes every later ticket easy and is the single test seam agreed in the spec.

**Blocked by:** None (can start immediately)

**Status:** ready-for-agent

**Spec:** `.scratch/hotkey-triggers/spec.md`, sections "Switching decision module" and "Testing Decisions"

## Notes

- Inputs: switching state (active profile name, override flag), a reduced profile list (name, executable basename, enabled), and one event. Output: new state plus one action (none / apply named profile / restore Windows).
- Only the `Startup` and `ForegroundChanged` events need to be implemented here. Leave room for the hotkey events; do not implement them yet.
- The override flag is present in the state from day one but is never set in this ticket.
- The timer handler and startup path in the Win32 layer become thin adapters: gather inputs, call the module, perform the action, repaint.
- The test executable is a plain console program with assert-style checks and a non-zero exit on failure. No framework. CI compiles it with a second compiler invocation and runs it before building the main executable so a failing test fails the build.
- The reduced profile list should be derived from the existing settings type by a small adapter so the module never sees the Win32 or settings globals.

## Acceptance criteria

- [x] The main executable builds and behaves as before: a configured executable in the foreground applies its profile, leaving it restores the Windows profile, startup restores the Windows profile. (Built locally; behaviour is covered by the module tests. Not yet run against a GPU.)
- [x] The decision module compiles as its own translation unit with no Windows headers included.
- [x] Tests cover: startup restores Windows; foreground change to an enabled configured executable applies it; foreground change to an unconfigured executable while a profile is active restores Windows; a disabled profile never matches; matching is case-insensitive on the basename; no action is emitted when the matched profile is already active.
- [x] The CI workflow builds and runs the test executable, and the job fails when a test fails. (Step added; not yet exercised on GitHub because nothing is committed.)
- [x] Vocabulary in identifiers and test names follows `CONTEXT.md` (Profile, Windows profile, Active profile, Executable trigger, Override, Automatic switching).

## Comments

**2026-09-08, implemented, uncommitted.** New `switching.h` / `switching.cpp` beside `main.cpp`; tests in `tests/switching_tests.cpp` (31 checks, all passing locally with VS Build Tools). The timer handler, startup, and the three places that used to write the active-profile name directly now go through the module's state. The `Startup` event replaces the hard-coded reset after the initial restore. CI gained a "Build and run tests" step ahead of the main build, and the main compiler command now includes the new source file.

Known limitation carried over from the original code: profiles are identified by name, so two profiles with the same name are ambiguous when the module says "apply this profile". The adapter picks the first by name. Not new, but worth a follow-up if names are ever allowed to duplicate.

Local builds write to the session scratchpad, not the repo. The CI test step writes to `tests/out/` inside the source tree; if `.scratch/` is gitignored later, ignore `tests/out/` too.
