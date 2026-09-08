# NvProfileSwitcher — Product Requirements Document

| | |
| --- | --- |
| Product | NvProfileSwitcher |
| Current version | 1.2.0 (2026-09-04) |
| Platform | Windows 10 / 11, x64, NVIDIA GPU |
| Upstream | https://github.com/mgcarnevali/NvProfileSwitcher (this repo is a fork) |
| License | MIT |
| Status of this document | Reverse-engineered from the shipped code and docs on 2026-09-07. Describes what exists today, then lists gaps and open questions. |

## 1. Summary

NvProfileSwitcher is a small native Windows utility that automatically switches NVIDIA display color settings (brightness, contrast, gamma, digital vibrance, hue) depending on which application owns the foreground window. The user configures one "Windows" profile per monitor and any number of per-game profiles keyed by the game's executable. When a configured game comes to the foreground, its profile is applied to each configured display. When the user switches away, every display returns to its own Windows profile.

It replaces the manual workflow of opening the NVIDIA App or vibranceGUI before and after each gaming session, and it does so without a .NET runtime, an installer requirement, or the NVIDIA App running in the background.

## 2. Problem

Competitive and casual PC gamers commonly run games with boosted digital vibrance, adjusted gamma, or other color tweaks that look wrong on the desktop. NVIDIA's own tooling applies these settings globally and does not switch them per application. Existing third-party tools (vibranceGUI and similar) cover vibrance only, are often .NET based, or have not kept up with multi-monitor setups.

Users want:

- Color settings that follow the game, not the machine.
- Independent settings per monitor, since game and desktop monitors are often different panels.
- Something lightweight that runs from the tray and is forgotten about.

## 3. Goals and non-goals

### Goals

1. Zero-touch switching: once a game profile is saved, the user never interacts with the app during play.
2. Faithful restore: leaving a game returns every monitor to exactly its saved Windows values.
3. Full NVIDIA App parity for the five color controls, using the same value ranges the NVIDIA App shows so users can copy numbers across.
4. Multi-monitor correctness: settings land on the intended physical display.
5. Lightweight and portable: single native x64 executable, no runtime dependencies, small footprint.
6. Trustworthy: no telemetry, network access only for the opt-out update check, open source, signed releases.

### Non-goals

- Supporting AMD or Intel GPUs.
- Managing NVIDIA 3D or driver profiles (anti-aliasing, V-sync, DLSS, etc.). "Profile" in this product means color settings only.
- Per-window or per-monitor-region color adjustments below the whole-display level.
- ICC profile management or HDR tone mapping.
- A background auto-updater. The update checker links to the GitHub release; the user installs it.
- Cross-platform support.

## 4. Target users

| Persona | Need |
| --- | --- |
| Competitive FPS player | High vibrance in game for target visibility, normal colors on desktop. Often multi-monitor with a game monitor and a chat/browser monitor. |
| Casual gamer with a calibrated desktop | Per-game gamma or brightness for dark games without disturbing color-managed desktop work. |
| Streamer / content creator | Wants the switch to happen automatically so the stream never shows the desktop in "game colors". |

Assumed technical level: comfortable downloading a ZIP from GitHub and picking an `.exe` in a file dialog. Not expected to edit JSON.

## 5. Functional requirements (current state)

Status legend: **Shipped** = present in 1.2.0. **Partial** = present with known limitations noted. **Gap** = not implemented.

### 5.1 Profiles

| ID | Requirement | Status |
| --- | --- | --- |
| P1 | One built-in, non-deletable **Windows** profile representing the desktop state. | Shipped |
| P2 | The Windows profile stores independent values for every detected NVIDIA display. | Shipped |
| P3 | User can add any number of **game profiles**, each with a name and an executable path chosen via file dialog. | Shipped |
| P4 | Each game profile stores independent values for every detected display. New game profiles are seeded from the current Windows values per display. | Shipped |
| P5 | Each game profile can be individually enabled or disabled without deleting it. | Shipped |
| P6 | Game profiles show the icon extracted from their executable; profiles without an executable show a placeholder. | Shipped |
| P7 | Profiles are matched to the foreground process by executable **basename only** (case-insensitive, extension stripped). The full path is stored but not compared. | Partial. Two games sharing a launcher/exe name (for example `game.exe` in different folders) cannot be told apart. |
| P8 | Profiles created before 1.1.1 (single flat set of values) are migrated to the per-display format on load. | Shipped |
| P9 | Reordering or duplicating profiles. | Gap |
| P10 | Matching by window title, Steam AppID, or process arguments. | Gap |

### 5.2 Color controls

| ID | Requirement | Status |
| --- | --- | --- |
| C1 | Brightness, range 80 to 120, default 100. Applied via the NVIDIA gamma correction LUT. | Shipped |
| C2 | Contrast, range 80 to 120, default 100. Applied via the LUT. | Shipped |
| C3 | Gamma, range 0.30 to 2.80, default 1.00. Applied via the LUT. | Shipped |
| C4 | Digital Vibrance, 0 to 100 percent with 50 as driver default. Mapped onto the driver-reported min/default/max range like the NVIDIA App. | Shipped |
| C5 | Hue, 0 to 359 degrees, default 0. | Shipped |
| C6 | Live preview while dragging a slider before saving. | Shipped (unreleased). Slider changes are applied to the selected display as a preview; discarded on selection, display, or minimize without a save. |
| C7 | "Reset to defaults" per control or per profile. | Partial. A **Defaults** button resets all five controls for the selected display; per-control reset is not implemented. |

### 5.3 Switching engine

| ID | Requirement | Status |
| --- | --- | --- |
| S1 | Poll the foreground window every 250 ms and resolve its process image name. | Shipped |
| S2 | When the foreground process matches an enabled game profile and differs from the currently active profile, apply that profile to all of its displays. | Shipped |
| S3 | When no enabled profile matches and a game was active, restore every display to its Windows values. | Shipped |
| S4 | On startup, apply the Windows profile to all displays so the desktop is always in a known state. | Shipped |
| S5 | Apply settings only when the active profile changes, never on every tick. | Shipped |
| S6 | Re-enumerate displays when monitors are connected, disconnected, or the primary changes. | Gap. Displays are enumerated once at startup. A hot-plugged monitor is not known until restart. |
| S7 | Re-apply the active profile after sleep/resume or a driver reset, which can clear NVAPI state. | Gap |
| S8 | Restore Windows values on exit, logoff, or shutdown. | Partial. Exit via tray unloads NVAPI but does not explicitly restore the Windows profile first. |
| S9 | Report NVAPI status ("Ready", "NVAPI not found", "Could not set Hue", etc.) in the footer. | Shipped |

### 5.4 Multi-monitor

| ID | Requirement | Status |
| --- | --- | --- |
| M1 | Enumerate active, non-mirroring GDI displays and resolve each to an NVAPI display handle and display ID. | Shipped |
| M2 | Label each display with its monitor model name and mark the primary. | Shipped |
| M3 | A display selector in Profile Settings shows and edits the values for one display at a time. | Shipped |
| M4 | Fall back to the primary NVIDIA display if per-display resolution fails. | Shipped |
| M5 | Displays driven by a non-NVIDIA GPU (for example a laptop iGPU) are skipped silently. | Shipped by omission. Should be surfaced to the user. |

### 5.5 Application behaviour

| ID | Requirement | Status |
| --- | --- | --- |
| A1 | Single instance. A second launch brings the existing window to the front. | Shipped |
| A2 | **Start with Windows** via the `HKCU` Run key, launching with `--minimized`. | Shipped |
| A3 | **Start minimized to tray.** | Shipped |
| A4 | **Minimize to tray** toggle. Tray icon exists only while minimized there. | Shipped |
| A5 | Tray menu: Open, Check for updates, About, Exit. | Shipped |
| A6 | Update checker: GET `api.github.com/repos/mgcarnevali/NvProfileSwitcher/releases/latest` on a worker thread with 4 to 6 s timeouts. Compares semver, shows a dialog with a link. Opt-out setting; manual check always available. | Shipped. Note the URL points to upstream, not this fork. |
| A7 | Dark themed native UI with three sections (Profiles, Profile Settings, Application Settings) and a status footer. | Shipped |
| A8 | Fixed window size, centered on the primary monitor, DPI aware. | Shipped |
| A9 | Localisation. | Gap. English only, strings hard-coded. |
| A10 | Keyboard accessibility and screen-reader labels for owner-drawn controls. | Gap |
| A11 | Command-line or hotkey to force a profile. | Gap |

### 5.6 Persistence

| ID | Requirement | Status |
| --- | --- | --- |
| D1 | All state stored in `%APPDATA%\NvProfileSwitcher\profiles.json`, created on first save. | Shipped |
| D2 | Human-readable JSON with `Windows Profiles`, `Games Profiles`, and four boolean app settings. | Shipped |
| D3 | Robust JSON handling (escaping, malformed input). | Partial. Hand-rolled parser and writer. Handles the app's own output; arbitrary edits may not round-trip. |
| D4 | Import/export or backup of profiles. | Gap |

## 6. Non-functional requirements

| Area | Requirement | Current state |
| --- | --- | --- |
| Footprint | Single executable, no runtime install. | Met. Static CRT (`/MT`), only Win32 + GDI+ + WinHTTP + `nvapi64.dll` from the driver. |
| Latency | Profile applies within one poll interval of the game gaining focus. | Met. 250 ms timer. |
| Idle cost | Negligible CPU when the foreground window is unchanged. | Met in practice, though every tick opens the foreground process handle. |
| Privacy | No telemetry. Only network call is the GitHub release check. | Met and documented in `CODE_SIGNING.md`. |
| Security | Runs as the invoking user. | Met for the executable (`asInvoker`). The Inno Setup installer requests admin because it installs to Program Files. |
| Compatibility | Windows 10/11 x64, any NVIDIA driver exposing NVAPI. | Met. Uses undocumented NVAPI function IDs, so a driver change could break individual calls. |
| Reliability | Failure of one NVAPI call is surfaced in the footer, not fatal. | Met. |
| Supply chain | Reproducible builds from GitHub Actions; releases signed via SignPath. | Met for upstream. Fork's CI still targets upstream URLs. |

## 7. Architecture overview

Everything lives in one translation unit, `NvProfileSwitcher/main.cpp` (about 2,100 lines), plus a resource script, manifest, and version header.

```
wWinMain
 ├─ single-instance mutex, GDI+ startup, embedded PNG/icon loading
 ├─ Load()  ────────── profiles.json → Settings (with v1.1.1 migration)
 ├─ InitNv() ───────── LoadLibrary(nvapi64.dll), nvapi_QueryInterface by ID,
 │                     EnumerateNvDisplays(), driver version
 ├─ RestoreAllDesktopProfiles()
 ├─ CreateWindow + BuildControls() (owner-drawn Win32 controls)
 └─ message loop
      ├─ WM_TIMER (250 ms) → CheckProcesses()
      │     ForegroundProcessName() → match against gSettings.profiles
      │     → ApplyGameProfile() / RestoreAllDesktopProfiles()
      │           └─ Apply(): SetDVC, SetHue, SetNvGamma(LUT) per display
      ├─ WM_COMMAND → Save/Add/Remove/Browse/settings toggles → Save()
      ├─ WM_TRAY → tray menu
      └─ WM_UPDATE_AVAILABLE ← UpdateCheckThread (WinHTTP)
```

Key data types:

- `DisplayProfileValues`: one display's five color values.
- `GameProfile`: name, exe path, enabled flag, legacy flat values, and a vector of `DisplayProfileValues`. The Windows profile is a vector of `GameProfile`, one per display.
- `Settings`: desktop profiles, game profiles, four booleans.
- `DisplayTarget`: GDI name, label, NVAPI handle, NVAPI display ID, primary flag.

NVAPI surface used (by interface ID): Initialize, Unload, EnumNvidiaDisplayHandle, GetDVCInfoEx, SetDVCLevelEx, GetHUEInfo, SetHUEAngle, GetPrimaryDisplayId, DISP_SetTargetGammaCorrection, GetAssociatedNvidiaDisplayHandle, DISP_GetDisplayIdByDisplayName, GetDriverAndBranchVersion.

## 8. Build, release, and distribution

- **Toolchain**: MSVC x64, C++20, `/O2 /EHsc /MT`, subsystem WINDOWS. No project file; the build is a two-command `rc` + `cl` invocation inside `.github/workflows/build.yml`.
- **Versioning**: CI rewrites `version.h`. Tags `vMAJOR.MINOR.PATCH` produce release builds; pushes to `main` produce `dev-<sha7>` builds flagged as development in the footer.
- **Artifacts**: portable ZIP containing the exe, plus an Inno Setup installer (`NvProfileSwitcher-Setup-v<version>.exe`). Both are attached to the GitHub Release on tag builds.
- **Signing**: SignPath Foundation certificate for official upstream releases only.
- **Configuration**: none at install time; everything is in the user's `%APPDATA%` JSON.

## 9. Known gaps and observations from the code review

These are candidates for tickets. None are confirmed bugs against upstream unless marked.

1. **Display hot-plug not handled** (S6). No `WM_DISPLAYCHANGE` handling; `EnumerateNvDisplays()` runs once. Docking a laptop or turning a monitor off and on leaves stale handles.
2. **Sleep/resume not handled** (S7). No `WM_POWERBROADCAST` handling. Vibrance and LUT may reset after resume while the app still believes the game profile is active.
3. **Exe matched by basename only** (P7). Any process named like the game triggers the profile, including the game's own launcher if it shares the name.
4. **Fork-specific URLs**. `APP_URL`, `SUPPORT_URL`, `UPDATE_PATH`, the installer `MyAppURL`, and `FUNDING.yml` all point at `mgcarnevali`. If this fork is meant to release independently, these need to change. If it is meant to contribute upstream, they should stay.
5. **Manifest identity mismatch**. `app.manifest` still declares `name="GameProfileSwitcher"` and description "Game Profile Switcher", a leftover from an earlier name.
6. **Installer vs. portable messaging**. README says "Portable — no installation required" and the Installation section only describes the ZIP, but CI now also ships an Inno Setup installer that requires admin. Docs should describe both.
7. **Hand-rolled JSON**. Parser and writer are custom string scanning. Works for the app's own output; fragile against hand edits or future schema growth.
8. **No tests**. There is no test target of any kind. The LUT math (`SetNvGamma`), vibrance mapping (`DvcRawFromPercent`), version comparison, and JSON round-trip are pure functions that could be unit tested without a GPU.
9. **Single 2,100-line file**. UI drawing, NVAPI binding, persistence, and the switching engine are interleaved. Splitting into `nvapi.cpp`, `settings.cpp`, `engine.cpp`, `ui.cpp` would make the code navigable and testable.
10. **Non-NVIDIA displays vanish silently** (M5). A user with a monitor on the iGPU sees it missing from the selector with no explanation.
11. **Exit does not restore desktop colors** (S8). Quitting while a game profile is active leaves the game colors applied until the next launch.
12. **Undocumented NVAPI IDs**. Standard for this class of tool, but a driver update could break a call. The app degrades gracefully with a footer message, which is the right behaviour; worth stating in README.
13. **Git history** is mostly "Add files via upload" commits from the GitHub web UI, so `git blame` and bisect are of limited use.

## 10. Open questions for this fork

1. Is the fork's purpose to contribute back upstream, or to diverge as a separate product? This decides item 4 above and whether Issues should be enabled here.
2. Should the switching engine move from polling to `SetWinEventHook(EVENT_SYSTEM_FOREGROUND)`? It would remove the 250 ms timer and the per-tick `OpenProcess`, at the cost of some complexity around hook lifetime.
3. Should profiles support full-path matching, or a Steam AppID, as an opt-in for users hit by item 3?
4. Is Windows 10 still a supported target, or can the app assume Windows 11 APIs?

## 11. Glossary

| Term | Meaning in this product |
| --- | --- |
| Profile | A named set of the five color values, per display. Not an NVIDIA driver 3D profile. |
| Windows profile | The desktop profile. One per detected display. Applied whenever no game profile matches. |
| Game profile | A profile bound to an executable name. Applied while that executable owns the foreground window. |
| Display / target | One physical monitor as seen by both GDI (`\\.\DISPLAYn`) and NVAPI (handle + display ID). |
| Active profile | The profile most recently applied by the switching engine. Shown in the footer. |
| Digital Vibrance (DVC) | NVIDIA's saturation control. Stored as 0 to 100 with 50 meaning driver default. |
| LUT | The 1024-entry gamma correction ramp used to implement brightness, contrast, and gamma. |
| Dev build | A CI build from `main`, versioned `dev-<sha>`, never signed. |
